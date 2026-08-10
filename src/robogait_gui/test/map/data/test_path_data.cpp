#include <gtest/gtest.h>

#include "Map/Data/PathData.hpp"

TEST(PathDataTest, StoresPointsAndResetsState)
{
  ROBOGait::map::data::PathData path_data;
  ROBOGait::map::data::PathData::PathMetadata metadata;
  metadata.points = {{1.0, 2.0}, {3.0, 4.0}};

  EXPECT_FALSE(path_data.isAvailable());
  EXPECT_EQ(path_data.getUpdateStamp(), 0u);

  path_data.setPath(metadata);

  const auto points = path_data.getPoints();
  EXPECT_TRUE(path_data.isAvailable());
  EXPECT_EQ(path_data.getUpdateStamp(), 1u);
  ASSERT_EQ(points.size(), 2u);
  EXPECT_DOUBLE_EQ(points[0].x_, 1.0);
  EXPECT_DOUBLE_EQ(points[0].y_, 2.0);
  EXPECT_DOUBLE_EQ(points[1].x_, 3.0);
  EXPECT_DOUBLE_EQ(points[1].y_, 4.0);

  path_data.reset();

  EXPECT_FALSE(path_data.isAvailable());
  EXPECT_TRUE(path_data.getPoints().empty());
  EXPECT_EQ(path_data.getUpdateStamp(), 2u);
}