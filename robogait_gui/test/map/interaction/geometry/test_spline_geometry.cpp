#include <cmath>

#include <QPointF>
#include <QVector>

#include <gtest/gtest.h>

#include "Map/Interaction/Geometry/BezierSplineModel.hpp"
#include "Map/Interaction/Geometry/SplineFitter.hpp"

namespace
{
using ROBOGait::map::interaction::geometry::BezierSplineModel;
using ROBOGait::map::interaction::geometry::ControlPointType;
using ROBOGait::map::interaction::geometry::CubicBezierSegment;
using ROBOGait::map::interaction::geometry::SplineFitter;

void expectPointNear(const QPointF& point, double x, double y)
{
  EXPECT_NEAR(point.x(), x, 1e-9);
  EXPECT_NEAR(point.y(), y, 1e-9);
}

void expectFiniteSegment(const CubicBezierSegment& segment)
{
  EXPECT_TRUE(std::isfinite(segment.p0.x()));
  EXPECT_TRUE(std::isfinite(segment.p0.y()));
  EXPECT_TRUE(std::isfinite(segment.c1.x()));
  EXPECT_TRUE(std::isfinite(segment.c1.y()));
  EXPECT_TRUE(std::isfinite(segment.c2.x()));
  EXPECT_TRUE(std::isfinite(segment.c2.y()));
  EXPECT_TRUE(std::isfinite(segment.p1.x()));
  EXPECT_TRUE(std::isfinite(segment.p1.y()));
}
} // namespace

TEST(SplineFitterTest, ReturnsNoSegmentsForInsufficientPoints)
{
  EXPECT_TRUE(SplineFitter::fitCentripetalCatmullRom({}, 0.5).empty());
  EXPECT_TRUE(SplineFitter::fitCentripetalCatmullRom({{0.0, 0.0}}, 0.5).empty());
}

TEST(SplineFitterTest, TwoPointsCreateStraightCubicSegment)
{
  const QVector<CubicBezierSegment> segments = SplineFitter::fitCentripetalCatmullRom({{0.0, 0.0}, {3.0, 0.0}}, 0.5);

  ASSERT_EQ(segments.size(), 1);
  expectPointNear(segments[0].p0, 0.0, 0.0);
  expectPointNear(segments[0].c1, 1.0, 0.0);
  expectPointNear(segments[0].c2, 2.0, 0.0);
  expectPointNear(segments[0].p1, 3.0, 0.0);
}

TEST(SplineFitterTest, RepeatedPointsProduceFiniteSegments)
{
  const QVector<CubicBezierSegment> segments = SplineFitter::fitCentripetalCatmullRom({{0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}}, 0.5);

  ASSERT_EQ(segments.size(), 2);
  expectFiniteSegment(segments[0]);
  expectFiniteSegment(segments[1]);
}

TEST(BezierSplineModelTest, StartsInvalidAndRejectsEmptySegments)
{
  BezierSplineModel model;

  EXPECT_FALSE(model.isValid());
  EXPECT_FALSE(model.setFromSegments({}));
  EXPECT_TRUE(model.getSegments().empty());
  EXPECT_TRUE(model.getControlPoints().empty());
}

TEST(BezierSplineModelTest, RebuildsSegmentsFromEditableModel)
{
  const QVector<CubicBezierSegment> original_segments = {
      {{0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0}},
      {{3.0, 0.0}, {4.0, 0.0}, {5.0, 0.0}, {6.0, 0.0}},
  };

  BezierSplineModel model;

  ASSERT_TRUE(model.setFromSegments(original_segments));
  const QVector<CubicBezierSegment> rebuilt_segments = model.getSegments();

  ASSERT_EQ(rebuilt_segments.size(), original_segments.size());
  expectPointNear(rebuilt_segments[0].p0, 0.0, 0.0);
  expectPointNear(rebuilt_segments[0].c1, 1.0, 0.0);
  expectPointNear(rebuilt_segments[0].c2, 2.0, 0.0);
  expectPointNear(rebuilt_segments[0].p1, 3.0, 0.0);
  expectPointNear(rebuilt_segments[1].p0, 3.0, 0.0);
  expectPointNear(rebuilt_segments[1].c1, 4.0, 0.0);
  expectPointNear(rebuilt_segments[1].c2, 5.0, 0.0);
  expectPointNear(rebuilt_segments[1].p1, 6.0, 0.0);
}

TEST(BezierSplineModelTest, MovingAnchorTranslatesAttachedHandles)
{
  BezierSplineModel model;
  ASSERT_TRUE(model.setFromSegments({{{0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0}}}));

  ASSERT_TRUE(model.setControlPoint(ControlPointType::Anchor, 0, {0.0, 2.0}));
  const QVector<CubicBezierSegment> segments = model.getSegments();

  ASSERT_EQ(segments.size(), 1);
  expectPointNear(segments[0].p0, 0.0, 2.0);
  expectPointNear(segments[0].c1, 1.0, 2.0);
  expectPointNear(segments[0].c2, 2.0, 0.0);
  expectPointNear(segments[0].p1, 3.0, 0.0);
}

TEST(BezierSplineModelTest, UpdatesHandleWithoutMovingAnchor)
{
  BezierSplineModel model;
  ASSERT_TRUE(model.setFromSegments({{{0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0}}}));

  ASSERT_TRUE(model.setControlPoint(ControlPointType::HandleIn, 1, {2.0, 2.0}));
  EXPECT_FALSE(model.setControlPoint(ControlPointType::HandleOut, -1, {0.0, 0.0}));

  const QVector<CubicBezierSegment> segments = model.getSegments();

  ASSERT_EQ(segments.size(), 1);
  expectPointNear(segments[0].p0, 0.0, 0.0);
  expectPointNear(segments[0].c2, 2.0, 2.0);
  expectPointNear(segments[0].p1, 3.0, 0.0);
}

TEST(BezierSplineModelTest, SamplesLinearSegmentByDistance)
{
  BezierSplineModel model;
  ASSERT_TRUE(model.setFromSegments({{{0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0}}}));

  const QVector<QPointF> sampled = model.sampleByDistance(1.0);

  ASSERT_EQ(sampled.size(), 4);
  expectPointNear(sampled[0], 0.0, 0.0);
  expectPointNear(sampled[1], 1.0, 0.0);
  expectPointNear(sampled[2], 2.0, 0.0);
  expectPointNear(sampled[3], 3.0, 0.0);
}