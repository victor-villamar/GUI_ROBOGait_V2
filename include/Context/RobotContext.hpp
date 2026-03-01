#pragma once

#include <QString>

#include <string>

namespace ROBOGait
{
namespace context
{
/**
 * @brief Class to manage robot context information
 */
class RobotContext
{
public:
  /**
   * @brief Constructor of RobotContext class
   */
  RobotContext();

  /**
   * @brief Set the selected robot context
   *
   * @param robot_identifier The identifier of the robot
   * @param is_namespace Flag indicating if the identifier is a namespace
   *
   * @return true if the context was set successfully, false otherwise
   */
  bool setSelectedRobot(const QString& robot_identifier, bool is_namespace);

  /**
   * @brief Clear the selected robot context
   */
  void clear();

  /**
   * @brief Check if the robot context is configured
   *
   * @return true if the context is configured, false otherwise
   */
  bool isConfigured() const;

  /**
   * @brief Check if the robot context uses a namespace
   *
   * @return true if the context uses a namespace, false otherwise
   */
  bool usesNamespace() const;

  /**
   * @brief Get the topic namespace
   *
   * @return The topic namespace
   */
  QString topicNamespace() const;

  /**
   * @brief Resolve the topic name
   *
   * @param topic The topic name to resolve
   *
   * @return The resolved topic name
   */
  std::string resolveTopic(const std::string& topic) const;

  /**
   * @brief Resolve the frame name
   *
   * @param frame The frame name to resolve
   *
   * @return The resolved frame name
   */
  std::string resolveFrame(const std::string& frame) const;

private:
  /**
   * @brief Normalize the robot namespace
   *
   * @param robot_namespace The robot namespace to normalize
   *
   * @return The normalized robot namespace
   */
  QString normalizeNamespace(const QString& robot_namespace) const;

  /**
   * @brief Normalize the topic name
   *
   * @param topic The topic name to normalize
   *
   * @return The normalized topic name
   */
  std::string normalizeTopic(const std::string& topic) const;

  /**
   * @brief Normalize the frame name
   *
   * @param frame The frame name to normalize
   *
   * @return The normalized frame name
   */
  std::string normalizeFrame(const std::string& frame) const;

  QString robot_identifier_; /**< The identifier of the robot */
  QString topic_namespace_;  /**< The namespace of the topic */
  bool use_namespace_;       /**< Flag indicating if a namespace is used */
};

} // namespace context
} // namespace ROBOGait
