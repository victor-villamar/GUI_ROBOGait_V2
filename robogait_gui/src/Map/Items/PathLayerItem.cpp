#include <QDebug>
#include <QSGFlatColorMaterial>
#include <QSGTransformNode>

#include "Map/Items/PathLayerItem.hpp"

using namespace ROBOGait::map::item;

PathLayerItem::PathLayerItem(QQuickItem* parent) : QQuickItem(parent), path_color_(DEFAULT_PATH_COLOR)
{
  setFlag(ItemHasContents, true);
  setAcceptedMouseButtons(Qt::NoButton);
  setAcceptHoverEvents(false);
  setAcceptTouchEvents(false);
  qInfo() << "[PathLayerItem::PathLayerItem] PathLayerItem created";
}

PathLayerItem::~PathLayerItem() { qInfo() << "[PathLayerItem::~PathLayerItem] PathLayerItem destroyed"; }

void PathLayerItem::setRenderScene(const std::shared_ptr<ROBOGait::map::rendering::RenderScene>& scene)
{
  if (pipeline_)
  {
    disconnect(pipeline_, nullptr, this, nullptr);
  }

  render_scene_ = scene;
  pipeline_ = nullptr;

  if (render_scene_ && render_scene_->getPipeline())
  {
    pipeline_ = render_scene_->getPipeline().get();

    // clang-format off
    connect(pipeline_,
            &ROBOGait::map::rendering::RenderPipeline::frameReady,
            this,
            &PathLayerItem::onFrameReady);
    // clang-format on
  }
}

void PathLayerItem::setRenderer(const std::shared_ptr<ROBOGait::map::layer::PathLayer>& path_render)
{
  path_render_ = path_render;
  update();
}

void PathLayerItem::setCamera(const std::shared_ptr<ROBOGait::map::rendering::RenderCamera>& camera)
{
  camera_ = camera;
  update();
}

QColor PathLayerItem::getPathColor() const { return path_color_; }

void PathLayerItem::setPathColor(const QColor& color)
{
  if (path_color_ == color)
  {
    return;
  }

  path_color_ = color;
  emit pathColorChanged();
  update();
}

QSGNode* PathLayerItem::updatePaintNode(QSGNode* old_node, UpdatePaintNodeData* data)
{
  Q_UNUSED(data);

  if (!path_render_ || !camera_)
  {
    delete old_node;
    return nullptr;
  }

  const auto points = path_render_->getPoints();
  if (points.size() < 2)
  {
    delete old_node;
    return nullptr;
  }

  auto* transform_node = static_cast<QSGTransformNode*>(old_node);
  if (!transform_node)
  {
    transform_node = new QSGTransformNode();
  }

  QSGGeometryNode* path_node = nullptr;
  if (!transform_node->firstChild())
  {
    path_node = new QSGGeometryNode();
    auto* geom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
    geom->setDrawingMode(QSGGeometry::DrawLineStrip);
    geom->setLineWidth(LINE_WIDTH);
    path_node->setGeometry(geom);
    path_node->setFlag(QSGNode::OwnsGeometry);

    auto* material = new QSGFlatColorMaterial();
    material->setColor(path_color_);
    material->setFlag(QSGMaterial::Blending, true);
    path_node->setMaterial(material);
    path_node->setFlag(QSGNode::OwnsMaterial);

    transform_node->appendChildNode(path_node);
  }
  else
  {
    path_node = static_cast<QSGGeometryNode*>(transform_node->firstChild());
  }

  auto* material = static_cast<QSGFlatColorMaterial*>(path_node->material());
  if (material)
  {
    material->setColor(path_color_);
    path_node->markDirty(QSGNode::DirtyMaterial);
  }

  QSGGeometry* geometry = path_node->geometry();
  const int vertex_count = static_cast<int>(points.size());
  geometry->allocate(vertex_count);
  geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
  geometry->setLineWidth(LINE_WIDTH);

  auto* vertices = geometry->vertexDataAsPoint2D();
  for (int i = 0; i < static_cast<int>(points.size()); ++i)
  {
    vertices[i].set(static_cast<float>(points[i].x), static_cast<float>(points[i].y));
  }

  path_node->markDirty(QSGNode::DirtyGeometry);

  camera_->setViewportSize(QSizeF(width(), height()));
  transform_node->setMatrix(camera_->getMatrix());

  return transform_node;
}

void PathLayerItem::onFrameReady() { update(); }
