#include "Map/Rendering/RenderScene.hpp"

using namespace ROBOGait::map::rendering;

RenderScene::RenderScene() : pipeline_(std::make_shared<RenderPipeline>()) {}

void RenderScene::setMapLayer(std::shared_ptr<ROBOGait::map::layer::MapLayer> map_layer)
{
  map_layer_ = std::move(map_layer);
  if (pipeline_)
  {
    pipeline_->removeLayer("map");
    if (map_layer_)
    {
      pipeline_->addLayer("map", map_layer_);
    }
  }
}

void RenderScene::setRobotLayer(std::shared_ptr<ROBOGait::map::layer::RobotLayer> robot_layer)
{
  robot_layer_ = std::move(robot_layer);
  if (pipeline_)
  {
    pipeline_->removeLayer("robot");
    if (robot_layer_)
    {
      pipeline_->addLayer("robot", robot_layer_);
    }
  }
}

std::shared_ptr<RenderPipeline> RenderScene::getPipeline() const { return pipeline_; }

void RenderScene::start()
{
  if (pipeline_)
  {
    pipeline_->startUpdate();
  }
}

void RenderScene::stop()
{
  if (pipeline_)
  {
    pipeline_->stopUpdate();
  }
}
