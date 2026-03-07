#pragma once

#include <memory>

#include <QObject>
#include <QPointer>

#include <rclcpp/node.hpp>

#include "Map/Items/MapLayerItem.hpp"
#include "Map/Items/RobotLayerItem.hpp"
#include "Map/Layer/MapLayer.hpp"
#include "Map/Layer/RobotLayer.hpp"
#include "Map/Rendering/RenderCamera.hpp"
#include "Map/Rendering/RenderScene.hpp"
#include "Map/Source/MapSource.hpp"
#include "Map/Source/RobotPoseSource.hpp"

namespace ROBOGait
{
namespace map
{
namespace manager
{

/**
 * @brief Orchestrates map visualization components and their interactions
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

  Q_PROPERTY(bool followRobot
             READ isFollowingRobot
             WRITE setFollowRobot
             NOTIFY followRobotChanged)
  // clang-format on

public:
  MapVisualizationManager();
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

  bool isFollowingRobot() const;

  /**
   * @brief Set whether the camera follows the robot
   *
   * @param follow_robot True to enable following, false to disable
   */
  void setFollowRobot(bool follow_robot);

  Q_INVOKABLE void activateSubscriptions();
  Q_INVOKABLE void destroySubscriptions();

  /**
   * @brief Register MapLayerItem created in QML
   *
   * @param item Pointer to MapLayerItem created in QML
   */
  Q_INVOKABLE void registerMapLayerItem(QObject* item);

  /**
   * @brief Register RobotLayerItem created in QML
   *
   * @param item Pointer to RobotLayerItem created in QML
   */
  Q_INVOKABLE void registerRobotLayerItem(QObject* item);

  /**
   * @brief Zoom in for GPU rendering camera
   */
  Q_INVOKABLE void zoomIn();

  /**
   * @brief Zoom out for GPU rendering camera
   */
  Q_INVOKABLE void zoomOut();

  /**
   * @brief Fit view of the map
   */
  Q_INVOKABLE void fitToView();

  /**
   * @brief Generate a PNG preview for the given map data
   */
  Q_INVOKABLE bool generateMapPreview(const QString& map_name);

  /**
   * @brief Delete the generated map preview
   */
  Q_INVOKABLE bool deleteMapPreview(const QString& map_name);

  /**
   * @brief Get the file path for the map preview image
   */
  Q_INVOKABLE QString getMapPreviewPath(const QString& map_name);

private slots:
  void onFrameReady(); // Slot for handling frame readiness

signals:
  void isInitializedChanged();      // Emitted when initialization state changes
  void mapAvailableChanged();       // Emitted when map availability changes
  void robotPoseAvailableChanged(); // Emitted when robot pose availability changes
  void zoomLevelChanged();          // Emitted when zoom level changes
  void followRobotChanged();        // Emitted when follow mode changes

private:
  void createLayers();
  void destroyLayers();
  void updateAvailability();
  void updateFollowRobotCamera();

  rclcpp::Node* parent_node_; /**< Parent ROS node pointer */

  std::shared_ptr<ROBOGait::map::rendering::RenderScene> render_scene_;   /**< Render scene  */
  std::shared_ptr<ROBOGait::map::rendering::RenderCamera> render_camera_; /**< Shared camera for layers */
  std::shared_ptr<ROBOGait::map::layer::MapLayer> map_layer_;             /**< Map layer renderer */
  std::shared_ptr<ROBOGait::map::layer::RobotLayer> robot_layer_;         /**< Robot layer renderer */
  QPointer<ROBOGait::map::item::MapLayerItem> map_layer_item_;            /**< Map layer item */
  QPointer<ROBOGait::map::item::RobotLayerItem> robot_layer_item_;        /**< Robot layer item */

  std::shared_ptr<ROBOGait::map::source::MapSource> map_source_;        /**< Map source */
  std::shared_ptr<ROBOGait::map::source::RobotPoseSource> pose_source_; /**< Robot pose source */

  QString selected_robot_namespace_; /**< Selected robot namespace */
  bool use_namespace_discovery_;     /**< Use namespace-based topic discovery */
  bool is_initialized_;              /**< Initialization flag */
  bool subscriptions_active_;        /**< Subscriptions active flag */
  bool map_available_cache_;         /**< Cached map availability state */
  bool robot_pose_available_cache_;  /**< Cached robot pose availability state */
  double robot_size_;                /**< Robot diameter used for rendering (meters) */
  bool follow_robot_;                /**< Whether the camera follows the robot */
};

} // namespace manager
} // namespace map
} // namespace ROBOGait
