#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "Map/Data/MapData.hpp"

namespace
{
using ROBOGait::map::data::MapData;

MapData::MapMetadata createMetadata(uint32_t width, uint32_t height)
{
  MapData::MapMetadata metadata;
  metadata.resolution_ = 0.1;
  metadata.width_ = width;
  metadata.height_ = height;
  metadata.origin_x_ = 1.0;
  metadata.origin_y_ = 2.0;
  metadata.origin_theta_ = 0.5;
  return metadata;
}
} // namespace

TEST(MapDataTest, StartsUnavailableWithDefaultMetadata)
{
  const MapData map_data;

  const auto metadata = map_data.getMetadata();
  EXPECT_FALSE(map_data.isAvailable());
  EXPECT_TRUE(map_data.getOccupancyData().empty());
  EXPECT_EQ(map_data.getUpdateStamp(), 0u);
  EXPECT_DOUBLE_EQ(metadata.resolution_, 0.05);
  EXPECT_EQ(metadata.width_, 0u);
  EXPECT_EQ(metadata.height_, 0u);
}

TEST(MapDataTest, StoresValidOccupancyDataAndMetadata)
{
  MapData map_data;
  const auto metadata = createMetadata(2u, 2u);
  const std::vector<int8_t> occupancy_data = {-1, 0, 50, 100};

  EXPECT_TRUE(map_data.setOccupancyData(occupancy_data, metadata));

  const auto stored_metadata = map_data.getMetadata();
  EXPECT_TRUE(map_data.isAvailable());
  EXPECT_EQ(map_data.getUpdateStamp(), 1u);
  EXPECT_EQ(map_data.getOccupancyData(), occupancy_data);
  EXPECT_EQ(stored_metadata.width_, 2u);
  EXPECT_EQ(stored_metadata.height_, 2u);
  EXPECT_DOUBLE_EQ(stored_metadata.resolution_, 0.1);
  EXPECT_DOUBLE_EQ(stored_metadata.origin_x_, 1.0);
  EXPECT_DOUBLE_EQ(stored_metadata.origin_y_, 2.0);
  EXPECT_DOUBLE_EQ(stored_metadata.origin_theta_, 0.5);
}

TEST(MapDataTest, RejectsInvalidOccupancyPayloadsWithoutStampUpdate)
{
  MapData map_data;

  EXPECT_FALSE(map_data.setOccupancyData({1, 2, 3}, createMetadata(2u, 2u)));
  EXPECT_FALSE(map_data.isAvailable());
  EXPECT_EQ(map_data.getUpdateStamp(), 0u);
  EXPECT_TRUE(map_data.getOccupancyData().empty());

  EXPECT_FALSE(map_data.setOccupancyData({}, createMetadata(0u, 2u)));
  EXPECT_FALSE(map_data.isAvailable());
  EXPECT_EQ(map_data.getUpdateStamp(), 0u);
}

TEST(MapDataTest, UpdatesSubRegionInRowMajorOrder)
{
  MapData map_data;
  EXPECT_TRUE(map_data.setOccupancyData({0, 1, 2, 3, 4, 5}, createMetadata(3u, 2u)));

  EXPECT_TRUE(map_data.updateRegion(1, 0, 2u, 2u, {9, 8, 7, 6}));

  const std::vector<int8_t> expected = {0, 9, 8, 3, 7, 6};
  EXPECT_TRUE(map_data.isAvailable());
  EXPECT_EQ(map_data.getUpdateStamp(), 2u);
  EXPECT_EQ(map_data.getOccupancyData(), expected);
}

TEST(MapDataTest, IgnoresOutOfBoundsRegionWithoutStampUpdate)
{
  MapData map_data;
  const std::vector<int8_t> original = {0, 1, 2, 3};
  EXPECT_TRUE(map_data.setOccupancyData(original, createMetadata(2u, 2u)));

  EXPECT_FALSE(map_data.updateRegion(1, 1, 2u, 1u, {8, 9}));

  EXPECT_EQ(map_data.getUpdateStamp(), 1u);
  EXPECT_EQ(map_data.getOccupancyData(), original);
}

TEST(MapDataTest, IgnoresNegativeRegionWithoutStampUpdate)
{
  MapData map_data;
  const std::vector<int8_t> original = {0, 1, 2, 3};
  EXPECT_TRUE(map_data.setOccupancyData(original, createMetadata(2u, 2u)));

  EXPECT_FALSE(map_data.updateRegion(-1, 0, 1u, 1u, {9}));
  EXPECT_FALSE(map_data.updateRegion(0, -1, 1u, 1u, {9}));

  EXPECT_EQ(map_data.getUpdateStamp(), 1u);
  EXPECT_EQ(map_data.getOccupancyData(), original);
}

TEST(MapDataTest, ResetClearsStateAndIncrementsStamp)
{
  MapData map_data;
  EXPECT_TRUE(map_data.setOccupancyData({0, 1, 2, 3}, createMetadata(2u, 2u)));

  map_data.reset();

  const auto metadata = map_data.getMetadata();
  EXPECT_FALSE(map_data.isAvailable());
  EXPECT_TRUE(map_data.getOccupancyData().empty());
  EXPECT_EQ(map_data.getUpdateStamp(), 2u);
  EXPECT_EQ(metadata.width_, 0u);
  EXPECT_EQ(metadata.height_, 0u);
  EXPECT_DOUBLE_EQ(metadata.resolution_, 0.05);
}