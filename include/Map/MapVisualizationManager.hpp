#pragma once

#include <QObject>
#include <memory>
#include <rclcpp/node.hpp>

#include "Map/Display/MapDisplay.hpp"
#include "Map/Display/RobotDisplay.hpp"
#include "Map/Rendering/MapRenderWidget.hpp"
#include "Map/Rendering/VisualizationManager.hpp"

namespace ROBOGait
{
namespace map
{
namespace manager
{

/**
 * @brief Manages map visualization lifecycle tied to robot selection and view visibility
 */
class MapVisualizationManager : public QObject
{
  Q_OBJECT

  // clang-format off
  Q_PROPERTY(bool isInitialized
             READ isInitialized
             NOTIFY isInitializedChanged)

  Q_PROPERTY(bool mapAvailable
             READ isMapAvailable
             NOTIFY mapAvailableChanged)

  Q_PROPERTY(bool robotPoseAvailable
             READ isRobotPoseAvailable
             NOTIFY robotPoseAvailableChanged)

  Q_PROPERTY(double zoomLevel
             READ getZoomLevel
             NOTIFY zoomLevelChanged)
  
  Q_PROPERTY(ROBOGait::map::rendering::MapRenderWidget* mapRenderWidget
             READ getMapRenderWidget
             CONSTANT)
  // clang-format on

public:
  /**
   * @brief Constructor of MapVisualizationManager class
   */
  MapVisualizationManager();

  /**
   * @brief Destructor of MapVisualizationManager class
   */
  ~MapVisualizationManager();

  /**
   * @brief Set ROS node for visualization system
   *
   * @param parent_node Pointer to parent ROS node
   */
  void setROSNode(rclcpp::Node* parent_node);

  /**
   * @brief Set selected robot for visualization
   *
   * Creates displays configured for the selected robot's namespace.
   *
   * @param robot_identifier Robot namespace or node name
   * @param is_namespace If true, identifier is namespace; if false, node name
   */
  void setSelectedRobot(const QString& robot_identifier, bool is_namespace);

  /**
   * @brief Activate map subscriptions
   *
   * Called when MapView becomes visible. Starts receiving map and pose data.
   */
  Q_INVOKABLE void activateSubscriptions();

  /**
   * @brief Destroy map subscriptions
   *
   * Called when MapView is hidden. Stops receiving data to save resources.
   */
  Q_INVOKABLE void destroySubscriptions();

  /**
   * @brief Check if manager is initialized
   *
   * @return true if ROS node is set
   */
  bool isInitialized() const;

  /**
   * @brief Check if map data is available
   *
   * @return true if map has been received
   */
  bool isMapAvailable() const;

  /**
   * @brief Check if robot pose is available
   *
   * @return true if robot pose is being tracked
   */
  bool isRobotPoseAvailable() const;

  /**
   * @brief Get current zoom level
   *
   * @return Zoom level (1.0 = 100%)
   */
  double getZoomLevel() const;

  /**
   * @brief Get map render widget for QML integration
   *
   * @return Pointer to MapRenderWidget
   */
  ROBOGait::map::rendering::MapRenderWidget* getMapRenderWidget() const;

  /**
   * @brief Register MapRenderWidget created in QML
   *
   * Called from QML Component.onCompleted to connect the widget to VisualizationManager
   *
   * @param widget Pointer to MapRenderWidget created in QML
   */
  Q_INVOKABLE void registerMapRenderWidget(QObject* widget);

signals:
  void isInitializedChanged();      // Emitted when initialization state changes
  void mapAvailableChanged();       // Emitted when map availability changes
  void robotPoseAvailableChanged(); // Emitted when robot pose availability changes
  void zoomLevelChanged();          // Emitted when zoom level changes

private:
  /**
   * @brief Create displays for selected robot
   */
  void createDisplays();

  /**
   * @brief Destroy all displays
   */
  void destroyDisplays();

  rclcpp::Node* parent_node_;                                                             /**< Parent ROS node pointer */
  std::shared_ptr<ROBOGait::map::rendering::VisualizationManager> visualization_manager_; /**< Visualization manager */
  ROBOGait::map::rendering::MapRenderWidget* map_render_widget_;                          /**< Map render widget (owned by QML) */

  std::shared_ptr<ROBOGait::map::display::MapDisplay> map_display_;     /**< Map display */
  std::shared_ptr<ROBOGait::map::display::RobotDisplay> robot_display_; /**< Robot display */

  QString selected_robot_namespace_; /**< Selected robot namespace */
  bool use_namespace_discovery_;     /**< Use namespace-based topic discovery */
  bool is_initialized_;              /**< Initialization flag */
  bool subscriptions_active_;        /**< Subscriptions active flag */
};

} // namespace manager
} // namespace map
} // namespace ROBOGait
