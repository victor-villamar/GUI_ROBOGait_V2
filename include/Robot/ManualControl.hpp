#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/publisher.hpp>

namespace ROBOGait
{
namespace control
{

/**
 * @brief Class for manual robot control
 *
 * This class provides functionality to manually control the robot
 * by sending velocity commands through the /cmd_vel topic.
 */
class ManualControl : public QObject
{
  Q_OBJECT

  // clang-format off
  Q_PROPERTY(double linearVelocity
             READ getLinearVelocity
             NOTIFY velocityChanged
  )
  Q_PROPERTY(double angularVelocity
             READ getAngularVelocity
             NOTIFY velocityChanged
  )
  // clang-format on

public:
  /**
   * @brief Constructor of ManualControl class
   */
  ManualControl();

  /**
   * @brief Destructor of ManualControl class
   */
  ~ManualControl();

  /**
   * @brief Sets the ROS node for manual control
   *
   * @param node Pointer to the ROS node
   */
  void setROSNode(rclcpp::Node* node);

  /**
   * @brief Sets the topic name
   *
   * @param topic_name Complete topic name
   */
  void setTopicName(const QString& topic_name);

  /**
   * @brief Destroys the publisher
   */
  void destroyPublisher();

  /**
   * @brief Gets the current linear velocity
   *
   * @return Current linear velocity in m/s
   */
  double getLinearVelocity() const;

  /**
   * @brief Gets the current angular velocity
   *
   * @return Current angular velocity in rad/s
   */
  double getAngularVelocity() const;

  /**
   * @brief Updates the robot velocity
   *
   * @param linear Linear velocity in m/s
   * @param angular Angular velocity in rad/s
   */
  Q_INVOKABLE void updateVelocity(double linear, double angular);

  /**
   * @brief Stops the robot (zero velocity)
   */
  Q_INVOKABLE void stopRobot();

signals:
  void velocityChanged();

private:
  /**
   * @brief Creates the publisher if it doesn't exist
   */
  void ensurePublisherCreated();

  /**
   * @brief Publishes the velocity command to the robot
   * @param linear Linear velocity
   * @param angular Angular velocity
   */
  void publishVelocity(double linear, double angular);

  rclcpp::Node* parent_node_;                                           /**< Pointer to the ROS node */
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_cmd_vel_; /**< Publisher for cmd_vel topic */

  QString topic_name_;      /**< Complete topic name */
  double linear_velocity_;  /**< Current linear velocity in m/s */
  double angular_velocity_; /**< Current angular velocity in rad/s */

  static constexpr double MAX_LINEAR_VELOCITY = 0.22;  /**< Maximum linear velocity */
  static constexpr double MAX_ANGULAR_VELOCITY = 2.84; /**< Maximum angular velocity */
};

} // namespace control
} // namespace ROBOGait