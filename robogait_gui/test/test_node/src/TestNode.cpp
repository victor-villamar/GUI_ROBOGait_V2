#include "TestNode.hpp"

TestNode::TestNode(std::shared_ptr<rclcpp::Node> node, const std::string& robot_namespace) :
    parent_node_(node), robot_name_("robogait"), robot_namespace_(robot_namespace)
{
  RCLCPP_INFO(parent_node_->get_logger(), "[TestNode::TestNode] Initializing TestNode");
  RCLCPP_INFO(parent_node_->get_logger(), "[TestNode::TestNode] Robot Name: %s", robot_name_.c_str());

  cb_group_ = parent_node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

  std::string topic_name;
  if (robot_namespace_.empty())
  {
    topic_name = T_ROBOT_STATUS;
  }
  else
  {
    topic_name = "/" + robot_namespace_ + T_ROBOT_STATUS;
  }

  pub_robot_status_ = parent_node_->create_publisher<command_executor_msgs::msg::RobotStatus>(topic_name, QOS_BEST_EFFORT);

  loop_timer_ = parent_node_->create_wall_timer(std::chrono::duration<float>(1.0), std::bind(&TestNode::publishLoop, this), cb_group_);

  RCLCPP_INFO(parent_node_->get_logger(), "[TestNode::TestNode] TestNode initialized successfully");
  RCLCPP_INFO(parent_node_->get_logger(), "[TestNode::TestNode] Publishing on topic: %s at 1Hz", topic_name.c_str());
}

TestNode::~TestNode() { RCLCPP_INFO(parent_node_->get_logger(), "[TestNode::~TestNode] TestNode destroyed"); }

void TestNode::publishLoop()
{
  command_executor_msgs::msg::RobotStatus msg;
  msg.id = 1;
  msg.battery = 100;
  msg.ns = robot_namespace_;
  msg.version = "1.0";
  msg.hardware_id = 35;
  msg.serial_number = "SN12345";

  pub_robot_status_->publish(msg);
}
