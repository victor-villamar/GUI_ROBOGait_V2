#include <QDebug>

#include <tf2/LinearMath/Transform.h>
#include <tf2/exceptions.hpp>
#include <tf2/utils.h>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include "Map/Subscribers/ParticleCloudSubscriber.hpp"
#include "Ros/QoSProfiles.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::subscribers;

ParticleCloudSubscriber::ParticleCloudSubscriber() :
    parent_node_(nullptr),
    particle_cloud_data_(nullptr),
    map_frame_(ROBOGait::ros::topics::TF_MAP_FRAME),
    active_(false),
    warn_logged_(false),
    context_(std::nullopt)
{
}

ParticleCloudSubscriber::~ParticleCloudSubscriber() { stop(); }

void ParticleCloudSubscriber::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[ParticleCloudSubscriber::initialize] Null parent node pointer";
    return;
  }

  parent_node_ = parent_node;
}

void ParticleCloudSubscriber::setParticleCloudData(data::ParticleCloudData* particle_cloud_data) { particle_cloud_data_ = particle_cloud_data; }

void ParticleCloudSubscriber::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;

  if (context_)
  {
    map_frame_ = context_->resolveFrame(map_frame_);
  }
}

void ParticleCloudSubscriber::start()
{
  if (!parent_node_)
  {
    qCritical() << "[ParticleCloudSubscriber::start] Parent node is null";
    return;
  }

  if (active_)
  {
    return;
  }

  // Create TF buffer and listener
  tf_buffer_ = std::make_shared<tf2_ros::Buffer>(parent_node_->get_clock());
  tf_buffer_->setUsingDedicatedThread(true);
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, parent_node_, false);

  std::string cloud_topic = ROBOGait::ros::topics::T_PARTICLE_CLOUD;

  if (context_)
  {
    cloud_topic = context_->resolveTopic(cloud_topic);
  }

  sub_cloud_ = parent_node_->create_subscription<nav2_msgs::msg::ParticleCloud>(
      cloud_topic, ROBOGait::ros::QosProfiles::QOS_BEST_EFFORT(), std::bind(&ParticleCloudSubscriber::callbackParticleCloud, this, std::placeholders::_1));

  active_ = true;
  warn_logged_ = false;
  qDebug() << "[ParticleCloudSubscriber::start] Subscribed to particle cloud topic:" << cloud_topic.c_str();
}

void ParticleCloudSubscriber::stop()
{
  if (!active_)
  {
    return;
  }

  if (sub_cloud_)
  {
    sub_cloud_.reset();
  }

  if (tf_listener_)
  {
    tf_listener_.reset();
  }

  if (tf_buffer_)
  {
    tf_buffer_.reset();
  }

  active_ = false;

  qDebug() << "[ParticleCloudSubscriber::stop] Subscriptions stopped";
}

bool ParticleCloudSubscriber::isActive() const { return active_; }

void ParticleCloudSubscriber::callbackParticleCloud(const nav2_msgs::msg::ParticleCloud::SharedPtr msg)
{
  if (!particle_cloud_data_)
  {
    qCritical() << "[ParticleCloudSubscriber::callbackParticleCloud] ParticleCloudData is null";
    return;
  }

  if (!msg)
  {
    qWarning() << "[ParticleCloudSubscriber::callbackParticleCloud] Invalid ParticleCloud message";
    return;
  }

  if (!tf_buffer_)
  {
    qCritical() << "[ParticleCloudSubscriber::callbackParticleCloud] TF buffer is not initialized";
    return;
  }

  std::string cloud_frame = msg->header.frame_id;

  if (!cloud_frame.empty() && cloud_frame.front() == '/')
  {
    cloud_frame.erase(0, 1);
  }

  if (cloud_frame.empty())
  {
    qWarning() << "[ParticleCloudSubscriber::callbackParticleCloud] Cloud frame is empty";
    return;
  }

  data::ParticleCloudData::ParticleCloudMetadata metadata = transformParticleCloud(msg, cloud_frame);

  if (metadata.particles.empty())
  {
    return;
  }

  particle_cloud_data_->setParticleCloudData(metadata);
  warn_logged_ = false;
}

ROBOGait::map::data::ParticleCloudData::ParticleCloudMetadata
ParticleCloudSubscriber::transformParticleCloud(const nav2_msgs::msg::ParticleCloud::SharedPtr msg, const std::string& cloud_frame)
{
  data::ParticleCloudData::ParticleCloudMetadata metadata;

  const bool needs_transform = (cloud_frame != map_frame_);

  tf2::Transform tf;
  tf.setIdentity();

  if (needs_transform)
  {
    geometry_msgs::msg::TransformStamped transform_stamped;
    bool transform_found = false;

    try
    {
      transform_stamped = tf_buffer_->lookupTransform(map_frame_, cloud_frame, msg->header.stamp);
      transform_found = true;
    }
    catch (const tf2::TransformException& ex)
    {
      try
      {
        transform_stamped = tf_buffer_->lookupTransform(map_frame_, cloud_frame, tf2::TimePointZero);
        transform_found = true;
      }
      catch (const tf2::TransformException& e)
      {
        if (!warn_logged_)
        {
          qDebug() << "[ParticleCloudSubscriber::transformParticleCloud] Could not transform from" << cloud_frame.c_str() << "to" << map_frame_.c_str() << ":"
                   << e.what();
          warn_logged_ = true;
        }
        return metadata;
      }
    }

    if (!transform_found)
    {
      qDebug() << "[ParticleCloudSubscriber::transformParticleCloud] Could not find transform from" << cloud_frame.c_str() << "to" << map_frame_.c_str();
      return metadata;
    }

    tf2::fromMsg(transform_stamped.transform, tf);
  }

  metadata.particles.reserve(msg->particles.size());

  for (const auto& particle : msg->particles)
  {
    double x = particle.pose.position.x;
    double y = particle.pose.position.y;
    double theta = 0.0;

    if (needs_transform)
    {
      tf2::Transform pose_tf;
      tf2::fromMsg(particle.pose, pose_tf);
      const tf2::Transform map_pose = tf * pose_tf;
      const tf2::Vector3 origin = map_pose.getOrigin();
      x = origin.x();
      y = origin.y();
      theta = tf2::getYaw(map_pose.getRotation());
    }
    else
    {
      tf2::Quaternion q;
      tf2::fromMsg(particle.pose.orientation, q);
      theta = tf2::getYaw(q);
    }

    metadata.particles.emplace_back(x, y, theta, particle.weight);
  }

  return metadata;
}
