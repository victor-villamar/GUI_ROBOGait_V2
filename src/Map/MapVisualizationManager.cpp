#include "Map/MapVisualizationManager.hpp"

#include "Context/RobotContext.hpp"

#include <QDebug>

using namespace ROBOGait::map::manager;

MapVisualizationManager::MapVisualizationManager() :
    parent_node_(nullptr),
    visualization_manager_(nullptr),
    map_render_widget_(nullptr),
    map_display_(nullptr),
    robot_display_(nullptr),
    selected_robot_namespace_(""),
    use_namespace_discovery_(true),
    is_initialized_(false),
    subscriptions_active_(false)
{
  visualization_manager_ = std::make_shared<ROBOGait::map::rendering::VisualizationManager>();

  qInfo() << "[MapVisualizationManager::MapVisualizationManager] Map Visualization Manager created";
}

MapVisualizationManager::~MapVisualizationManager()
{
  destroySubscriptions();
  destroyDisplays();

  qInfo() << "[MapVisualizationManager] Manager destroyed";
}

void MapVisualizationManager::setROSNode(rclcpp::Node* parent_node)
{
  if (is_initialized_)
  {
    qWarning() << "[MapVisualizationManager::setROSNode] Already initialized";
    return;
  }

  if (!parent_node)
  {
    qCritical() << "[MapVisualizationManager::setROSNode] Null node pointer";
    return;
  }

  parent_node_ = parent_node;

  // Initialize VisualizationManager with ROS node
  if (visualization_manager_)
  {
    visualization_manager_->initialize(parent_node_);
  }

  is_initialized_ = true;

  emit isInitializedChanged();

  qInfo() << "[MapVisualizationManager::setROSNode] ROS node set and VisualizationManager initialized";
}

void MapVisualizationManager::setSelectedRobot(const QString& robot_identifier, bool is_namespace)
{
  if (!is_initialized_)
  {
    qWarning() << "[MapVisualizationManager::setSelectedRobot] Not initialized";
    return;
  }

  // Destroy previous displays if they exist
  if (subscriptions_active_)
  {
    destroySubscriptions();
  }

  destroyDisplays();

  // Store new robot selection
  selected_robot_namespace_ = robot_identifier;
  use_namespace_discovery_ = is_namespace;

  // Create new displays for this robot
  createDisplays();
}

void MapVisualizationManager::activateSubscriptions()
{
  if (!is_initialized_)
  {
    qWarning() << "[MapVisualizationManager::activateSubscriptions] Not initialized";
    return;
  }

  if (subscriptions_active_)
  {
    qInfo() << "[MapVisualizationManager::activateSubscriptions] Already active";
    return;
  }

  if (!map_display_ || !robot_display_)
  {
    qWarning() << "[MapVisualizationManager::activateSubscriptions] Displays not created yet";
    return;
  }

  ROBOGait::context::RobotContext context;
  if (context.setSelectedRobot(selected_robot_namespace_, use_namespace_discovery_))
  {
    map_display_->setRobotContext(context);
    robot_display_->setRobotContext(context);
  }
  else
  {
    qWarning() << "[MapVisualizationManager::activateSubscriptions] Invalid robot context, using default topics";
  }

  map_display_->activateSubscriptions();

  // Start visualization update loop
  visualization_manager_->startUpdate();

  subscriptions_active_ = true;
}

void MapVisualizationManager::destroySubscriptions()
{
  if (!subscriptions_active_)
  {
    return;
  }

  // Stop visualization updates
  visualization_manager_->stopUpdate();

  subscriptions_active_ = false;

  qInfo() << "[MapVisualizationManager::destroySubscriptions] Subscriptions destroyed";
}

bool MapVisualizationManager::isInitialized() const { return is_initialized_; }

bool MapVisualizationManager::isMapAvailable() const
{
  if (!map_display_)
  {
    return false;
  }

  return map_display_->isMapAvailable();
}

bool MapVisualizationManager::isRobotPoseAvailable() const
{
  if (!robot_display_)
  {
    return false;
  }

  return robot_display_->isRobotPoseAvailable();
}

double MapVisualizationManager::getZoomLevel() const
{
  if (!map_render_widget_)
  {
    return 1.0;
  }

  return map_render_widget_->getZoomLevel();
}

ROBOGait::map::rendering::MapRenderWidget* MapVisualizationManager::getMapRenderWidget() const { return map_render_widget_; }

void MapVisualizationManager::createDisplays()
{
  if (!is_initialized_)
  {
    qWarning() << "[MapVisualizationManager::createDisplays] Not initialized";
    return;
  }

  // Create displays
  map_display_ = std::make_shared<ROBOGait::map::display::MapDisplay>();
  robot_display_ = std::make_shared<ROBOGait::map::display::RobotDisplay>();

  // Initialize with ROS node
  map_display_->initialize(parent_node_);
  robot_display_->initialize(parent_node_);

  // Connect signals
  // clang-format off
  connect(map_display_.get(),
          &ROBOGait::map::display::MapDisplay::mapUpdated,
          this,
          &MapVisualizationManager::mapAvailableChanged);

  connect(robot_display_.get(),
          &ROBOGait::map::display::RobotDisplay::poseUpdated,
          this,
          &MapVisualizationManager::robotPoseAvailableChanged);
  // clang-format on

  // Add to visualization manager
  visualization_manager_->addDisplay("map", map_display_);
  visualization_manager_->addDisplay("robot", robot_display_);

  qInfo() << "[MapVisualizationManager::createDisplays] Displays created";
}

void MapVisualizationManager::destroyDisplays()
{
  if (!map_display_ && !robot_display_)
  {
    return;
  }

  // Remove from visualization manager
  visualization_manager_->removeAllDisplays();

  // Shutdown displays
  if (map_display_)
  {
    map_display_->shutdown();
    map_display_.reset();
  }

  if (robot_display_)
  {
    robot_display_->shutdown();
    robot_display_.reset();
  }

  qInfo() << "[MapVisualizationManager::destroyDisplays] Displays destroyed";
}
void MapVisualizationManager::registerMapRenderWidget(QObject* widget)
{
  if (!widget)
  {
    qCritical() << "[MapVisualizationManager::registerMapRenderWidget] Null widget";
    return;
  }

  auto* render_widget = qobject_cast<ROBOGait::map::rendering::MapRenderWidget*>(widget);
  if (!render_widget)
  {
    qCritical() << "[MapVisualizationManager::registerMapRenderWidget] Invalid widget type";
    return;
  }

  map_render_widget_ = render_widget;

  // Connect widget to visualization manager
  if (visualization_manager_)
  {
    map_render_widget_->setVisualizationManager(visualization_manager_);
    qInfo() << "[MapVisualizationManager::registerMapRenderWidget] Widget registered and connected";
  }
}
