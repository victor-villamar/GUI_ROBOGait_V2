#pragma once

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <rclcpp/node.hpp>
#include <string>
#include <vector>

namespace ROBOTGait
{
namespace discovery
{
class RobotDiscovery : public QObject
{
  Q_OBJECT

public:
  enum class State
  {
    IDLE,
    NO_NODE,
    SCANNING,
    NO_ROBOTS,
    ROBOTS_FOUND,
    ERROR
  };
  Q_ENUM(State)

  RobotDiscovery();
  ~RobotDiscovery() override = default;

  // clang-format off
  Q_PROPERTY(QStringList robots
             READ getRobots
             NOTIFY robotsChanged
  )
  Q_PROPERTY(bool isScanning
             READ isScanning
             NOTIFY isScanningChanged
  )
  Q_PROPERTY(State state
             READ getState
             NOTIFY stateChanged
  )
  Q_PROPERTY(int pollInterval
             READ getPollInterval
             WRITE setPollInterval
             NOTIFY pollIntervalChanged
  )
  // clang-format on

  QStringList getRobots() const;
  bool isScanning() const;
  State getState() const;
  int getPollInterval() const;
  void setPollInterval(int interval);

  void setROSNode(rclcpp::Node* node);

  Q_INVOKABLE void startScanning();
  Q_INVOKABLE void stopScanning();
  Q_INVOKABLE void refreshOnce();

signals:
  void robotsChanged();
  void isScanningChanged();
  void stateChanged();
  void pollIntervalChanged();

private:
  void setRobots(const QStringList& robots);
  void setIsScanning(bool is_scanning);
  void setState(State state);
  void updateFromGraph();
  static QStringList computeRobotNamespaces(const std::vector<std::pair<std::string, std::string>>& nodes_names_and_namespaces,
                                            const std::string& self_node_name);

  rclcpp::Node* parent_node_;
  QStringList robots_;
  bool is_scanning_;
  State state_;
  int poll_interval_;
  QTimer poll_timer_;
};
} // namespace discovery
} // namespace ROBOTGait