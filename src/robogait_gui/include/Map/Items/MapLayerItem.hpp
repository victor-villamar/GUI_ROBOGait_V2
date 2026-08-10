#pragma once

#include <memory>

#include <QPointer>
#include <QQuickItem>
#include <QRectF>
#include <QSGSimpleTextureNode>
#include <QVector>

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
  // clang-format off
  Q_PROPERTY(bool isPanningEnabled
             READ isPanningEnabled
             WRITE setPanningEnabled
             NOTIFY panningEnabledChanged)
  // clang-format on

  /**
   * @brief Constructor for the MapLayerItem class
   */
  explicit MapLayerItem(QQuickItem* parent = nullptr);

  /**
   * @brief Destructor for the MapLayerItem class
   */
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

  /**
   * @brief Register an item to be updated in sync with map interactions
   *
   * @param item The sync item to register
   */
  void setSyncItem(QQuickItem* item);

  /**
   * @brief Check if panning is enabled
   *
   * @return True if panning is enabled, false otherwise
   */
  bool isPanningEnabled() const;

  /**
   * @brief Enable or disable panning behavior
   *
   * @param enabled True to enable panning, false to disable
   */
  void setPanningEnabled(bool enabled);

signals:
  void zoomChanged();           // Emitted when the zoom level changes
  void viewTransformChanged();  // Emitted when map camera transform changes (pan/zoom/fit)
  void panningEnabledChanged(); // Emitted when the panning enabled state changes

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
  /**
   * @brief Clamp the camera view center to the map boundaries
   *
   * @param center The camera view center to clamp
   *
   * @return The clamped camera view center
   */
  QPointF clampCenterToMap(const QPointF& center) const;

  /**
   * @brief Apply a pan delta to the camera view
   *
   * @param delta The pan delta to apply
   */
  void applyPanDelta(const QPointF& delta);

  /**
   * @brief Request update on all synchronized overlay items
   */
  void updateSyncItems();

  QPointer<ROBOGait::map::rendering::RenderPipeline> pipeline_;         /**< Render pipeline for the map item */
  std::shared_ptr<ROBOGait::map::rendering::RenderScene> render_scene_; /**< Render scene for the map item */
  std::shared_ptr<ROBOGait::map::layer::MapLayer> map_render_;          /**< Map layer for the map item */
  std::shared_ptr<ROBOGait::map::rendering::RenderCamera> camera_;      /**< Camera for the map item */
  QVector<QPointer<QQuickItem>> sync_items_;                            /**< Sync items updated together with map interactions */
  qint64 last_image_key_;                                               /**< Last image key for the map item */
  QRectF map_rect_;                                                     /**< Last map rect in world coordinates */
  bool has_map_rect_;                                                   /**< Map rect availability flag */

  bool fit_done_;              /**< Fit done flag for the map item */
  bool is_panning_;            /**< Is panning flag for the map item */
  bool panning_enabled_;       /**< Enable/disable panning behavior */
  QPointF last_pan_pos_;       /**< Last pan position for the map item */
  bool is_pinching_;           /**< Is pinching flag for the map item */
  qreal pinch_start_zoom_;     /**< Pinch start zoom for the map item */
  qreal pinch_start_distance_; /**< Pinch start distance for the map item */
};

} // namespace item
} // namespace map
} // namespace ROBOGait
