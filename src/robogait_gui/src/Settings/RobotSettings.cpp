#include <cmath>
#include <string>

#include <QDebug>
#include <QRegularExpression>

#include "Loader/YamlLoader.hpp"
#include "Ros/TopicsName.hpp"
#include "Settings/RobotSettings.hpp"

using namespace ROBOGait::settings;

namespace
{
bool isValidPositiveValue(double value) { return std::isfinite(value) && value > 0.0; }

bool isValidTopic(const QString& topic)
{
  static const QRegularExpression topic_pattern(QStringLiteral(R"(^/?[A-Za-z_][A-Za-z0-9_]*(/[A-Za-z_][A-Za-z0-9_]*)*$)"));
  return topic_pattern.match(topic).hasMatch();
}

bool isValidFrame(const QString& frame)
{
  static const QRegularExpression frame_pattern(QStringLiteral(R"(^[A-Za-z_][A-Za-z0-9_]*(/[A-Za-z_][A-Za-z0-9_]*)*$)"));
  return frame_pattern.match(frame).hasMatch();
}
} // namespace

RobotSettings& RobotSettings::getInstance()
{
  static RobotSettings instance;
  return instance;
}

RobotSettings::RobotSettings() :
    robot_radius_(DEFAULT_ROBOT_RADIUS),
    max_linear_velocity_(DEFAULT_MAX_LINEAR_VELOCITY),
    max_angular_velocity_(DEFAULT_MAX_ANGULAR_VELOCITY),
    cmd_vel_topic_(QString::fromStdString(ROBOGait::ros::topics::T_CMD_VEL)),
    scan_topic_(QString::fromStdString(ROBOGait::ros::topics::T_SCAN)),
    camera_detection_topic_(QString::fromStdString(ROBOGait::ros::topics::T_CAMERA_DETECTION)),
    map_frame_(QString::fromStdString(ROBOGait::ros::topics::TF_MAP_FRAME)),
    robot_frame_(QString::fromStdString(ROBOGait::ros::topics::TF_ROBOT_FRAME)),
    map_image_path_(DEFAULT_MAP_IMAGE_PATH),
    robot_map_path_(DEFAULT_ROBOT_MAP_PATH)
{
  qDebug() << "[RobotSettings::RobotSettings] RobotSettings created";
}

double RobotSettings::getRobotRadius() const { return robot_radius_; }

double RobotSettings::getMaxLinearVelocity() const { return max_linear_velocity_; }

double RobotSettings::getMaxAngularVelocity() const { return max_angular_velocity_; }

QString RobotSettings::getCmdVelTopic() const { return cmd_vel_topic_; }

QString RobotSettings::getScanTopic() const { return scan_topic_; }

QString RobotSettings::getCameraDetectionTopic() const { return camera_detection_topic_; }

QString RobotSettings::getMapFrame() const { return map_frame_; }

QString RobotSettings::getRobotFrame() const { return robot_frame_; }

QString RobotSettings::getMapImagePath() const { return map_image_path_; }

QString RobotSettings::getRobotMapPath() const { return robot_map_path_; }

void RobotSettings::setRobotRadius(double radius)
{
  if (!isValidPositiveValue(radius))
  {
    qWarning() << "[RobotSettings::setRobotRadius] Radius must be a finite positive value:" << radius;
    return;
  }

  if (robot_radius_ != radius)
  {
    robot_radius_ = radius;
    emit robotRadiusChanged();
  }
}

void RobotSettings::setMaxLinearVelocity(double velocity)
{
  if (!isValidPositiveValue(velocity))
  {
    qWarning() << "[RobotSettings::setMaxLinearVelocity] Velocity must be a finite positive value:" << velocity;
    return;
  }

  if (max_linear_velocity_ != velocity)
  {
    max_linear_velocity_ = velocity;
    emit maxLinearVelocityChanged();
  }
}

void RobotSettings::setMaxAngularVelocity(double velocity)
{
  if (!isValidPositiveValue(velocity))
  {
    qWarning() << "[RobotSettings::setMaxAngularVelocity] Velocity must be a finite positive value:" << velocity;
    return;
  }

  if (max_angular_velocity_ != velocity)
  {
    max_angular_velocity_ = velocity;
    emit maxAngularVelocityChanged();
  }
}

void RobotSettings::setCmdVelTopic(const QString& topic)
{
  const QString normalized_topic = topic.trimmed();
  if (!isValidTopic(normalized_topic))
  {
    qWarning() << "[RobotSettings::setCmdVelTopic] Invalid ROS topic:" << topic;
    return;
  }

  if (cmd_vel_topic_ != normalized_topic)
  {
    cmd_vel_topic_ = normalized_topic;
    emit cmdVelTopicChanged();
  }
}

void RobotSettings::setScanTopic(const QString& topic)
{
  const QString normalized_topic = topic.trimmed();
  if (!isValidTopic(normalized_topic))
  {
    qWarning() << "[RobotSettings::setScanTopic] Invalid ROS topic:" << topic;
    return;
  }

  if (scan_topic_ != normalized_topic)
  {
    scan_topic_ = normalized_topic;
    emit scanTopicChanged();
  }
}

void RobotSettings::setCameraDetectionTopic(const QString& topic)
{
  const QString normalized_topic = topic.trimmed();
  if (!isValidTopic(normalized_topic))
  {
    qWarning() << "[RobotSettings::setCameraDetectionTopic] Invalid ROS topic:" << topic;
    return;
  }

  if (camera_detection_topic_ != normalized_topic)
  {

    camera_detection_topic_ = normalized_topic;
    emit cameraDetectionTopicChanged();
  }
}

void RobotSettings::setMapFrame(const QString& frame)
{
  const QString normalized_frame = frame.trimmed();
  if (!isValidFrame(normalized_frame))
  {
    qWarning() << "[RobotSettings::setMapFrame] Invalid TF frame:" << frame;
    return;
  }

  if (map_frame_ != normalized_frame)
  {
    map_frame_ = normalized_frame;
    emit mapFrameChanged();
  }
}

void RobotSettings::setRobotFrame(const QString& frame)
{
  const QString normalized_frame = frame.trimmed();
  if (!isValidFrame(normalized_frame))
  {
    qWarning() << "[RobotSettings::setRobotFrame] Invalid TF frame:" << frame;
    return;
  }

  if (robot_frame_ != normalized_frame)
  {
    robot_frame_ = normalized_frame;
    emit robotFrameChanged();
  }
}

void RobotSettings::initializeDefaults()
{
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qWarning() << "[RobotSettings::initializeDefaults] YAML configuration not loaded, using constructor defaults";
    return;
  }

  const double robot_radius = yaml_loader.getValue<double>("robot.radius", DEFAULT_ROBOT_RADIUS);
  const double max_linear_velocity = yaml_loader.getValue<double>("robot.max_linear_velocity", DEFAULT_MAX_LINEAR_VELOCITY);
  const double max_angular_velocity = yaml_loader.getValue<double>("robot.max_angular_velocity", DEFAULT_MAX_ANGULAR_VELOCITY);

  robot_radius_ = isValidPositiveValue(robot_radius) ? robot_radius : DEFAULT_ROBOT_RADIUS;
  max_linear_velocity_ = isValidPositiveValue(max_linear_velocity) ? max_linear_velocity : DEFAULT_MAX_LINEAR_VELOCITY;
  max_angular_velocity_ = isValidPositiveValue(max_angular_velocity) ? max_angular_velocity : DEFAULT_MAX_ANGULAR_VELOCITY;

  const QString default_cmd_vel_topic = QString::fromStdString(ROBOGait::ros::topics::T_CMD_VEL);
  const QString default_scan_topic = QString::fromStdString(ROBOGait::ros::topics::T_SCAN);
  const QString default_camera_detection_topic = QString::fromStdString(ROBOGait::ros::topics::T_CAMERA_DETECTION);
  const QString default_map_frame = QString::fromStdString(ROBOGait::ros::topics::TF_MAP_FRAME);
  const QString default_robot_frame = QString::fromStdString(ROBOGait::ros::topics::TF_ROBOT_FRAME);

  const QString cmd_vel_topic = QString::fromStdString(yaml_loader.getValue<std::string>("topics.cmd_vel", default_cmd_vel_topic.toStdString())).trimmed();
  const QString scan_topic = QString::fromStdString(yaml_loader.getValue<std::string>("topics.scan", default_scan_topic.toStdString())).trimmed();
  const QString camera_detection_topic =
      QString::fromStdString(yaml_loader.getValue<std::string>("topics.camera_detection", default_camera_detection_topic.toStdString())).trimmed();
  const QString map_frame = QString::fromStdString(yaml_loader.getValue<std::string>("frames.map", default_map_frame.toStdString())).trimmed();
  const QString robot_frame = QString::fromStdString(yaml_loader.getValue<std::string>("frames.robot", default_robot_frame.toStdString())).trimmed();

  cmd_vel_topic_ = isValidTopic(cmd_vel_topic) ? cmd_vel_topic : default_cmd_vel_topic;
  scan_topic_ = isValidTopic(scan_topic) ? scan_topic : default_scan_topic;
  camera_detection_topic_ = isValidTopic(camera_detection_topic) ? camera_detection_topic : default_camera_detection_topic;
  map_frame_ = isValidFrame(map_frame) ? map_frame : default_map_frame;
  robot_frame_ = isValidFrame(robot_frame) ? robot_frame : default_robot_frame;

  map_image_path_ = QString::fromStdString(yaml_loader.getValue<std::string>("map.image_path", DEFAULT_MAP_IMAGE_PATH));
  robot_map_path_ = QString::fromStdString(yaml_loader.getValue<std::string>("map.map_path", DEFAULT_ROBOT_MAP_PATH));

  if (map_image_path_.isEmpty())
  {
    map_image_path_ = DEFAULT_MAP_IMAGE_PATH;
  }
  if (robot_map_path_.isEmpty())
  {
    robot_map_path_ = DEFAULT_ROBOT_MAP_PATH;
  }

  emit robotRadiusChanged();
  emit maxLinearVelocityChanged();
  emit maxAngularVelocityChanged();
  emit cmdVelTopicChanged();
  emit scanTopicChanged();
  emit cameraDetectionTopicChanged();
  emit mapFrameChanged();
  emit robotFrameChanged();
  emit mapImagePathChanged();
  emit robotMapPathChanged();
}