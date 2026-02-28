#include <QDebug>
#include <QSGFlatColorMaterial>
#include <QSGTransformNode>
#include <QtMath>

#include "Map/Items/RobotLayerItem.hpp"

using namespace ROBOGait::map::item;

RobotLayerItem::RobotLayerItem(QQuickItem* parent) : QQuickItem(parent)
{
  setFlag(ItemHasContents, true);
  qInfo() << "[RobotLayerItem::RobotLayerItem] RobotLayerItem created";
}

RobotLayerItem::~RobotLayerItem() { qInfo() << "[RobotLayerItem::~RobotLayerItem] RobotLayerItem destroyed"; }

void RobotLayerItem::setRenderScene(const std::shared_ptr<ROBOGait::map::rendering::RenderScene>& scene)
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
            &RobotLayerItem::onFrameReady);
    // clang-format on
  }
}

void RobotLayerItem::setRenderer(const std::shared_ptr<ROBOGait::map::layer::RobotLayer>& robot_render)
{
  robot_render_ = robot_render;
  update();
}

void RobotLayerItem::setCamera(const std::shared_ptr<ROBOGait::map::rendering::RenderCamera>& camera)
{
  camera_ = camera;
  update();
}

QSGNode* RobotLayerItem::updatePaintNode(QSGNode* old_node, UpdatePaintNodeData* data)
{
  Q_UNUSED(data);

  if (!robot_render_)
  {
    delete old_node;
    return nullptr;
  }

  auto* transform_node = static_cast<QSGTransformNode*>(old_node);
  if (!transform_node)
  {
    transform_node = new QSGTransformNode();
  }

  QSGGeometryNode* body_node = nullptr;
  QSGGeometryNode* arrow_node = nullptr;

  if (!transform_node->firstChild())
  {
    body_node = new QSGGeometryNode();
    auto* body_geom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
    body_geom->setDrawingMode(QSGGeometry::DrawTriangles);
    body_node->setGeometry(body_geom);
    body_node->setFlag(QSGNode::OwnsGeometry);

    auto* body_mat = new QSGFlatColorMaterial();
    body_mat->setColor(QColor(0, 120, 255, 180));
    body_node->setMaterial(body_mat);
    body_node->setFlag(QSGNode::OwnsMaterial);
    transform_node->appendChildNode(body_node);

    arrow_node = new QSGGeometryNode();
    auto* arrow_geom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
    arrow_geom->setDrawingMode(QSGGeometry::DrawTriangles);
    arrow_node->setGeometry(arrow_geom);
    arrow_node->setFlag(QSGNode::OwnsGeometry);

    auto* arrow_mat = new QSGFlatColorMaterial();
    arrow_mat->setColor(QColor(255, 255, 0, 220));
    arrow_node->setMaterial(arrow_mat);
    arrow_node->setFlag(QSGNode::OwnsMaterial);
    transform_node->appendChildNode(arrow_node);
  }
  else
  {
    body_node = static_cast<QSGGeometryNode*>(transform_node->firstChild());
    arrow_node = static_cast<QSGGeometryNode*>(body_node->nextSibling());
  }

  const ROBOGait::map::layer::RobotLayer::Pose2D pose = robot_render_->getInterpolatedPose();
  const double radius = robot_render_->getRobotSize() * 0.5;
  const int segments = 20;

  const int body_vertex_count = segments * 3;
  QSGGeometry* body_geometry = body_node->geometry();
  body_geometry->allocate(body_vertex_count);
  auto* body_vertices = body_geometry->vertexDataAsPoint2D();

  const float cx = static_cast<float>(pose.x);
  const float cy = static_cast<float>(-pose.y); // Match ROS -> Qt Y flip

  for (int i = 0; i < segments; ++i)
  {
    const double a0 = (static_cast<double>(i) / segments) * 2.0 * M_PI;
    const double a1 = (static_cast<double>(i + 1) / segments) * 2.0 * M_PI;

    const float x0 = cx + static_cast<float>(std::cos(a0) * radius);
    const float y0 = cy + static_cast<float>(std::sin(a0) * radius);
    const float x1 = cx + static_cast<float>(std::cos(a1) * radius);
    const float y1 = cy + static_cast<float>(std::sin(a1) * radius);

    const int base = i * 3;
    body_vertices[base + 0].set(cx, cy);
    body_vertices[base + 1].set(x0, y0);
    body_vertices[base + 2].set(x1, y1);
  }

  const double arrow_length = robot_render_->getRobotSize() * 0.6;
  const double arrow_width = robot_render_->getRobotSize() * 0.3;

  const double yaw = -pose.yaw; // Match ROS -> Qt rotation
  const float tip_x = cx + static_cast<float>(std::cos(yaw) * arrow_length);
  const float tip_y = cy + static_cast<float>(std::sin(yaw) * arrow_length);

  const double left_angle = yaw + M_PI * 0.5;
  const double right_angle = yaw - M_PI * 0.5;
  const float left_x = cx + static_cast<float>(std::cos(left_angle) * (arrow_width * 0.5));
  const float left_y = cy + static_cast<float>(std::sin(left_angle) * (arrow_width * 0.5));
  const float right_x = cx + static_cast<float>(std::cos(right_angle) * (arrow_width * 0.5));
  const float right_y = cy + static_cast<float>(std::sin(right_angle) * (arrow_width * 0.5));

  QSGGeometry* arrow_geometry = arrow_node->geometry();
  arrow_geometry->allocate(3);
  auto* arrow_vertices = arrow_geometry->vertexDataAsPoint2D();
  arrow_vertices[0].set(tip_x, tip_y);
  arrow_vertices[1].set(left_x, left_y);
  arrow_vertices[2].set(right_x, right_y);

  body_node->markDirty(QSGNode::DirtyGeometry);
  arrow_node->markDirty(QSGNode::DirtyGeometry);

  if (camera_)
  {
    camera_->setViewportSize(QSizeF(width(), height()));
    transform_node->setMatrix(camera_->getMatrix());
  }

  return transform_node;
}

void RobotLayerItem::onFrameReady() { update(); }
