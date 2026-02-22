#pragma once

#include <QGraphicsItem>
#include <QObject>
#include <QPainter>
#include <QTransform>
#include <rclcpp/node.hpp>
#include <string>

namespace ROBOGait
{
namespace map
{
namespace display
{

/**
 * @brief Base class for all visualization displays
 */
class BaseDisplay : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Constructor of BaseDisplay class
   *
   * @param name Display name
   * @param parent Qt parent object
   */
  BaseDisplay(const std::string& name, QObject* parent = nullptr);

  /**
   * @brief Virtual destructor
   */
  virtual ~BaseDisplay() = default;

  /**
   * @brief Initialize display with ROS2 node
   *
   * This method is called once after construction to set up ROS2 subscriptions,
   * TF listeners, and any other resources needed by the display.
   *
   * @param parent_node ROS2 node to use for subscriptions
   */
  virtual void initialize(rclcpp::Node* parent_node) = 0;

  /**
   * @brief Shutdown display and release resources
   *
   * Called when the display is being destroyed or disabled.
   * Should cancel timers, destroy subscriptions, and free memory.
   */
  virtual void shutdown() = 0;

  /**
   * @brief Update display state
   *
   * @param wall_dt Delta time since last update (wall clock, seconds)
   * @param ros_dt Delta time since last update (ROS time, seconds)
   */
  virtual void update(double wall_dt, double ros_dt) = 0;

  /**
   * @brief Enable or disable the display
   *
   * @param enabled True to enable, false to disable
   */
  void setEnabled(bool enabled);

  /**
   * @brief Check if display is enabled
   *
   * @return True if enabled, false otherwise
   */
  bool isEnabled() const { return enabled_; }

  /**
   * @brief Get display name
   *
   * @return Display name
   */
  std::string getName() const { return name_; }

  /**
   * @brief Get the graphics item for this display
   *
   * Returns the QGraphicsItem that should be added to the scene.
   * Returns nullptr if this display doesn't have a visual representation.
   *
   * @return Pointer to QGraphicsItem, or nullptr
   */
  virtual QGraphicsItem* getGraphicsItem() { return nullptr; }

signals:
  void renderRequested();                     // Signal emitted when a render is requested
  void statusChanged(const QString& message); // Signal emitted when display status changes

protected:
  std::string name_;          /**< Display name */
  bool enabled_;              /**< Whether display is enabled */
  rclcpp::Node* parent_node_; /**< Parent ROS2 node pointer */
};

} // namespace display
} // namespace map
} // namespace ROBOGait
