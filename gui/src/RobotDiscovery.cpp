#include <QDebug>
#include <QSet>
#include <algorithm>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>

#include "RobotDiscovery.hpp"

using namespace ROBOTGait::discovery;

RobotDiscovery::RobotDiscovery() :
    parent_node_(nullptr), robots_(QStringList()), is_scanning_(false), state_(State::IDLE), poll_interval_(1000), poll_timer_(this)
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
    setRobots(QStringList());
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
    setRobots(QStringList{});
    setIsScanning(false);
    setState(State::NO_NODE);
    return;
  }

  setRobots(QStringList{});
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

void RobotDiscovery::refreshOnce()
{
  if (parent_node_ == nullptr)
  {
    qCritical() << "[RobotDiscovery::refreshOnce] No valid ROS node available";
    setRobots(QStringList{});
    setIsScanning(false);
    setState(State::NO_NODE);
    return;
  }

  updateFromGraph();
}

void RobotDiscovery::setRobots(const QStringList& robots)
{
  if (robots_ != robots)
  {
    robots_ = robots;
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
    const QStringList robots = computeRobotNamespaces(nodes, node_name);

    setRobots(robots);

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
    setRobots(QStringList{});
    setState(State::ERROR);
  }
}

QStringList RobotDiscovery::computeRobotNamespaces(const std::vector<std::pair<std::string, std::string>>& nodes_names_and_namespaces,
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

    namespaces.insert(parts.first());
  }

  QStringList out = namespaces.values();
  out.sort();
  return out;
}
