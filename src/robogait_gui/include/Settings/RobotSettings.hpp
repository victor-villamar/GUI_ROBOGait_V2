#pragma once

#include <QObject>
#include <QString>

namespace ROBOGait
{
namespace settings
{

/**
 * @brief Runtime-editable robot, topic and frame configuration
 *
 * Values are loaded from the shared application YAML. Editable properties are
 * validated, kept in memory for the current session and notify their consumers.
 * Map paths are exposed as information only because they cannot be changed
 * safely while the Docker environment is running.
 */
class RobotSettings : public QObject
{
  Q_OBJECT

public:
  static RobotSettings& getInstance();

  // Delete copy constructor and assignment operator to enforce singleton pattern
  RobotSettings(const RobotSettings&) = delete;
  RobotSettings& operator=(const RobotSettings&) = delete;

  // clang-format off
  Q_PROPERTY(double robotRadius
             READ getRobotRadius
             WRITE setRobotRadius
             NOTIFY robotRadiusChanged)

  Q_PROPERTY(double maxLinearVelocity
             READ getMaxLinearVelocity
             WRITE setMaxLinearVelocity
             NOTIFY maxLinearVelocityChanged)

  Q_PROPERTY(double maxAngularVelocity
             READ getMaxAngularVelocity
             WRITE setMaxAngularVelocity
             NOTIFY maxAngularVelocityChanged)

  Q_PROPERTY(QString cmdVelTopic
             READ getCmdVelTopic
             WRITE setCmdVelTopic
             NOTIFY cmdVelTopicChanged)

  Q_PROPERTY(QString scanTopic
             READ getScanTopic
             WRITE setScanTopic
             NOTIFY scanTopicChanged)

  Q_PROPERTY(QString cameraDetectionTopic
             READ getCameraDetectionTopic
             WRITE setCameraDetectionTopic
             NOTIFY cameraDetectionTopicChanged)

  Q_PROPERTY(QString mapFrame
             READ getMapFrame
             WRITE setMapFrame
             NOTIFY mapFrameChanged)

  Q_PROPERTY(QString robotFrame
             READ getRobotFrame
             WRITE setRobotFrame
             NOTIFY robotFrameChanged)

  Q_PROPERTY(QString mapImagePath
             READ getMapImagePath
             NOTIFY mapImagePathChanged)

  Q_PROPERTY(QString robotMapPath
             READ getRobotMapPath
             NOTIFY robotMapPathChanged)
  // clang-format on

  double getRobotRadius() const;
  double getMaxLinearVelocity() const;
  double getMaxAngularVelocity() const;
  QString getCmdVelTopic() const;
  QString getScanTopic() const;
  QString getCameraDetectionTopic() const;
  QString getMapFrame() const;
  QString getRobotFrame() const;
  QString getMapImagePath() const;
  QString getRobotMapPath() const;

  void setRobotRadius(double radius);
  void setMaxLinearVelocity(double velocity);
  void setMaxAngularVelocity(double velocity);
  void setCmdVelTopic(const QString& topic);
  void setScanTopic(const QString& topic);
  void setCameraDetectionTopic(const QString& topic);
  void setMapFrame(const QString& frame);
  void setRobotFrame(const QString& frame);

  /**
   * @brief Load all properties from the currently loaded YAML configuration
   */
  void initializeDefaults();

signals:
  void robotRadiusChanged();          // emitted when robot radius is changed
  void maxLinearVelocityChanged();    // emitted when max linear velocity is changed
  void maxAngularVelocityChanged();   // emitted when max angular velocity is changed
  void cmdVelTopicChanged();          // emitted when cmd_vel topic is changed
  void scanTopicChanged();            // emitted when scan topic is changed
  void cameraDetectionTopicChanged(); // emitted when camera detection topic is changed
  void mapFrameChanged();             // emitted when map frame is changed
  void robotFrameChanged();           // emitted when robot frame is changed
  void mapImagePathChanged();         // emitted when map image path is changed
  void robotMapPathChanged();         // emitted when robot map path is changed

private:
  /**
   * @brief Private constructor for singleton pattern
   */
  RobotSettings();

  /**
   * @brief Destructor
   */
  ~RobotSettings() override = default;

  double robot_radius_;
  double max_linear_velocity_;
  double max_angular_velocity_;
  QString cmd_vel_topic_;
  QString scan_topic_;
  QString camera_detection_topic_;
  QString map_frame_;
  QString robot_frame_;
  QString map_image_path_;
  QString robot_map_path_;

  static constexpr double DEFAULT_ROBOT_RADIUS = 0.326;
  static constexpr double DEFAULT_MAX_LINEAR_VELOCITY = 0.22;
  static constexpr double DEFAULT_MAX_ANGULAR_VELOCITY = 2.84;
  static constexpr const char* DEFAULT_MAP_IMAGE_PATH = ".local/robogait/maps/";
  static constexpr const char* DEFAULT_ROBOT_MAP_PATH = "/maps/";
};
} // namespace settings
} // namespace ROBOGait