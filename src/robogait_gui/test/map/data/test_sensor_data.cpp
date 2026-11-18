#include <gtest/gtest.h>

#include "Map/Data/LaserScanData.hpp"
#include "Map/Data/ParticleCloudData.hpp"

TEST(LaserScanDataTest, StoresLaserPointsAndResetsState)
{
  ROBOGait::map::data::LaserScanData laser_scan_data;
  ROBOGait::map::data::LaserScanData::LaserScanMetadata metadata;
  metadata.points = {{0.1, 0.2}, {0.3, 0.4}};

  laser_scan_data.setLaserScanData(metadata);

  const auto points = laser_scan_data.getPoints();
  EXPECT_TRUE(laser_scan_data.isAvailable());
  EXPECT_EQ(laser_scan_data.getUpdateStamp(), 1u);
  ASSERT_EQ(points.size(), 2u);
  EXPECT_DOUBLE_EQ(points[0].x_, 0.1);
  EXPECT_DOUBLE_EQ(points[0].y_, 0.2);
  EXPECT_DOUBLE_EQ(points[1].x_, 0.3);
  EXPECT_DOUBLE_EQ(points[1].y_, 0.4);

  laser_scan_data.reset();

  EXPECT_FALSE(laser_scan_data.isAvailable());
  EXPECT_TRUE(laser_scan_data.getPoints().empty());
  EXPECT_EQ(laser_scan_data.getUpdateStamp(), 2u);
}

TEST(ParticleCloudDataTest, StoresParticlesAndResetsState)
{
  ROBOGait::map::data::ParticleCloudData particle_cloud_data;
  ROBOGait::map::data::ParticleCloudData::ParticleCloudMetadata metadata;
  metadata.particles = {{1.0, 2.0, 0.3, 0.4}, {3.0, 4.0, 0.5, 0.6}};

  particle_cloud_data.setParticleCloudData(metadata);

  const auto particles = particle_cloud_data.getParticles();
  EXPECT_TRUE(particle_cloud_data.isAvailable());
  EXPECT_EQ(particle_cloud_data.getUpdateStamp(), 1u);
  ASSERT_EQ(particles.size(), 2u);
  EXPECT_DOUBLE_EQ(particles[0].x_, 1.0);
  EXPECT_DOUBLE_EQ(particles[0].y_, 2.0);
  EXPECT_DOUBLE_EQ(particles[0].theta_, 0.3);
  EXPECT_DOUBLE_EQ(particles[0].weight_, 0.4);
  EXPECT_DOUBLE_EQ(particles[1].x_, 3.0);
  EXPECT_DOUBLE_EQ(particles[1].y_, 4.0);
  EXPECT_DOUBLE_EQ(particles[1].theta_, 0.5);
  EXPECT_DOUBLE_EQ(particles[1].weight_, 0.6);

  particle_cloud_data.reset();

  EXPECT_FALSE(particle_cloud_data.isAvailable());
  EXPECT_TRUE(particle_cloud_data.getParticles().empty());
  EXPECT_EQ(particle_cloud_data.getUpdateStamp(), 2u);
}