#pragma once

#include <QObject>
#include <atomic>
#include <memory>
#include <rclcpp/executors.hpp>
#include <rclcpp/node.hpp>
#include <thread>

#include "RobotDiscovery.hpp"

namespace ROBOTGait
{
namespace manager
{
using namespace ROBOTGait::discovery;

class RosNodeManager : public QObject
{
  Q_OBJECT
public:
  explicit RosNodeManager();
  ~RosNodeManager() override;

  // clang-format off
  Q_PROPERTY(bool isRunning
             READ isRunning
             NOTIFY isRunningChanged
  )
  Q_PROPERTY(QString nodeName
             READ getNodeName
             WRITE setNodeName
             NOTIFY nodeNameChanged
  )
  Q_PROPERTY(RobotDiscovery* robotDiscovery
             READ getRobotDiscovery
             CONSTANT
  )
  // clang-format on

  bool isRunning() const;
  QString getNodeName() const;
  void setNodeName(const QString& name);
  RobotDiscovery* getRobotDiscovery() const;

  void initialize(int argc, char** argv);

  Q_INVOKABLE void shutdown();

signals:
  void isRunningChanged();
  void nodeNameChanged();
  void rosNodeConnected(rclcpp::Node* node);

private:
  void startSpinThread();
  void stopSpinThread();

  QString node_name_;
  std::shared_ptr<rclcpp::Node> ros_node_;
  std::unique_ptr<rclcpp::executors::MultiThreadedExecutor> executor_;
  std::thread spin_thread_;

  std::unique_ptr<RobotDiscovery> robot_discovery_;

  std::atomic<bool> is_running_;
};
} // namespace manager
} // namespace ROBOTGait