#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include <QMutex>
#include <QObject>
#include <QTimer>

#include "Map/Layer/LayerInterface.hpp"

namespace ROBOGait
{
namespace map
{
namespace rendering
{
/**
 * @brief Fixed-rate update loop that coordinates when a frame is rendered
 */
class RenderPipeline : public QObject
{
  Q_OBJECT

public:
  explicit RenderPipeline(QObject* parent = nullptr);
  ~RenderPipeline() override;

  /**
   * @brief Start the update loop
   */
  void startUpdate();

  /**
   * @brief Stop the update loop
   */
  void stopUpdate();

  /**
   * @brief Add a layer to the pipeline
   *
   * @param name Name of the layer
   * @param layer the layer to add
   */
  void addLayer(const std::string& name, std::shared_ptr<ROBOGait::map::layer::LayerInterface> layer);

  /**
   * @brief Remove a selected layer from the pipeline
   *
   * @param name Name of the layer
   */
  void removeLayer(const std::string& name);
  void removeAllLayers();

  /**
   * @brief Check if the update loop is running
   *
   * @return True if the update loop is running, false otherwise
   */
  bool isRunning() const;

signals:
  void frameReady(double fps); // Emitted when a new frame is ready
  void updateStarted();        // Emitted when the update loop starts
  void updateStopped();        // Emitted when the update loop stops

private slots:
  void onUpdateTimer(); // Called on each update tick

private:
  void updateLayers();
  void renderFrame();
  bool anyLayerNeedsRender() const;
  void clearLayerRenderRequests();

  QTimer* update_timer_; /**< Timer for the update loop */

  std::map<std::string, std::shared_ptr<ROBOGait::map::layer::LayerInterface>> layer_map_; /**< Map of layer names to layer instances */

  std::chrono::steady_clock::time_point last_update_; /**< Time point of the last update */
  std::chrono::steady_clock::time_point last_render_; /**< Time point of the last render */

  bool render_requested_; /**< Flag indicating if a render is requested */
  QMutex render_mutex_;   /**< Mutex for synchronizing access to render_requested_ */

  static constexpr int UPDATE_RATE_MS = 33;                   /**< Update rate in milliseconds (30Hz) */
  static constexpr double FORCE_RENDER_INTERVAL_SEC = 0.1;    /**< Force render if no render has occurred within this interval (0.1s) */
  static constexpr double FPS_CALCULATION_INTERVAL_SEC = 1.0; /**< Interval for calculating FPS (1 second) */
};

} // namespace rendering
} // namespace map
} // namespace ROBOGait
