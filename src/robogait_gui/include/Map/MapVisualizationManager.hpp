#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <QObject>
#include <QPointF>
#include <QPointer>
#include <QVariantList>
#include <QVariantMap>

#include <rclcpp/node.hpp>

#include "Map/Data/RobotPoseData.hpp"
#include "Map/History/TracedRouteHistory.hpp"
#include "Map/Interaction/SplinePathEditor.hpp"
#include "Map/Items/LaserLayerItem.hpp"
#include "Map/Items/MapLayerItem.hpp"
#include "Map/Items/ParticleCloudLayerItem.hpp"
#include "Map/Items/PathLayerItem.hpp"
#include "Map/Items/RobotLayerItem.hpp"
#include "Map/Layer/LaserLayer.hpp"
#include "Map/Layer/MapLayer.hpp"
#include "Map/Layer/ParticleCloudLayer.hpp"
#include "Map/Layer/PathLayer.hpp"
#include "Map/Layer/RobotLayer.hpp"
#include "Map/Rendering/RenderCamera.hpp"
#include "Map/Rendering/RenderScene.hpp"
#include "Map/Source/LaserSource.hpp"
#include "Map/Source/MapSource.hpp"
#include "Map/Source/ParticleCloudSource.hpp"
#include "Map/Source/PathSource.hpp"
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

  Q_PROPERTY(bool particleCloudAvailable
             READ isParticleCloudAvailable
             NOTIFY particleCloudAvailableChanged)

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

  Q_PROPERTY(ROBOGait::map::interaction::SplinePathEditor* splinePathEditor
             READ getSplinePathEditor
             CONSTANT)

  Q_PROPERTY(ROBOGait::map::history::TracedRouteHistory* tracedRouteHistory
             READ getTracedRouteHistory
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
   * @brief Check if particle cloud data is available
   *
   * @return true if particle cloud has been received
   */
  bool isParticleCloudAvailable() const;

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
   * @brief Convert map coordinates to screen coordinates
   *
   * @param map_point Point in map coordinates
   * @param screen_point Output parameter for point in screen coordinates
   *
   * @return true if conversion was successful, false otherwise
   */
  bool mapToScreen(const QPointF& map_point, QPointF& screen_point) const;

  /**
   * @brief Convert screen coordinates to map coordinates
   *
   * @param screen_x Screen x-coordinate
   * @param screen_y Screen y-coordinate
   *
   * @return QVariantMap with keys available, x, y
   */
  Q_INVOKABLE QVariantMap screenToMap(double screen_x, double screen_y) const;

  /**
   * @brief Convert map coordinates to screen coordinates
   *
   * @param map_x Map x-coordinate
   * @param map_y Map y-coordinate
   *
   * @return QVariantMap with keys available, x, y
   */
  Q_INVOKABLE QVariantMap mapToScreen(double map_x, double map_y) const;

  /**
   * @brief Set the robot's pose manually for visualization
   *
   * @param x X coordinate of the robot in map frame (meters)
   * @param y Y coordinate of the robot in map frame (meters)
   * @param theta Orientation of the robot in radians (0 = facing right, positive counter-clockwise)
   */
  void setManualRobotPose(double x, double y, double theta);

  /**
   * @brief Get the current robot pose from the TF stream
   *
   * @return QVariantMap with keys x, y, theta, available
   */
  Q_INVOKABLE QVariantMap getRobotPose() const;

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
   * @brief Get spline path editor
   */
  ROBOGait::map::interaction::SplinePathEditor* getSplinePathEditor() const;

  /**
   * @brief Get traced route history
   */
  ROBOGait::map::history::TracedRouteHistory* getTracedRouteHistory() const;

  /**
   * @brief Activate subscriptions for data sources
   */
  Q_INVOKABLE void activateSubscriptions();

  /**
   * @brief Destroy subscriptions for data sources
   */
  Q_INVOKABLE void destroySubscriptions();

  /**
   * @brief Enable or disable TF-based robot pose updates
   */
  Q_INVOKABLE void setRobotPoseUpdatesEnabled(bool enabled);

  /**
   * @brief Enable or disable path updates from the /plan topic
   */
  Q_INVOKABLE void setPathUpdatesEnabled(bool enabled);

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
   * @brief Register Goal RobotLayerItem created in QML
   *
   * @param item Pointer to RobotLayerItem created in QML
   */
  Q_INVOKABLE void registerGoalRobotLayerItem(QObject* item);

  /**
   * @brief Register PathLayerItem created in QML
   *
   * @param item Pointer to PathLayerItem created in QML
   */
  Q_INVOKABLE void registerPathLayerItem(QObject* item);

  /**
   * @brief Register manual drawing PathLayerItem created in QML
   *
   * @param item Pointer to PathLayerItem created in QML
   */
  Q_INVOKABLE void registerManualDrawPathLayerItem(QObject* item);

  /**
   * @brief Register live PathLayerItem created in QML
   *
   * @param item Pointer to PathLayerItem created in QML
   */
  Q_INVOKABLE void registerLivePathLayerItem(QObject* item);

  /**
   * @brief Set goal robot pose for visualization
   *
   * @param x X coordinate of the goal robot in map frame (meters)
   * @param y Y coordinate of the goal robot in map frame (meters)
   * @param theta Orientation of the goal robot in radians
   */
  Q_INVOKABLE void setGoalRobotPose(double x, double y, double theta);

  /**
   * @brief Clear goal robot pose
   */
  Q_INVOKABLE void clearGoalRobotPose();

  /**
   * @brief Set path points for manual visualization (e.g., compute path result)
   *
   * @param points List of {x,y} maps in map frame
   */
  Q_INVOKABLE void setManualPathPoints(const QVariantList& points);

  /**
   * @brief Clear the manual path visualization
   */
  Q_INVOKABLE void clearManualPath();

  /**
   * @brief Set points for manual freehand drawing visualization
   *
   * @param points List of {x,y} maps in map frame
   */
  void setManualDrawPathPoints(const QVariantList& points);

  /**
   * @brief Clear manual freehand drawing visualization
   */
  void clearManualDrawPath();

  /**
   * @brief Clear only the manual freehand drawing layer, preserving the editor state
   */
  Q_INVOKABLE void clearManualDrawPathVisualization();

  /**
   * @brief Start GUI-side live visualization for a manually followed path
   *
   * @param points List of {x,y} maps in map frame
   */
  Q_INVOKABLE void startManualLivePath(const QVariantList& points);

  /**
   * @brief Stop GUI-side live visualization for a manually followed path
   */
  Q_INVOKABLE void stopManualLivePath();

  /**
   * @brief Register LaserLayerItem created in QML
   *
   * @param item Pointer to LaserLayerItem created in QML
   */
  Q_INVOKABLE void registerLaserLayerItem(QObject* item);

  /**
   * @brief Register ParticleCloudLayerItem created in QML
   *
   * @param item Pointer to ParticleCloudLayerItem created in QML
   */
  Q_INVOKABLE void registerParticleCloudLayerItem(QObject* item);

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
   *
   * @param map_name Name of the map
   *
   * @return true if the preview was generated successfully, false otherwise
   */
  Q_INVOKABLE bool generateMapPreview(const QString& map_name);

  /**
   * @brief Clear the current map
   */
  Q_INVOKABLE void clearMap();

  /**
   * @brief Clear the current particle cloud
   */
  Q_INVOKABLE void resetParticleCloud();

private slots:
  void onFrameReady(); // Slot for handling frame readiness

signals:
  void isInitializedChanged();          // Emitted when initialization state changes
  void mapAvailableChanged();           // Emitted when map availability changes
  void robotPoseAvailableChanged();     // Emitted when robot pose availability changes
  void laserAvailableChanged();         // Emitted when laser availability changes
  void particleCloudAvailableChanged(); // Emitted when particle cloud availability changes
  void zoomLevelChanged();              // Emitted when zoom level changes
  void viewTransformChanged();          // Emitted when camera transform (pan/zoom/center) changes
  void mapResolutionChanged();          // Emitted when map resolution changes
  void scaleChanged();                  // Emitted when scale changes
  void followRobotChanged();            // Emitted when follow mode changes

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

  /**
   * @brief Update remaining live path for manually followed paths
   */
  void updateManualLivePath();

  /**
   * @brief Convert a QVariantList of map points into PathData points
   */
  std::vector<ROBOGait::map::data::PathData::PathPoint> pathPointsFromVariantList(const QVariantList& points) const;

  /**
   * @brief Repaint the live path layer after data changes
   */
  void updateLivePathLayer();

  rclcpp::Node* parent_node_; /**< Parent ROS node pointer */

  std::shared_ptr<ROBOGait::map::rendering::RenderScene> render_scene_;       /**< Render scene  */
  std::shared_ptr<ROBOGait::map::rendering::RenderCamera> render_camera_;     /**< Shared camera for layers */
  std::shared_ptr<ROBOGait::map::layer::MapLayer> map_layer_;                 /**< Map layer renderer */
  std::shared_ptr<ROBOGait::map::layer::RobotLayer> robot_layer_;             /**< Robot layer renderer */
  std::shared_ptr<ROBOGait::map::layer::RobotLayer> goal_robot_layer_;        /**< Goal robot layer renderer */
  std::shared_ptr<ROBOGait::map::layer::PathLayer> path_layer_;               /**< Preview path layer renderer */
  std::shared_ptr<ROBOGait::map::layer::PathLayer> manual_draw_path_layer_;   /**< Manual drawing path layer renderer */
  std::shared_ptr<ROBOGait::map::layer::PathLayer> live_path_layer_;          /**< Live path layer renderer (/plan) */
  std::shared_ptr<ROBOGait::map::layer::LaserLayer> laser_layer_;             /**< Laser layer renderer */
  std::shared_ptr<ROBOGait::map::layer::ParticleCloudLayer> particle_layer_;  /**< Particle cloud layer renderer */
  QPointer<ROBOGait::map::item::MapLayerItem> map_layer_item_;                /**< Map layer item */
  QPointer<ROBOGait::map::item::RobotLayerItem> robot_layer_item_;            /**< Robot layer item */
  QPointer<ROBOGait::map::item::RobotLayerItem> goal_robot_layer_item_;       /**< Goal robot layer item */
  QPointer<ROBOGait::map::item::PathLayerItem> path_layer_item_;              /**< Preview path layer item */
  QPointer<ROBOGait::map::item::PathLayerItem> manual_draw_path_layer_item_;  /**< Manual drawing path layer item */
  QPointer<ROBOGait::map::item::PathLayerItem> live_path_layer_item_;         /**< Live path layer item */
  QPointer<ROBOGait::map::item::LaserLayerItem> laser_layer_item_;            /**< Laser layer item */
  QPointer<ROBOGait::map::item::ParticleCloudLayerItem> particle_layer_item_; /**< Particle cloud layer item */

  std::shared_ptr<ROBOGait::map::source::MapSource> map_source_;                     /**< Map source */
  std::shared_ptr<ROBOGait::map::source::RobotPoseSource> pose_source_;              /**< Robot pose source */
  std::shared_ptr<ROBOGait::map::data::RobotPoseData> goal_robot_pose_data_;         /**< Goal robot pose data */
  std::shared_ptr<ROBOGait::map::data::PathData> manual_path_data_;                  /**< Preview path data */
  std::shared_ptr<ROBOGait::map::data::PathData> manual_draw_path_data_;             /**< Manual drawing path data */
  std::shared_ptr<ROBOGait::map::source::PathSource> path_source_;                   /**< Path source */
  std::shared_ptr<ROBOGait::map::source::LaserSource> laser_source_;                 /**< Laser source */
  std::shared_ptr<ROBOGait::map::source::ParticleCloudSource> particle_source_;      /**< Particle cloud source */
  std::unique_ptr<ROBOGait::map::interaction::SplinePathEditor> spline_path_editor_; /**< Spline path editor */
  std::unique_ptr<ROBOGait::map::history::TracedRouteHistory> traced_route_history_; /**< Traced route history */

  QString selected_robot_namespace_;                                              /**< Selected robot namespace */
  bool use_namespace_discovery_;                                                  /**< Use namespace-based topic discovery */
  bool is_initialized_;                                                           /**< Initialization flag */
  bool subscriptions_active_;                                                     /**< Subscriptions active flag */
  bool layers_active_;                                                            /**< Render layers active flag */
  bool map_available_cache_;                                                      /**< Cached map availability state */
  bool robot_pose_available_cache_;                                               /**< Cached robot pose availability state */
  bool laser_available_cache_;                                                    /**< Cached laser availability state */
  bool particle_cloud_available_cache_;                                           /**< Cached particle cloud availability state */
  double map_resolution_cache_;                                                   /**< Cached map resolution (meters per pixel) */
  double scale_meters_cache_;                                                     /**< Cached scale bar meters value */
  int scale_pixels_cache_;                                                        /**< Cached scale bar pixel length */
  double robot_size_;                                                             /**< Robot diameter used for rendering (meters) */
  bool follow_robot_;                                                             /**< Whether the camera follows the robot */
  std::vector<ROBOGait::map::data::PathData::PathPoint> manual_live_path_points_; /**< Manual path used for GUI-side live progress */
  std::size_t manual_live_path_progress_index_;                                   /**< Current progress index in manual live path */
  uint64_t manual_live_path_last_pose_stamp_;                                     /**< Last pose stamp used to update manual live path */
  bool manual_live_path_enabled_;                                                 /**< Whether manual live path progress is active */
};

} // namespace manager
} // namespace map
} // namespace ROBOGait