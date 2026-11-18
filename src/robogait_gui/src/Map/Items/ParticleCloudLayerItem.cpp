#include <cstddef>

#include <QDebug>
#include <QSGFlatColorMaterial>
#include <QSGTransformNode>

#include "Map/Items/ParticleCloudLayerItem.hpp"
#include "Themes/AppTheme.hpp"

using namespace ROBOGait::map::item;

ParticleCloudLayerItem::ParticleCloudLayerItem(QQuickItem* parent) : QQuickItem(parent), particle_color_(DEFAULT_PARTICLE_COLOR)
{
  const auto& theme = ROBOGait::settings::AppTheme::getInstance();
  if (auto* map_theme = qobject_cast<ROBOGait::settings::ThemeMap*>(theme.getMap()))
  {
    particle_color_ = map_theme->getParticle();
  }

  setFlag(ItemHasContents, true);
  qDebug() << "[ParticleCloudLayerItem::ParticleCloudLayerItem] ParticleCloudLayerItem created";
}

ParticleCloudLayerItem::~ParticleCloudLayerItem() { qDebug() << "[ParticleCloudLayerItem::~ParticleCloudLayerItem] ParticleCloudLayerItem destroyed"; }

void ParticleCloudLayerItem::setRenderScene(const std::shared_ptr<ROBOGait::map::rendering::RenderScene>& scene)
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
            &ParticleCloudLayerItem::onFrameReady);
    // clang-format on
  }
}

void ParticleCloudLayerItem::setRenderer(const std::shared_ptr<ROBOGait::map::layer::ParticleCloudLayer>& particle_render)
{
  particle_render_ = particle_render;
  update();
}

void ParticleCloudLayerItem::setCamera(const std::shared_ptr<ROBOGait::map::rendering::RenderCamera>& camera)
{
  camera_ = camera;
  update();
}

QColor ParticleCloudLayerItem::getParticleColor() const { return particle_color_; }

void ParticleCloudLayerItem::setParticleColor(const QColor& color)
{
  if (particle_color_ != color)
  {
    particle_color_ = color;
    emit particleColorChanged();
    update();
  }
}

QSGNode* ParticleCloudLayerItem::updatePaintNode(QSGNode* old_node, UpdatePaintNodeData* data)
{
  Q_UNUSED(data);

  if (!particle_render_ || !camera_)
  {
    delete old_node;
    return nullptr;
  }

  const auto particles = particle_render_->getParticles();
  if (particles.empty())
  {
    delete old_node;
    return nullptr;
  }

  auto* transform_node = static_cast<QSGTransformNode*>(old_node);
  if (!transform_node)
  {
    transform_node = new QSGTransformNode();
  }

  QSGGeometryNode* points_node = nullptr;
  if (!transform_node->firstChild())
  {
    points_node = new QSGGeometryNode();
    auto* geom = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
    geom->setDrawingMode(QSGGeometry::DrawTriangles);
    points_node->setGeometry(geom);
    points_node->setFlag(QSGNode::OwnsGeometry);

    auto* material = new QSGFlatColorMaterial();
    material->setColor(particle_color_);
    material->setFlag(QSGMaterial::Blending, false);
    points_node->setMaterial(material);
    points_node->setFlag(QSGNode::OwnsMaterial);

    transform_node->appendChildNode(points_node);
  }
  else
  {
    points_node = static_cast<QSGGeometryNode*>(transform_node->firstChild());
  }

  QSGGeometry* geometry = points_node->geometry();
  auto* material = static_cast<QSGFlatColorMaterial*>(points_node->material());
  if (material)
  {
    material->setColor(particle_color_);
    points_node->markDirty(QSGNode::DirtyMaterial);
  }
  const int vertex_count = static_cast<int>(particles.size()) * 6;
  geometry->allocate(vertex_count);
  auto* vertices = geometry->vertexDataAsPoint2D();

  for (size_t i = 0; i < particles.size(); ++i)
  {
    const float cx = static_cast<float>(particles[i].x_);
    const float cy = static_cast<float>(particles[i].y_);
    writeRect(vertices, static_cast<int>(i * 6), cx, cy, POINT_SIZE, POINT_SIZE);
  }

  points_node->markDirty(QSGNode::DirtyGeometry);

  camera_->setViewportSize(QSizeF(width(), height()));
  transform_node->setMatrix(camera_->getMatrix());

  return transform_node;
}

void ParticleCloudLayerItem::onFrameReady() { update(); }

void ParticleCloudLayerItem::writeRect(QSGGeometry::Point2D* vertices, int start, float cx, float cy, float w, float h)
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
