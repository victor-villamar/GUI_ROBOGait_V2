#include <functional>
#include <iostream>
#include <logs/Logs.hpp>
#include <memory>
#include <string>

#include "LauncherManager.hpp"
#include "common/functions.hpp"
#include "common/topics_name.hpp"

using namespace ROBOGait::common;
using namespace ROBOGait::manager;
using namespace ROBOGait::common::logs;

LauncherManager::LauncherManager(rclcpp::Node* parent_node) : parent_node_(parent_node)
{
  Logs::infoStream("[LauncherManager::LauncherManager] Creating LauncherManager");
}

LauncherManager::~LauncherManager() { Logs::infoStream("[LauncherManager::~LauncherManager] Destroying LauncherManager"); }

bool LauncherManager::init()
{
  const std::string param_dir = common::functions::getFilePath("launcher_manager", "config/");
  if (param_dir.empty())
  {
    Logs::error("[LauncherManager::init] Could not find parameter directory.");
    return false;
  }

  state_machine_ = std::make_unique<ROBOGait::state_machine::StateMachine>();

  if (state_machine_ == nullptr)
  {
    Logs::error("[LauncherManager::init] Could not create StateMachine instance.");
    return false;
  }

  if (!state_machine_->init(param_dir))
  {
    Logs::error("[LauncherManager::init] Could not initialize StateMachine.");
    return false;
  }
  Logs::infoStream("[LauncherManager::init] StateMachine initialized successfully.");

  // Service
  ser_manage_process_ = parent_node_->create_service<interface_srv::srv::ManageProcess>(
      S_LM_GUI, std::bind(&LauncherManager::handleProcessCommandRequest, this, std::placeholders::_1, std::placeholders::_2));

  return true;
}

void LauncherManager::handleProcessCommandRequest(const std::shared_ptr<interface_srv::srv::ManageProcess::Request>& request,
                                                  std::shared_ptr<interface_srv::srv::ManageProcess::Response> response)
{
  response->success = false;
  const auto command = static_cast<common::ProcessCommand>(request->process_id);

  if (state_machine_ == nullptr)
  {
    Logs::error("[LauncherManager::handleProcessCommandRequest] StateMachine instance is null, cannot process command");
    return;
  }

  response->success = state_machine_->dispatchProcessCommand(command, request->start, request->name);
}