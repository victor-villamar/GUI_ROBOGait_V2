#include <QDebug>
#include <QSet>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <utility>

#include "RobotDiscovery.hpp"

using namespace ROBOTGait::discovery;

RobotDiscovery::RobotDiscovery() :
    parent_node_(nullptr),
    robots_(QStringList()),
    robots_namespaces_(QStringList()),
    is_scanning_(false),
    state_(State::IDLE),
    poll_interval_(1000),
    poll_timer_(this)
{
  poll_timer_.setInterval(poll_interval_);
  poll_timer_.setSingleShot(false);
  // clang-format off
  connect(&poll_timer_,
          &QTimer::timeout,
          this,
          &RobotDiscovery::updateFromGraph
  );
  // clang-format on

  qInfo() << "[RobotDiscovery::RobotDiscovery] RobotDiscovery created";
}

QStringList RobotDiscovery::getRobots() const { return robots_; }

bool RobotDiscovery::isScanning() const { return is_scanning_; }

RobotDiscovery::State RobotDiscovery::getState() const { return state_; }

int RobotDiscovery::getPollInterval() const { return poll_interval_; }

void RobotDiscovery::setROSNode(rclcpp::Node* node)
{
  if (node == nullptr)
  {
    qCritical() << "[RobotDiscovery::setROSNode] No valid ROS node provided";
    stopScanning();
    setRobots(QStringList(), QStringList());
    setState(State::NO_NODE);
    return;
  }

  parent_node_ = node;
}

void RobotDiscovery::setPollInterval(int interval)
{
  if (poll_interval_ != interval)
  {
    poll_interval_ = interval;
    poll_timer_.setInterval(poll_interval_);
    emit pollIntervalChanged();
  }
}

void RobotDiscovery::startScanning()
{
  if (parent_node_ == nullptr)
  {
    qCritical() << "[RobotDiscovery::startScanning] No valid ROS node available";
    setRobots(QStringList{}, QStringList{});
    setIsScanning(false);
    setState(State::NO_NODE);
    return;
  }

  setRobots(QStringList{}, QStringList{});
  setIsScanning(true);
  setState(State::SCANNING);
  updateFromGraph();
  poll_timer_.start();
}

void RobotDiscovery::stopScanning()
{
  if (!is_scanning_ && !poll_timer_.isActive())
  {
    qInfo() << "[RobotDiscovery::stopScanning] Scanning is already stopped";
    return;
  }

  poll_timer_.stop();
  setIsScanning(false);

  if (state_ == State::SCANNING || state_ == State::ROBOTS_FOUND)
  {
    setState(State::IDLE);
  }
}

QString RobotDiscovery::namespaceForIndexSelected(int index) const { return robots_namespaces_.value(index); }

void RobotDiscovery::setRobots(const QStringList& robots, const QStringList& robot_namespaces)
{
  if (robots_ != robots || robots_namespaces_ != robot_namespaces)
  {
    robots_ = robots;
    robots_namespaces_ = robot_namespaces;
    emit robotsChanged();
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

  try
  {
    const auto nodes = parent_node_->get_node_graph_interface()->get_node_names_and_namespaces();
    const auto node_name = parent_node_->get_name();
    const auto [robots, robot_namespaces] = computeRobotsListFromGraph(nodes, node_name);

    setRobots(robots, robot_namespaces);

    if (robots.isEmpty())
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
  catch (const std::exception& e)
  {
    qCritical() << "[RobotDiscovery::updateFromGraph] Exception occurred: " << e.what() << ", graph update aborted";
    setRobots(QStringList{}, QStringList{});
    setState(State::ERROR);
  }
}

std::pair<QStringList, QStringList>
RobotDiscovery::computeRobotsListFromGraph(const std::vector<std::pair<std::string, std::string>>& nodes_names_and_namespaces,
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

    namespaces.insert(namespace_real);
  }

  return buildRobotsListFromNamespaces(namespaces);
}

std::pair<QStringList, QStringList> RobotDiscovery::buildRobotsListFromNamespaces(const QSet<QString>& namespaces)
{
  std::vector<std::pair<QString, QString>> robots;

  robots.reserve(namespaces.size());

  for (const auto& ns : namespaces)
  {
    QString robot_display = ns;

    if (robot_display.startsWith('/'))
    {
      robot_display.remove(0, 1);
    }

    robot_display.replace("_", " ");
    robots.emplace_back(robot_display, ns);
  }

  std::sort(robots.begin(), robots.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

  QStringList display_list;
  QStringList namespace_list;

  display_list.reserve(static_cast<int>(robots.size()));
  namespace_list.reserve(static_cast<int>(robots.size()));

  for (const auto& [display, ns] : robots)
  {
    display_list.append(display);
    namespace_list.append(ns);
  }

  return {display_list, namespace_list};
}
