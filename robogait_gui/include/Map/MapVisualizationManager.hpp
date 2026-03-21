#pragma once

#include <memory>

#include <QObject>
#include <QPointF>
#include <QPointer>

#include <rclcpp/node.hpp>

#include "Map/Items/LaserLayerItem.hpp"
#include "Map/Items/MapLayerItem.hpp"
#include "Map/Items/RobotLayerItem.hpp"
#include "Map/Layer/LaserLayer.hpp"
#include "Map/Layer/MapLayer.hpp"
#include "Map/Layer/RobotLayer.hpp"
#include "Map/Rendering/RenderCamera.hpp"
#include "Map/Rendering/RenderScene.hpp"
#include "Map/Source/LaserSource.hpp"
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

  Q_PROPERTY(bool laserAvailable
             READ isLaserAvailable
             NOTIFY laserAvailableChanged)

  Q_PROPERTY(double zoomLevel
             READ getZoomLevel
             NOTIFY zoomLevelChanged)

  Q_PROPERTY(double mapResolution
             READ getMapResolution
             NOTIFY mapResolutionChanged)

  Q_PROPERTY(double scaleMeters
             READ getScaleMeters
             NOTIFY scaleChanged)

  Q_PROPERTY(int scalePixels
             READ getScalePixels
             NOTIFY scaleChanged)

  Q_PROPERTY(bool followRobot
             READ isFollowingRobot
             WRITE setFollowRobot
             NOTIFY followRobotChanged)
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
   * @brief Check if laser data is available
   *
   * @return true if laser data has been received
   */
  bool isLaserAvailable() const;

  /**
   * @brief Get current zoom level
   *
   * @return Zoom level (1.0 = 100%)
   */
  double getZoomLevel() const;

  /**
   * @brief Get current map resolution
   *
   * @return Map resolution in meters per pixel
   */
  double getMapResolution() const;

  /**
   * @brief Get current scale in meters
   *
   * @return Scale in meters
   */
  double getScaleMeters() const;

  /**
   * @brief Get current scale in pixels
   *
   * @return Scale in pixels
   */
  int getScalePixels() const;

  /**
   * @brief Check if the camera is following the robot
   *
   * @return true if following the robot, false otherwise
   */
  bool isFollowingRobot() const;

  /**
   * @brief Set whether the camera follows the robot
   *
   * @param follow_robot True to enable following, false to disable
   */
  void setFollowRobot(bool follow_robot);

  /**
   * @brief Convert screen coordinates to map coordinates
   *
   * @param screen_point Point in screen coordinates
   * @param map_point Output parameter for point in map coordinates
   *
   * @return true if conversion was successful, false otherwise
   */
  bool screenToMap(const QPointF& screen_point, QPointF& map_point) const;

  /**
   * @brief Set the robot's pose manually for visualization
   *
   * @param x X coordinate of the robot in map frame (meters)
   * @param y Y coordinate of the robot in map frame (meters)
   * @param theta Orientation of the robot in radians (0 = facing right, positive counter-clockwise)
   */
  void setManualRobotPose(double x, double y, double theta);

  /**
   * @brief Check if a given map point is inside the map boundaries
   *
   * @param x X coordinate of the point in map frame (meters)
   * @param y Y coordinate of the point in map frame (meters)
   *
   * @return true if the point is inside the map, false otherwise
   */
  bool isMapPointInside(double x, double y) const;

  /**
   * @brief Activate subscriptions for data sources
   */
  Q_INVOKABLE void activateSubscriptions();

  /**
   * @brief Destroy subscriptions for data sources
   */
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
   * @brief Register LaserLayerItem created in QML
   *
   * @param item Pointer to LaserLayerItem created in QML
   */
  Q_INVOKABLE void registerLaserLayerItem(QObject* item);

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

  /**
   * @brief Clear the current map
   */
  Q_INVOKABLE void clearMap();

private slots:
  void onFrameReady(); // Slot for handling frame readiness

signals:
  void isInitializedChanged();      // Emitted when initialization state changes
  void mapAvailableChanged();       // Emitted when map availability changes
  void robotPoseAvailableChanged(); // Emitted when robot pose availability changes
  void laserAvailableChanged();     // Emitted when laser availability changes
  void zoomLevelChanged();          // Emitted when zoom level changes
  void mapResolutionChanged();      // Emitted when map resolution changes
  void scaleChanged();              // Emitted when scale changes
  void followRobotChanged();        // Emitted when follow mode changes

private:
  /**
   * @brief Create render layers
   */
  void createLayers();

  /**
   * @brief Destroy render layers
   */
  void destroyLayers();

  /**
   * @brief Update availability of data sources
   */
  void updateAvailability();

  /**
   * @brief Update map resolution
   */
  void updateMapResolution();

  /**
   * @brief Update scale
   */
  void updateScale();

  /**
   * @brief Update follow robot camera
   */
  void updateFollowRobotCamera();

  rclcpp::Node* parent_node_; /**< Parent ROS node pointer */

  std::shared_ptr<ROBOGait::map::rendering::RenderScene> render_scene_;   /**< Render scene  */
  std::shared_ptr<ROBOGait::map::rendering::RenderCamera> render_camera_; /**< Shared camera for layers */
  std::shared_ptr<ROBOGait::map::layer::MapLayer> map_layer_;             /**< Map layer renderer */
  std::shared_ptr<ROBOGait::map::layer::RobotLayer> robot_layer_;         /**< Robot layer renderer */
  std::shared_ptr<ROBOGait::map::layer::LaserLayer> laser_layer_;         /**< Laser layer renderer */
  QPointer<ROBOGait::map::item::MapLayerItem> map_layer_item_;            /**< Map layer item */
  QPointer<ROBOGait::map::item::RobotLayerItem> robot_layer_item_;        /**< Robot layer item */
  QPointer<ROBOGait::map::item::LaserLayerItem> laser_layer_item_;        /**< Laser layer item */

  std::shared_ptr<ROBOGait::map::source::MapSource> map_source_;        /**< Map source */
  std::shared_ptr<ROBOGait::map::source::RobotPoseSource> pose_source_; /**< Robot pose source */
  std::shared_ptr<ROBOGait::map::source::LaserSource> laser_source_;    /**< Laser source */

  QString selected_robot_namespace_; /**< Selected robot namespace */
  bool use_namespace_discovery_;     /**< Use namespace-based topic discovery */
  bool is_initialized_;              /**< Initialization flag */
  bool subscriptions_active_;        /**< Subscriptions active flag */
  bool map_available_cache_;         /**< Cached map availability state */
  bool robot_pose_available_cache_;  /**< Cached robot pose availability state */
  bool laser_available_cache_;       /**< Cached laser availability state */
  double map_resolution_cache_;      /**< Cached map resolution (meters per pixel) */
  double scale_meters_cache_;        /**< Cached scale bar meters value */
  int scale_pixels_cache_;           /**< Cached scale bar pixel length */
  double robot_size_;                /**< Robot diameter used for rendering (meters) */
  bool follow_robot_;                /**< Whether the camera follows the robot */
};

} // namespace manager
} // namespace map
} // namespace ROBOGait
