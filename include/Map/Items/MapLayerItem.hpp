#pragma once

#include <memory>

#include <QPointer>
#include <QQuickItem>
#include <QSGSimpleTextureNode>

#include "Map/Layer/MapLayer.hpp"
#include "Map/Rendering/RenderCamera.hpp"
#include "Map/Rendering/RenderScene.hpp"

namespace ROBOGait
{
namespace map
{
namespace item
{

/**
 * @brief QtQuickItem that renders the map texture in the scene graph
 */
class MapLayerItem : public QQuickItem
{
  Q_OBJECT

public:
  explicit MapLayerItem(QQuickItem* parent = nullptr);

  ~MapLayerItem();

  /**
   * @brief Set the render scene to use for rendering
   *
   * @param scene The render scene to use
   */
  void setRenderScene(const std::shared_ptr<ROBOGait::map::rendering::RenderScene>& scene);

  /**
   * @brief Set the map layer to use for rendering
   *
   * @param renderer The map layer to use
   */
  void setRenderer(const std::shared_ptr<ROBOGait::map::layer::MapLayer>& map_render);

  /**
   * @brief Set the camera to use for rendering
   *
   * @param camera The camera to use
   */
  void setCamera(const std::shared_ptr<ROBOGait::map::rendering::RenderCamera>& camera);

  void setSyncItem(QQuickItem* item);

signals:
  void zoomChanged(); // Emitted when the zoom level changes

protected:
  /**
   * @brief Update the paint node for the map layer item
   *
   * @param old_node The old paint node
   * @param data The update paint node data
   * @return The updated paint node
   */
  QSGNode* updatePaintNode(QSGNode* old_node, UpdatePaintNodeData* data) override;

  // Event handlers
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void touchEvent(QTouchEvent* event) override;
  bool event(QEvent* event) override;

private slots:
  void onFrameReady(); // Slot for frame ready signal

private:
  QPointer<ROBOGait::map::rendering::RenderPipeline> pipeline_;         /**< Render pipeline for the map item */
  std::shared_ptr<ROBOGait::map::rendering::RenderScene> render_scene_; /**< Render scene for the map item */
  std::shared_ptr<ROBOGait::map::layer::MapLayer> map_render_;          /**< Map layer for the map item */
  std::shared_ptr<ROBOGait::map::rendering::RenderCamera> camera_;      /**< Camera for the map item */
  QPointer<QQuickItem> sync_item_;                                      /**< Sync item for the map item */
  qint64 last_image_key_;                                               /**< Last image key for the map item */

  bool fit_done_;          /**< Fit done flag for the map item */
  bool is_panning_;        /**< Is panning flag for the map item */
  QPointF last_pan_pos_;   /**< Last pan position for the map item */
  qreal pinch_start_zoom_; /**< Pinch start zoom for the map item */
};

} // namespace item
} // namespace map
} // namespace ROBOGait
