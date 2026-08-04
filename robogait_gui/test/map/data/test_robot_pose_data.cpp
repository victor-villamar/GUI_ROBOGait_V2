#include <gtest/gtest.h>

#include "Map/Data/RobotPoseData.hpp"

TEST(RobotPoseDataTest, StoresPoseAndResetsToOrigin)
{
  ROBOGait::map::data::RobotPoseData pose_data;
  ROBOGait::map::data::RobotPoseData::RobotPoseMetadata metadata;
  metadata.x_ = 1.25;
  metadata.y_ = -0.5;
  metadata.theta_ = 3.14;

  pose_data.setPose(metadata);

  auto stored_metadata = pose_data.getMetadata();
  EXPECT_TRUE(pose_data.isAvailable());
  EXPECT_EQ(pose_data.getUpdateStamp(), 1u);
  EXPECT_DOUBLE_EQ(stored_metadata.x_, 1.25);
  EXPECT_DOUBLE_EQ(stored_metadata.y_, -0.5);
  EXPECT_DOUBLE_EQ(stored_metadata.theta_, 3.14);

  pose_data.reset();

  stored_metadata = pose_data.getMetadata();
  EXPECT_FALSE(pose_data.isAvailable());
  EXPECT_EQ(pose_data.getUpdateStamp(), 2u);
  EXPECT_DOUBLE_EQ(stored_metadata.x_, 0.0);
  EXPECT_DOUBLE_EQ(stored_metadata.y_, 0.0);
  EXPECT_DOUBLE_EQ(stored_metadata.theta_, 0.0);
}