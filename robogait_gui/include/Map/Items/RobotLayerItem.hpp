#pragma once

#include <memory>

#include <QPointer>
#include <QQuickItem>
#include <QSGGeometryNode>

#include "Map/Layer/RobotLayer.hpp"
#include "Map/Rendering/RenderCamera.hpp"
#include "Map/Rendering/RenderScene.hpp"

namespace ROBOGait
{
namespace map
{
namespace item
{

/**
 * @brief QtQuickItem that renders the robot geometry in the scene graph
 */
class RobotLayerItem : public QQuickItem
{
  Q_OBJECT

public:
  explicit RobotLayerItem(QQuickItem* parent = nullptr);

  ~RobotLayerItem();

  /**
   * @brief Sets the render scene to use for rendering
   *
   * @param scene The render scene to use
   */
  void setRenderScene(const std::shared_ptr<ROBOGait::map::rendering::RenderScene>& scene);

  /**
   * @brief Sets the robot layer to use for rendering
   *
   * @param robot_renderer The robot layer to use
   */
  void setRenderer(const std::shared_ptr<ROBOGait::map::layer::RobotLayer>& robot_render);

  /**
   * @brief Sets the camera to use for rendering
   *
   * @param camera The camera to use
   */
  void setCamera(const std::shared_ptr<ROBOGait::map::rendering::RenderCamera>& camera);

protected:
  /**
   * @brief Updates the paint node for the robot layer
   *
   * @param old_node The old paint node
   * @param data The update paint node data
   * @return The updated paint node
   */
  QSGNode* updatePaintNode(QSGNode* old_node, UpdatePaintNodeData* data) override;

private slots:
  void onFrameReady(); // Slot for frame ready signal

private:
  /**
   * @brief Writes a rectangle to the given vertex buffer
   *
   * @param vertices The vertex buffer to write to
   * @param start The starting index in the vertex buffer
   * @param cx The center x-coordinate of the rectangle
   * @param cy The center y-coordinate of the rectangle
   * @param w The width of the rectangle
   * @param h The height of the rectangle
   */
  static void writeRect(QSGGeometry::Point2D* vertices, int start, float cx, float cy, float w, float h);

  QPointer<ROBOGait::map::rendering::RenderPipeline> pipeline_;         /**< Render pipeline for the robot item */
  std::shared_ptr<ROBOGait::map::rendering::RenderScene> render_scene_; /**< Render scene for the robot item */
  std::shared_ptr<ROBOGait::map::layer::RobotLayer> robot_render_;      /**< Robot layer for the robot item */
  std::shared_ptr<ROBOGait::map::rendering::RenderCamera> camera_;      /**< Camera for the robot item */
};

} // namespace item
} // namespace map
} // namespace ROBOGait
