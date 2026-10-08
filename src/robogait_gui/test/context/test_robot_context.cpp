#include <gtest/gtest.h>

#include "Context/RobotContext.hpp"

TEST(RobotContextTest, StartsUnconfiguredAndNormalizesPlainTopics)
{
  const ROBOGait::context::RobotContext context;

  EXPECT_FALSE(context.isConfigured());
  EXPECT_TRUE(context.usesNamespace());
  EXPECT_TRUE(context.topicNamespace().isEmpty());
  EXPECT_EQ(context.resolveTopic("cmd_vel"), "/cmd_vel");
  EXPECT_EQ(context.resolveTopic("/scan"), "/scan");
  EXPECT_EQ(context.resolveFrame("/base_link"), "base_link");
}

TEST(RobotContextTest, NamespaceRobotPrefixesTopics)
{
  ROBOGait::context::RobotContext context;

  ASSERT_TRUE(context.setSelectedRobot(" robot one/ ", true));

  EXPECT_TRUE(context.isConfigured());
  EXPECT_TRUE(context.usesNamespace());
  EXPECT_EQ(context.topicNamespace(), "/robot_one");
  EXPECT_EQ(context.resolveTopic("cmd_vel"), "/robot_one/cmd_vel");
  EXPECT_EQ(context.resolveTopic("map"), "/robot_one/map");
  EXPECT_EQ(context.resolveTopic("/robot_one/cmd_vel"), "/robot_one/cmd_vel");
}

TEST(RobotContextTest, NamespaceRobotResolvesFramesForSelectedBuildMode)
{
  ROBOGait::context::RobotContext context;

  ASSERT_TRUE(context.setSelectedRobot(" robot one/ ", true));

#ifdef ROBOGAIT_SIMULATION
  EXPECT_EQ(context.resolveFrame("base_link"), "base_link");
  EXPECT_EQ(context.resolveFrame("/odom"), "odom");
#else
  EXPECT_EQ(context.resolveFrame("base_link"), "robot_one/base_link");
  EXPECT_EQ(context.resolveFrame("/odom"), "robot_one/odom");
#endif

  EXPECT_EQ(context.resolveFrame("/map"), "map");
  EXPECT_EQ(context.resolveFrame("/robot_one/base_link"), "robot_one/base_link");
  EXPECT_EQ(context.resolveFrame(""), "");
}

TEST(RobotContextTest, ResolvesFramesInNestedNamespacesForSelectedBuildMode)
{
  ROBOGait::context::RobotContext context;

  ASSERT_TRUE(context.setSelectedRobot("/fleet/robot1", true));

#ifdef ROBOGAIT_SIMULATION
  EXPECT_EQ(context.resolveFrame("base_link"), "base_link");
#else
  EXPECT_EQ(context.resolveFrame("base_link"), "fleet/robot1/base_link");
  EXPECT_EQ(context.resolveFrame("fleet/robot1/base_link"), "fleet/robot1/base_link");
#endif

  EXPECT_EQ(context.resolveFrame("map"), "map");
}

TEST(RobotContextTest, NonNamespaceRobotLeavesTopicsAndFramesGlobal)
{
  ROBOGait::context::RobotContext context;

  ASSERT_TRUE(context.setSelectedRobot("robot_serial_1", false));

  EXPECT_TRUE(context.isConfigured());
  EXPECT_FALSE(context.usesNamespace());
  EXPECT_TRUE(context.topicNamespace().isEmpty());
  EXPECT_EQ(context.resolveTopic("cmd_vel"), "/cmd_vel");
  EXPECT_EQ(context.resolveFrame("/base_link"), "base_link");
}

TEST(RobotContextTest, RejectsEmptyIdentifierWithoutChangingPreviousContext)
{
  ROBOGait::context::RobotContext context;
  ASSERT_TRUE(context.setSelectedRobot("robot_a", true));

  EXPECT_FALSE(context.setSelectedRobot("   ", true));

  EXPECT_TRUE(context.isConfigured());
  EXPECT_EQ(context.topicNamespace(), "/robot_a");
}

TEST(RobotContextTest, ClearResetsContext)
{
  ROBOGait::context::RobotContext context;
  ASSERT_TRUE(context.setSelectedRobot("robot_a", true));

  context.clear();

  EXPECT_FALSE(context.isConfigured());
  EXPECT_TRUE(context.usesNamespace());
  EXPECT_TRUE(context.topicNamespace().isEmpty());
}
