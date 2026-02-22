#pragma once

#include <QGraphicsScene>
#include <QMutex>
#include <QObject>
#include <QTimer>
#include <chrono>
#include <map>
#include <memory>
#include <vector>

#include "Map/Display/BaseDisplay.hpp"

namespace ROBOGait
{
namespace map
{
namespace rendering
{

/**
 * @brief Central orchestrator for visualization displays
 *
 * This class manages the update/render loop and coordinates multiple display layers.
 */
class VisualizationManager : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Constructor of VisualizationManager class
   *
   * @param parent Qt parent object
   */
  VisualizationManager(QObject* parent = nullptr);

  /**
   * @brief Destructor of VisualizationManager class
   */
  ~VisualizationManager();

  /**
   * @brief Initialize with pointer to ROS2 parent node
   *
   * @param parent_node Pointer to the parent ROS2 node
   */
  void initialize(rclcpp::Node* parent_node);

  /**
   * @brief Start the update/render loop
   *
   */
  void startUpdate();

  /**
   * @brief Stop the update/render loop
   */
  void stopUpdate();

  /**
   * @brief Check if manager is initialized
   *
   * @return True if initialize() was called successfully, false otherwise
   */
  bool isInitialized() const { return is_initialized_; }

  /**
   * @brief Add a display to the manager
   *
   * @param name Display name for identification
   * @param display The display to add
   */
  void addDisplay(const std::string& name, std::shared_ptr<display::BaseDisplay> display);

  /**
   * @brief Remove a display by name
   *
   * @param name Display name
   */
  void removeDisplay(const std::string& name);

  /**
   * @brief Remove all displays
   */
  void removeAllDisplays();

  /**
   * @brief Get a display by name
   *
   * @param name Display name
   *
   * @return The display, or nullptr if not found
   */
  std::shared_ptr<display::BaseDisplay> getDisplay(const std::string& name);

  /**
   * @brief Get the graphics scene
   *
   * @return Pointer to QGraphicsScene
   */
  QGraphicsScene* getScene() const;

  /**
   * @brief Request a render on next update
   */
  void queueRender();

signals:
  void frameRendered(double fps); // Signal emitted after each frame is rendered
  void updateStarted();           // Signal emitted when update loop starts
  void updateStopped();           // Signal emitted when update loop stops

private slots:
  void onUpdateTimer();     // Update timer callback (called at ~30 Hz)
  void onRenderRequested(); // Handle render request from displays

private:
  /**
   * @brief Update all enabled displays
   *
   * @param wall_dt Wall clock delta time (seconds)
   * @param ros_dt ROS time delta time (seconds)
   */
  void updateDisplays(double wall_dt, double ros_dt);

  /**
   * @brief Trigger a render frame
   */
  void renderFrame();

  rclcpp::Node* parent_node_;                                                /**< Pointer to the parent ROS2 node */
  QTimer* update_timer_;                                                     /**< Update loop timer */
  std::map<std::string, std::shared_ptr<display::BaseDisplay>> display_map_; /**< Map of displays by name */
  std::unique_ptr<QGraphicsScene> scene_;                                    /**< Graphics scene for rendering */

  std::chrono::steady_clock::time_point last_update_; /**< Last update timestamp */
  std::chrono::steady_clock::time_point last_render_; /**< Last render timestamp */

  bool is_initialized_;   /**< Whether initialize() was called */
  bool render_requested_; /**< Whether render was requested */
  QMutex render_mutex_;   /**< Mutex for render state */

  static constexpr int UPDATE_RATE_MS = 33;                /**< Update rate (30 Hz) */
  static constexpr double FORCE_RENDER_INTERVAL_SEC = 0.1; /**< Force render interval (100 ms) */
};

} // namespace rendering
} // namespace map
} // namespace ROBOGait
