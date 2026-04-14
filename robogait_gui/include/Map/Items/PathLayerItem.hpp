#pragma once

#include <memory>

#include <QColor>
#include <QPointer>
#include <QQuickItem>
#include <QSGGeometryNode>

#include "Map/Layer/PathLayer.hpp"
#include "Map/Rendering/RenderCamera.hpp"
#include "Map/Rendering/RenderScene.hpp"

namespace ROBOGait
{
namespace map
{
namespace item
{
/**
 * @brief QtQuickItem that renders the path in the scene graph
 */
class PathLayerItem : public QQuickItem
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor pathColor
             READ getPathColor
             WRITE setPathColor
             NOTIFY pathColorChanged)
  // clang-format on

  /**
   * @brief Constructor for the PathLayerItem class
   */
  explicit PathLayerItem(QQuickItem* parent = nullptr);

  /**
   * @brief Destructor for the PathLayerItem class
   */
  ~PathLayerItem();

  /**
   * @brief Set the render scene to use for rendering
   *
   * @param scene The render scene
   */
  void setRenderScene(const std::shared_ptr<ROBOGait::map::rendering::RenderScene>& scene);

  /**
   * @brief Set the path layer to use for rendering
   *
   * @param path_render The path layer to use
   */
  void setRenderer(const std::shared_ptr<ROBOGait::map::layer::PathLayer>& path_render);

  /**
   * @brief Set the camera to use for rendering
   *
   * @param camera The camera to use
   */
  void setCamera(const std::shared_ptr<ROBOGait::map::rendering::RenderCamera>& camera);

  /**
   * @brief Get the path color
   *
   * @return Current path color
   */
  QColor getPathColor() const;

  /**
   * @brief Set the path color
   *
   * @param color New color for the path
   */
  void setPathColor(const QColor& color);

protected:
  /**
   * @brief Update the paint node for the path layer item
   *
   * @param old_node The old paint node
   * @param data The update paint node data
   * @return The updated paint node
   */
  QSGNode* updatePaintNode(QSGNode* old_node, UpdatePaintNodeData* data) override;

signals:
  void pathColorChanged();

private slots:
  void onFrameReady(); // Slot to handle frame ready signal

private:
  QPointer<ROBOGait::map::rendering::RenderPipeline> pipeline_;         /**< Render pipeline for the path item */
  std::shared_ptr<ROBOGait::map::rendering::RenderScene> render_scene_; /**< Render scene for the path item */
  std::shared_ptr<ROBOGait::map::layer::PathLayer> path_render_;        /**< Path layer for the path item */
  std::shared_ptr<ROBOGait::map::rendering::RenderCamera> camera_;      /**< Camera for the path item */
  QColor path_color_;                                                   /**< Color of the path */

  static constexpr float LINE_WIDTH = 3.0f;                        /**< Width of the path line */
  static constexpr auto DEFAULT_PATH_COLOR = QColor(145, 24, 219); /**< Default color of the path */
};

} // namespace item
} // namespace map
} // namespace ROBOGait
