#pragma once

#include <memory>

#include <QPointer>
#include <QQuickItem>
#include <QSGGeometryNode>

#include "Map/Layer/LaserLayer.hpp"
#include "Map/Rendering/RenderCamera.hpp"
#include "Map/Rendering/RenderScene.hpp"

namespace ROBOGait
{
namespace map
{
namespace item
{
/**
 * @brief QtQuickItem that renders the laser scan points in the scene graph
 */
class LaserLayerItem : public QQuickItem
{
  Q_OBJECT

public:
  /**
   * @brief Constructor for the LaserLayerItem class
   */
  explicit LaserLayerItem(QQuickItem* parent = nullptr);

  /**
   * @brief Destructor for the LaserLayerItem class
   */
  ~LaserLayerItem();

  /**
   * @brief Set the render scene to use for rendering
   *
   * @param scene The render scene
   */
  void setRenderScene(const std::shared_ptr<ROBOGait::map::rendering::RenderScene>& scene);

  /**
   * @brief Set the laser layer to use for rendering
   *
   * @param laser_render The laser layer to use
   */
  void setRenderer(const std::shared_ptr<ROBOGait::map::layer::LaserLayer>& laser_render);

  /**
   * @brief Set the camera to use for rendering
   *
   * @param camera The camera to use
   */
  void setCamera(const std::shared_ptr<ROBOGait::map::rendering::RenderCamera>& camera);

protected:
  /**
   * @brief Update the paint node for the laser layer item
   *
   * @param old_node The old paint node
   * @param data The update paint node data
   * @return The updated paint node
   */
  QSGNode* updatePaintNode(QSGNode* old_node, UpdatePaintNodeData* data) override;

private slots:
  void onFrameReady(); // Slot to handle frame ready signal

private:
  /**
   * @brief Write a rectangle to the geometry
   *
   * @param vertices The vertex data
   * @param start The start index
   * @param cx The center x position
   * @param cy The center y position
   * @param w The width
   * @param h The height
   */
  static void writeRect(QSGGeometry::Point2D* vertices, int start, float cx, float cy, float w, float h);

  QPointer<ROBOGait::map::rendering::RenderPipeline> pipeline_;         /**< Render pipeline for the laser item */
  std::shared_ptr<ROBOGait::map::rendering::RenderScene> render_scene_; /**< Render scene for the laser item */
  std::shared_ptr<ROBOGait::map::layer::LaserLayer> laser_render_;      /**< Laser layer for the laser item */
  std::shared_ptr<ROBOGait::map::rendering::RenderCamera> camera_;      /**< Camera for the laser item */

  static constexpr float POINT_SIZE = 0.03f; /**< Size of the laser points */
};

} // namespace item
} // namespace map
} // namespace ROBOGait
