#include "Ros/TopicsName.hpp"
#include "Ros/Define.hpp"

namespace ROBOGait
{
namespace ros
{
namespace topics
{
// clang-format off
// Topics
const std::string T_CMD_VEL =                          "/cmd_vel";
const std::string T_ROBOT_STATUS =                     "/" + ROBOGait::ros::define::COMMAND_EXECUTOR + "/robot_status";
const std::string T_MAP =                              "/map";
const std::string T_MAP_UPDATES =                      "/map_updates";
const std::string T_PLAN =                             "/plan";
const std::string T_SCAN =                             "/scan";
const std::string T_PARTICLE_CLOUD =                   "/particle_cloud";
const std::string T_POSE_INITIALIZE =                  "/initialpose";
const std::string T_USER_DETECTION =                   "/user_detection_filter";

// Services
const std::string S_CMD =                              define::COMMAND_EXECUTOR + "/cmd";
const std::string S_GET_MAP_DATA =                     define::COMMAND_EXECUTOR + "/get_map_data";
const std::string S_REINITIALIZE_GLOBAL_LOCALIZATION = "/reinitialize_global_localization";

// Actions
const std::string A_NAVIGATE_TO_POSE =                 "/navigate_to_pose";
const std::string A_COMPUTE_PATH_TO_POSE =             "/compute_path_to_pose";
const std::string A_FOLLOW_PATH =                      "/follow_path";

// TF Frames
const std::string TF_MAP_FRAME =                       "map";
const std::string TF_ROBOT_FRAME =                     "base_link";
// clang-format on
} // namespace topics
} // namespace ros
} // namespace ROBOGait
