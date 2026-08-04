#include <QMatrix4x4>
#include <QPointF>
#include <QRectF>
#include <QSizeF>

#include <gtest/gtest.h>

#include "Map/Rendering/RenderCamera.hpp"

namespace
{
using ROBOGait::map::rendering::RenderCamera;

void expectPointNear(const QPointF& point, double x, double y)
{
  EXPECT_NEAR(point.x(), x, 1e-6);
  EXPECT_NEAR(point.y(), y, 1e-6);
}
} // namespace

TEST(RenderCameraTest, StartsAtOriginWithDefaultZoom)
{
  const RenderCamera camera;

  expectPointNear(camera.getViewCenter(), 0.0, 0.0);
  EXPECT_DOUBLE_EQ(camera.getZoom(), 1.0);
}

TEST(RenderCameraTest, SetZoomClampsToSupportedRange)
{
  RenderCamera camera;

  camera.setZoom(10.0);
  EXPECT_DOUBLE_EQ(camera.getZoom(), 70.0);

  camera.setZoom(500.0);
  EXPECT_DOUBLE_EQ(camera.getZoom(), 470.0);

  camera.setZoom(100.0);
  EXPECT_DOUBLE_EQ(camera.getZoom(), 100.0);
}

TEST(RenderCameraTest, ZoomByFactorUsesCurrentZoomAndClamps)
{
  RenderCamera camera;
  camera.setZoom(100.0);

  camera.zoomByFactor(2.0);
  EXPECT_DOUBLE_EQ(camera.getZoom(), 200.0);

  camera.zoomByFactor(0.01);
  EXPECT_DOUBLE_EQ(camera.getZoom(), 70.0);
}

TEST(RenderCameraTest, FitToRectCentersViewAndUsesViewportLimitedZoom)
{
  RenderCamera camera;
  camera.setViewportSize(QSizeF(200.0, 100.0));

  camera.fitToRect(QRectF(10.0, 20.0, 2.0, 1.0), 1.0);

  expectPointNear(camera.getViewCenter(), 11.0, 20.5);
  EXPECT_DOUBLE_EQ(camera.getZoom(), 100.0);
}

TEST(RenderCameraTest, EmptyFitRectDoesNotChangeCamera)
{
  RenderCamera camera;
  camera.setViewCenter(QPointF(2.0, 3.0));
  camera.setZoom(120.0);

  camera.fitToRect(QRectF());

  expectPointNear(camera.getViewCenter(), 2.0, 3.0);
  EXPECT_DOUBLE_EQ(camera.getZoom(), 120.0);
}

TEST(RenderCameraTest, MatrixMapsViewCenterToViewportCenter)
{
  RenderCamera camera;
  camera.setViewportSize(QSizeF(200.0, 100.0));
  camera.setViewCenter(QPointF(3.0, 4.0));
  camera.setZoom(100.0);

  const QMatrix4x4 matrix = camera.getMatrix();
  const QPointF mapped_center = matrix.map(QPointF(3.0, 4.0));

  expectPointNear(mapped_center, 100.0, 50.0);
}