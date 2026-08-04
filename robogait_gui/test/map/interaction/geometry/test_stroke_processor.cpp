#include <QPointF>
#include <QVector>

#include <gtest/gtest.h>

#include "Map/Interaction/Geometry/StrokeProcessor.hpp"

namespace
{
using ROBOGait::map::interaction::geometry::StrokeProcessor;

void expectPointNear(const QPointF& point, double x, double y)
{
  EXPECT_NEAR(point.x(), x, 1e-9);
  EXPECT_NEAR(point.y(), y, 1e-9);
}
} // namespace

TEST(StrokeProcessorTest, RemoveDuplicatedPointsFiltersNearConsecutiveSamples)
{
  const QVector<QPointF> points = {{0.0, 0.0}, {0.01, 0.0}, {0.2, 0.0}, {0.21, 0.0}, {0.5, 0.0}};

  const QVector<QPointF> filtered = StrokeProcessor::removeDuplicatedPoints(points, 0.1);

  ASSERT_EQ(filtered.size(), 3);
  expectPointNear(filtered[0], 0.0, 0.0);
  expectPointNear(filtered[1], 0.2, 0.0);
  expectPointNear(filtered[2], 0.5, 0.0);
}

TEST(StrokeProcessorTest, ResampleByArcLengthSamplesStraightPolylineUniformly)
{
  const QVector<QPointF> points = {{0.0, 0.0}, {3.0, 0.0}};

  const QVector<QPointF> resampled = StrokeProcessor::resampleByArcLength(points, 1.0);

  ASSERT_EQ(resampled.size(), 4);
  expectPointNear(resampled[0], 0.0, 0.0);
  expectPointNear(resampled[1], 1.0, 0.0);
  expectPointNear(resampled[2], 2.0, 0.0);
  expectPointNear(resampled[3], 3.0, 0.0);
}

TEST(StrokeProcessorTest, SmoothMovingAveragePreservesEndpoints)
{
  const QVector<QPointF> points = {{0.0, 0.0}, {1.0, 3.0}, {2.0, 0.0}};

  const QVector<QPointF> smoothed = StrokeProcessor::smoothMovingAverage(points, 1);

  ASSERT_EQ(smoothed.size(), 3);
  expectPointNear(smoothed[0], 0.0, 0.0);
  expectPointNear(smoothed[1], 1.0, 1.0);
  expectPointNear(smoothed[2], 2.0, 0.0);
}

TEST(StrokeProcessorTest, SimplifyDouglasPeuckerRemovesCollinearInteriorPoints)
{
  const QVector<QPointF> points = {{0.0, 0.0}, {1.0, 0.01}, {2.0, -0.01}, {3.0, 0.0}};

  const QVector<QPointF> simplified = StrokeProcessor::simplifyDouglasPeucker(points, 0.05);

  ASSERT_EQ(simplified.size(), 2);
  expectPointNear(simplified[0], 0.0, 0.0);
  expectPointNear(simplified[1], 3.0, 0.0);
}

TEST(StrokeProcessorTest, SimplifyDouglasPeuckerKeepsSignificantBend)
{
  const QVector<QPointF> points = {{0.0, 0.0}, {1.0, 1.0}, {2.0, 0.0}};

  const QVector<QPointF> simplified = StrokeProcessor::simplifyDouglasPeucker(points, 0.1);

  ASSERT_EQ(simplified.size(), 3);
  expectPointNear(simplified[0], 0.0, 0.0);
  expectPointNear(simplified[1], 1.0, 1.0);
  expectPointNear(simplified[2], 2.0, 0.0);
}

TEST(StrokeProcessorTest, LimitPointCountPreservesEndpoints)
{
  const QVector<QPointF> points = {{0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0}, {4.0, 0.0}};

  const QVector<QPointF> limited = StrokeProcessor::limitPointCount(points, 3);

  ASSERT_EQ(limited.size(), 3);
  expectPointNear(limited.first(), 0.0, 0.0);
  expectPointNear(limited.last(), 4.0, 0.0);
}