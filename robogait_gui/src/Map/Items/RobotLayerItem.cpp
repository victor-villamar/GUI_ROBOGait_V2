#include <QDebug>
#include <QMatrix4x4>
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

  QSGTransformNode* robot_node = nullptr;
  QSGGeometryNode* body_node = nullptr;
  QSGGeometryNode* wheels_node = nullptr;
  QSGGeometryNode* head_node = nullptr;

  if (!transform_node->firstChild())
  {
    robot_node = new QSGTransformNode();
    transform_node->appendChildNode(robot_node);
  }
  else
  {
    robot_node = static_cast<QSGTransformNode*>(transform_node->firstChild());
  }

  if (!robot_node->firstChild())
  {
    body_node = new QSGGeometryNode();
    auto* body_geom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
    body_geom->setDrawingMode(QSGGeometry::DrawTriangles);
    body_node->setGeometry(body_geom);
    body_node->setFlag(QSGNode::OwnsGeometry);

    auto* body_mat = new QSGFlatColorMaterial();
    body_mat->setColor(BODY_COLOR);
    body_node->setMaterial(body_mat);
    body_node->setFlag(QSGNode::OwnsMaterial);
    robot_node->appendChildNode(body_node);

    wheels_node = new QSGGeometryNode();
    auto* wheels_geom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
    wheels_geom->setDrawingMode(QSGGeometry::DrawTriangles);
    wheels_node->setGeometry(wheels_geom);
    wheels_node->setFlag(QSGNode::OwnsGeometry);

    auto* wheels_mat = new QSGFlatColorMaterial();
    wheels_mat->setColor(WHEEL_COLOR);
    wheels_node->setMaterial(wheels_mat);
    wheels_node->setFlag(QSGNode::OwnsMaterial);
    robot_node->appendChildNode(wheels_node);

    head_node = new QSGGeometryNode();
    auto* head_geom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
    head_geom->setDrawingMode(QSGGeometry::DrawTriangles);
    head_node->setGeometry(head_geom);
    head_node->setFlag(QSGNode::OwnsGeometry);

    auto* head_mat = new QSGFlatColorMaterial();
    head_mat->setColor(HEAD_COLOR);
    head_node->setMaterial(head_mat);
    head_node->setFlag(QSGNode::OwnsMaterial);
    robot_node->appendChildNode(head_node);
  }
  else
  {
    body_node = static_cast<QSGGeometryNode*>(robot_node->firstChild());
    wheels_node = static_cast<QSGGeometryNode*>(body_node->nextSibling());
    head_node = static_cast<QSGGeometryNode*>(wheels_node->nextSibling());
  }

  const ROBOGait::map::layer::RobotLayer::Pose2D pose = robot_render_->getInterpolatedPose();
  const double size = robot_render_->getRobotSize();
  const double body_w = size * 0.72;
  const double body_h = size * 0.48;
  const double wheel_w = size * 0.2;
  const double wheel_h = size * 0.1;
  const double connector_w = wheel_w * 0.4;
  const double head_w = size * 0.18;
  const double head_h = size * 0.07;

  QSGGeometry* body_geometry = body_node->geometry();
  body_geometry->allocate(6);
  auto* body_vertices = body_geometry->vertexDataAsPoint2D();
  writeRect(body_vertices, 0, 0.0f, 0.0f, static_cast<float>(body_w), static_cast<float>(body_h));

  QSGGeometry* wheels_geometry = wheels_node->geometry();
  wheels_geometry->allocate(48);
  auto* wheel_vertices = wheels_geometry->vertexDataAsPoint2D();

  const float wheel_x = static_cast<float>(body_w * 0.3);
  const float wheel_y = static_cast<float>(size * 0.5 - wheel_h * 0.5);

  writeRect(wheel_vertices, 0, -wheel_x, wheel_y, static_cast<float>(wheel_w), static_cast<float>(wheel_h));
  writeRect(wheel_vertices, 6, wheel_x, wheel_y, static_cast<float>(wheel_w), static_cast<float>(wheel_h));
  writeRect(wheel_vertices, 12, -wheel_x, -wheel_y, static_cast<float>(wheel_w), static_cast<float>(wheel_h));
  writeRect(wheel_vertices, 18, wheel_x, -wheel_y, static_cast<float>(wheel_w), static_cast<float>(wheel_h));

  const float wheel_inner_y = static_cast<float>(wheel_y - (wheel_h * 0.5));
  const float body_edge_y = static_cast<float>(body_h * 0.5);
  float connector_h = wheel_inner_y - body_edge_y;
  if (connector_h < 0.0f)
  {
    connector_h = 0.0f;
  }
  const float connector_y = body_edge_y + (connector_h * 0.5f);

  writeRect(wheel_vertices, 24, -wheel_x, connector_y, static_cast<float>(connector_w), connector_h);
  writeRect(wheel_vertices, 30, wheel_x, connector_y, static_cast<float>(connector_w), connector_h);
  writeRect(wheel_vertices, 36, -wheel_x, -connector_y, static_cast<float>(connector_w), connector_h);
  writeRect(wheel_vertices, 42, wheel_x, -connector_y, static_cast<float>(connector_w), connector_h);

  QSGGeometry* head_geometry = head_node->geometry();
  head_geometry->allocate(6);
  auto* head_vertices = head_geometry->vertexDataAsPoint2D();
  const float head_x = static_cast<float>(body_w * 0.5 - head_w * 0.5 - size * 0.02);
  writeRect(head_vertices, 0, head_x, 0.0f, static_cast<float>(head_w), static_cast<float>(head_h));

  body_node->markDirty(QSGNode::DirtyGeometry);
  wheels_node->markDirty(QSGNode::DirtyGeometry);
  head_node->markDirty(QSGNode::DirtyGeometry);

  if (camera_)
  {
    camera_->setViewportSize(QSizeF(width(), height()));
    transform_node->setMatrix(camera_->getMatrix());
  }

  const float cx = static_cast<float>(pose.x);
  const float cy = static_cast<float>(pose.y);
  const double yaw = pose.yaw;

  QMatrix4x4 robot_matrix;
  robot_matrix.translate(cx, cy);
  robot_matrix.rotate(qRadiansToDegrees(yaw), 0.0f, 0.0f, 1.0f);
  robot_node->setMatrix(robot_matrix);

  return transform_node;
}

void RobotLayerItem::onFrameReady() { update(); }

void RobotLayerItem::writeRect(QSGGeometry::Point2D* vertices, int start, float cx, float cy, float w, float h)
{
  const float x0 = cx - w * 0.5f;
  const float x1 = cx + w * 0.5f;
  const float y0 = cy - h * 0.5f;
  const float y1 = cy + h * 0.5f;
  vertices[start + 0].set(x0, y0);
  vertices[start + 1].set(x1, y0);
  vertices[start + 2].set(x1, y1);
  vertices[start + 3].set(x0, y0);
  vertices[start + 4].set(x1, y1);
  vertices[start + 5].set(x0, y1);
}
