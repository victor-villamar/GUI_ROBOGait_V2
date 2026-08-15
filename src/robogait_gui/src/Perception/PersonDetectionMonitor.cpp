#include <functional>

#include <QBuffer>
#include <QDebug>
#include <QImage>

#include "Context/RobotContext.hpp"
#include "Perception/PersonDetectionMonitor.hpp"
#include "Ros/QoSProfiles.hpp"
#include "Ros/TopicsName.hpp"

namespace
{
QImage buildQImageFromRosImage(const sensor_msgs::msg::Image& image)
{
  const uchar* data = image.data.data();
  const int width = static_cast<int>(image.width);
  const int height = static_cast<int>(image.height);
  const int bytes_per_line = static_cast<int>(image.step);

  if (image.encoding == "rgba8")
  {
    return QImage(data, width, height, bytes_per_line, QImage::Format_RGBA8888).copy();
  }
  if (image.encoding == "rgb8")
  {
    return QImage(data, width, height, bytes_per_line, QImage::Format_RGB888).copy();
  }
  if (image.encoding == "bgr8")
  {
    return QImage(data, width, height, bytes_per_line, QImage::Format_BGR888).copy();
  }
  if (image.encoding == "mono8")
  {
    return QImage(data, width, height, bytes_per_line, QImage::Format_Grayscale8).copy();
  }

  qCritical() << "[buildQImageFromRosImage] Unsupported image encoding:" << image.encoding.c_str();
  return QImage();
}
} // namespace

using namespace ROBOGait::perception::monitor;

PersonDetectionMonitor::PersonDetectionMonitor(QObject* parent) :
    QObject(parent), parent_node_(nullptr), selected_robot_namespace_(), use_namespace_discovery_(true), sub_camera_detection_(nullptr), is_monitoring_(false)
{
}

void PersonDetectionMonitor::setROSNode(rclcpp::Node* parent_node)
{
  if (parent_node_ == parent_node)
  {
    return;
  }

  stopMonitoring();
  parent_node_ = parent_node;
}

void PersonDetectionMonitor::setSelectedRobot(const QString& robot_namespace, bool use_namespace_discovery)
{
  if (is_monitoring_)
  {
    stopMonitoring();
  }

  selected_robot_namespace_ = robot_namespace;
  use_namespace_discovery_ = use_namespace_discovery;
}

bool PersonDetectionMonitor::startMonitoring()
{
  if (is_monitoring_)
  {
    return true;
  }

  if (!parent_node_)
  {
    qCritical() << "[PersonDetectionMonitor::startMonitoring] Parent ROS node is not set";
    return false;
  }

  ROBOGait::context::RobotContext context;

  if (!context.setSelectedRobot(selected_robot_namespace_, use_namespace_discovery_))
  {
    qCritical() << "[PersonDetectionMonitor::startMonitoring] Robot context is not configured";
    return false;
  }

  const std::string full_camera_topic = context.resolveTopic(ROBOGait::ros::topics::T_CAMERA_DETECTION);

  sub_camera_detection_ = parent_node_->create_subscription<navigation_pkg::msg::CameraDetection>(
      full_camera_topic, ROBOGait::ros::QosProfiles::QOS_BEST_EFFORT(),
      std::bind(&PersonDetectionMonitor::callbackCameraDetection, this, std::placeholders::_1));

  is_monitoring_ = true;
  emit monitoringChanged();

  qDebug() << "[PersonDetectionMonitor::startMonitoring] Monitoring started on topic:" << full_camera_topic.c_str();
  return true;
}

void PersonDetectionMonitor::stopMonitoring()
{
  if (!is_monitoring_ && !sub_camera_detection_)
  {
    return;
  }

  sub_camera_detection_.reset();

  if (is_monitoring_)
  {
    is_monitoring_ = false;
    emit monitoringChanged();
  }

  qDebug() << "[PersonDetectionMonitor::stopMonitoring] Monitoring stopped";
}

bool PersonDetectionMonitor::isMonitoring() const { return is_monitoring_; }

void PersonDetectionMonitor::callbackCameraDetection(const navigation_pkg::msg::CameraDetection::ConstSharedPtr msg)
{
  if (!is_monitoring_)
  {
    return;
  }

  if (!msg)
  {
    return;
  }

  if (msg->user_detection.detection <= NOT_DETECTED)
  {
    return;
  }

  const int detections = msg->user_detection.detection;
  const QString image_source = imageToDataUrl(msg->snapshot);

  emit cameraPersonDetected(detections, image_source);
  stopMonitoring();
}

QString PersonDetectionMonitor::imageToDataUrl(const sensor_msgs::msg::Image& image) const
{
  if (image.height == 0 || image.width == 0 || image.step == 0 || image.data.empty())
  {
    qCritical() << "[PersonDetectionMonitor::imageToDataUrl] Empty image received";
    return QString();
  }

  const qsizetype required_size = static_cast<qsizetype>(image.step) * static_cast<qsizetype>(image.height);
  if (static_cast<qsizetype>(image.data.size()) < required_size)
  {
    qCritical() << "[PersonDetectionMonitor::imageToDataUrl] Image data is smaller than expected";
    return QString();
  }

  const QImage q_image = buildQImageFromRosImage(image);

  if (q_image.isNull())
  {
    return QString();
  }

  QByteArray png_data;
  QBuffer buffer(&png_data);
  buffer.open(QIODevice::WriteOnly);

  if (!q_image.save(&buffer, "PNG"))
  {
    qCritical() << "[PersonDetectionMonitor::imageToDataUrl] Failed to encode image as PNG";
    return QString();
  }

  return QStringLiteral("data:image/png;base64,") + QString::fromLatin1(png_data.toBase64());
}
