#include <utility>

#include <QDebug>
#include <QSet>

#include <rclcpp/node_interfaces/node_graph_interface.hpp>

#include "Robot/RobotDiscovery.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::robot::discovery;

RobotDiscovery::RobotDiscovery() :
    parent_node_(nullptr),
    robots_namespaces_(QStringList()),
    is_scanning_(false),
    state_(State::IDLE),
    poll_timer_(this),
    poll_interval_(DEFAULT_POLL_INTERVAL),
    scan_timeout_timer_(this),
    scan_timeout_(DEFAULT_SCAN_TIMEOUT),
    use_namespace_discovery_(true),
    use_topic_filter_(true)
{
  poll_timer_.setInterval(poll_interval_);
  poll_timer_.setSingleShot(false);
  scan_timeout_timer_.setInterval(scan_timeout_);
  scan_timeout_timer_.setSingleShot(true);

  // clang-format off
  connect(&poll_timer_,
          &QTimer::timeout,
          this,
          &RobotDiscovery::updateFromGraph
  );
  connect(&scan_timeout_timer_,
          &QTimer::timeout,
          this,
          &RobotDiscovery::onScanTimeout
  );
  // clang-format on

  qInfo() << "[RobotDiscovery::RobotDiscovery] RobotDiscovery created";
}

bool RobotDiscovery::isScanning() const { return is_scanning_; }

RobotDiscovery::State RobotDiscovery::getState() const { return state_; }

int RobotDiscovery::getPollInterval() const { return poll_interval_; }

void RobotDiscovery::setPollInterval(int interval)
{
  if (poll_interval_ != interval)
  {
    poll_interval_ = interval;
    poll_timer_.setInterval(poll_interval_);
    emit pollIntervalChanged();
  }
}

QStringList RobotDiscovery::getRobotsNamespaces() const { return robots_namespaces_; }

void RobotDiscovery::setROSNode(rclcpp::Node* node)
{
  if (node == nullptr)
  {
    qCritical() << "[RobotDiscovery::setROSNode] No valid ROS node provided";
    stopScanning();
    setRobotsNamespaces(QStringList{});
    setState(State::NO_NODE);
    return;
  }

  parent_node_ = node;
}

void RobotDiscovery::startScanning()
{
  if (parent_node_ == nullptr)
  {
    qCritical() << "[RobotDiscovery::startScanning] No valid ROS node available";
    setRobotsNamespaces(QStringList{});
    setIsScanning(false);
    setState(State::NO_NODE);
    return;
  }

  setRobotsNamespaces(QStringList{});
  setIsScanning(true);
  setState(State::SCANNING);
  updateFromGraph();
  poll_timer_.start();
  scan_timeout_timer_.start();
}

void RobotDiscovery::stopScanning()
{
  if (!is_scanning_ && !poll_timer_.isActive())
  {
    qInfo() << "[RobotDiscovery::stopScanning] Scanning is already stopped";
    return;
  }

  poll_timer_.stop();
  scan_timeout_timer_.stop();
  setIsScanning(false);

  if (state_ == State::SCANNING || state_ == State::ROBOTS_FOUND)
  {
    setState(State::IDLE);
  }
}

void RobotDiscovery::setRobotsNamespaces(const QStringList& robot_namespaces)
{
  if (robots_namespaces_ != robot_namespaces)
  {
    robots_namespaces_ = robot_namespaces;
    emit robotsNamespacesChanged();
  }
}

void RobotDiscovery::setIsScanning(bool is_scanning)
{
  if (is_scanning_ != is_scanning)
  {
    is_scanning_ = is_scanning;
    emit isScanningChanged();
  }
}

void RobotDiscovery::setUseNamespaceDiscovery(bool use_namespace_discovery)
{
  if (use_namespace_discovery_ != use_namespace_discovery)
  {
    use_namespace_discovery_ = use_namespace_discovery;

    if (is_scanning_)
    {
      updateFromGraph();
    }
  }
}

void RobotDiscovery::setUseTopicFilter(bool use_topic_filter)
{
  if (use_topic_filter_ != use_topic_filter)
  {
    use_topic_filter_ = use_topic_filter;

    if (is_scanning_)
    {
      updateFromGraph();
    }
  }
}

void RobotDiscovery::setState(State state)
{
  if (state_ != state)
  {
    state_ = state;
    emit stateChanged();
  }
}

void RobotDiscovery::updateFromGraph()
{
  if (parent_node_ == nullptr)
  {
    qCritical() << "[RobotDiscovery::updateFromGraph] No valid ROS node available, graph update aborted";
    return;
  }

  const auto nodes = parent_node_->get_node_graph_interface()->get_node_names_and_namespaces();
  const auto node_name = parent_node_->get_name();

  QStringList robot_list;

  if (use_namespace_discovery_)
  {
    robot_list = computeRobotsListFromGraph(nodes, node_name);
  }
  else
  {
    if (use_topic_filter_ && !hasRobotStatusTopic(""))
    {
      robot_list = QStringList{};
    }
    else
    {
      robot_list = buildRobotNodeNamesFromGraph(nodes, node_name);
    }
  }

  setRobotsNamespaces(robot_list);

  if (robot_list.isEmpty())
  {
    if (state_ == State::ROBOTS_FOUND)
    {
      setState(State::NO_ROBOTS);
      stopScanning();
    }
    return;
  }

  setState(State::ROBOTS_FOUND);
}

QStringList RobotDiscovery::computeRobotsListFromGraph(const std::vector<std::pair<std::string, std::string>>& nodes_names_and_namespaces,
                                                       const std::string& self_node_name)
{
  QSet<QString> namespaces;

  for (const auto& [name, ns] : nodes_names_and_namespaces)
  {
    if (name == "rosout")
    {
      continue;
    }

    if (name == self_node_name)
    {
      continue;
    }

    QString ns_qstr = QString::fromStdString(ns).trimmed();

    // Ignore empty namespaces
    if (ns_qstr.isEmpty() || ns_qstr == "/")
    {
      continue;
    }

    // Ignore non-canonical namespaces
    if (!ns_qstr.startsWith('/'))
    {
      continue;
    }

    const QStringList parts = ns_qstr.split('/', Qt::SkipEmptyParts);

    if (parts.isEmpty())
    {
      continue;
    }

    const QString namespace_real = "/" + parts.first();

    if (use_topic_filter_)
    {
      if (hasRobotStatusTopic(namespace_real.toStdString()))
      {
        namespaces.insert(namespace_real);
      }
    }
    else
    {
      namespaces.insert(namespace_real);
    }
  }

  return buildRobotNamespacesFromNamespaces(namespaces);
}

QStringList RobotDiscovery::buildRobotNamespacesFromNamespaces(const QSet<QString>& namespaces)
{
  QStringList list;

  list.reserve(namespaces.size());

  for (const auto& ns : namespaces)
  {
    QString robot_display = ns;

    if (robot_display.startsWith('/'))
    {
      robot_display.remove(0, 1);
    }

    robot_display.replace("_", " ");
    list.append(robot_display);
  }

  std::sort(list.begin(), list.end());

  return list;
}

QStringList RobotDiscovery::buildRobotNodeNamesFromGraph(const std::vector<std::pair<std::string, std::string>>& nodes_names_and_namespaces,
                                                         const std::string& self_node_name)
{
  QSet<QString> node_names;

  for (const auto& [name, ns] : nodes_names_and_namespaces)
  {
    if (name == "rosout")
    {
      continue;
    }

    if (name == self_node_name)
    {
      continue;
    }

    QString name_qstr = QString::fromStdString(name).trimmed();

    if (name_qstr.isEmpty())
    {
      continue;
    }

    // Look for potential robot nodes by name
    if (name_qstr.contains("robot") || name_qstr.contains("turtlebot"))
    {
      node_names.insert(name_qstr);
    }
  }

  QStringList list;
  list.reserve(node_names.size());

  for (const auto& node_name : node_names)
  {
    QString robot_display = node_name;
    robot_display.replace("_", " ");
    list.append(robot_display);
  }

  std::sort(list.begin(), list.end());

  return list;
}

bool RobotDiscovery::hasRobotStatusTopic(const std::string& robot_namespace) const
{
  if (parent_node_ == nullptr)
  {
    qCritical() << "[RobotDiscovery::hasRobotStatusTopic] No valid ROS node available, cannot check for topic existence";
    return false;
  }

  const std::string full_topic = robot_namespace + T_ROBOT_STATUS;

  const auto topics = parent_node_->get_topic_names_and_types();

  return topics.find(full_topic) != topics.end();
}

void RobotDiscovery::onScanTimeout()
{
  if (is_scanning_ && state_ == State::SCANNING)
  {
    qInfo() << "[RobotDiscovery] Scan timeout reached, stopping discovery";
    stopScanning();
  }
}
