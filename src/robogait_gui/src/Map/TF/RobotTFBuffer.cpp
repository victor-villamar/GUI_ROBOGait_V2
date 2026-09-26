#include <functional>

#include <QDebug>

#include <tf2/exceptions.h>

#include "Map/TF/RobotTFBuffer.hpp"
#include "Ros/QoSProfiles.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::tf;

RobotTFBuffer::RobotTFBuffer() : parent_node_(nullptr), active_(false) {}

RobotTFBuffer::~RobotTFBuffer() { stop(); }

void RobotTFBuffer::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[RobotTFBuffer::initialize] No valid ROS node provided";
    return;
  }

  const bool restart = active_;
  stop();
  parent_node_ = parent_node;

  if (restart)
  {
    start();
  }
}

void RobotTFBuffer::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  const bool restart = active_;
  stop();
  context_ = context;

  if (restart)
  {
    start();
  }
}

void RobotTFBuffer::start()
{
  if (active_)
  {
    return;
  }

  if (!parent_node_)
  {
    qCritical() << "[RobotTFBuffer::start] Parent node is null";
    return;
  }

  if (!context_ || !context_->isConfigured())
  {
    qCritical() << "[RobotTFBuffer::start] Robot context is not configured";
    return;
  }

  tf_topic_ = context_->resolveTopic(ROBOGait::ros::topics::T_TF);
  tf_static_topic_ = context_->resolveTopic(ROBOGait::ros::topics::T_TF_STATIC);

  tf_buffer_ = std::make_shared<tf2_ros::Buffer>(parent_node_->get_clock());
  tf_buffer_->setUsingDedicatedThread(true);

  sub_tf_ = parent_node_->create_subscription<TFMessage>(tf_topic_, ROBOGait::ros::QosProfiles::QOS_TF_DYNAMIC(),
                                                         std::bind(&RobotTFBuffer::callbackTF, this, std::placeholders::_1));
  sub_tf_static_ = parent_node_->create_subscription<TFMessage>(tf_static_topic_, ROBOGait::ros::QosProfiles::QOS_TF_STATIC(),
                                                                std::bind(&RobotTFBuffer::callbackTFStatic, this, std::placeholders::_1));

  active_ = true;

  qDebug() << "[RobotTFBuffer::start] Subscribed to TF topics:" << tf_topic_.c_str() << "and" << tf_static_topic_.c_str();
}

void RobotTFBuffer::stop()
{
  sub_tf_.reset();
  sub_tf_static_.reset();

  if (tf_buffer_)
  {
    tf_buffer_->clear();
  }

  tf_buffer_.reset();
  tf_topic_.clear();
  tf_static_topic_.clear();
  active_ = false;
}

std::shared_ptr<tf2_ros::Buffer> RobotTFBuffer::getBuffer() const { return tf_buffer_; }

bool RobotTFBuffer::isActive() const { return active_; }

void RobotTFBuffer::callbackTF(const TFMessage::ConstSharedPtr msg) { insertTransforms(msg, tf_buffer_, tf_topic_, false); }

void RobotTFBuffer::callbackTFStatic(const TFMessage::ConstSharedPtr msg) { insertTransforms(msg, tf_buffer_, tf_static_topic_, true); }

void RobotTFBuffer::insertTransforms(const TFMessage::ConstSharedPtr& msg, const std::shared_ptr<tf2_ros::Buffer>& buffer, const std::string& authority,
                                     bool is_static)
{
  if (!msg || !buffer)
  {
    return;
  }

  for (const auto& transform : msg->transforms)
  {
    try
    {
      if (!buffer->setTransform(transform, authority, is_static))
      {
        qWarning() << "[RobotTFBuffer::insertTransforms] Rejected transform from" << transform.header.frame_id.c_str() << "to"
                   << transform.child_frame_id.c_str();
      }
    }
    catch (const tf2::TransformException& exception)
    {
      qWarning() << "[RobotTFBuffer::insertTransforms] Invalid transform:" << exception.what();
    }
  }
}