#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "DataBase/DataBaseManager.hpp"
#include "Ros/RosNodeManager.hpp"
#include "User/Patient.hpp"

namespace ROBOGait
{
namespace session
{

/**
 * @brief Central session manager for user-centric application state
 *
 * This class manages all user-related state including authentication,
 * assigned resources (robot, patient, map), and ensures proper cleanup
 * when switching users.
 */
class UserSession : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Constructor of UserSession class
   *
   * @param ros_manager ROS node manager instance
   */
  UserSession(ROBOGait::ros::manager::RosNodeManager* ros_manager);

  /**
   * @brief Destructor of UserSession class
   */
  ~UserSession() override;

  // clang-format off
  // User properties
  Q_PROPERTY(bool isAuthenticated
             READ isAuthenticated
             NOTIFY authenticationChanged)
             
  Q_PROPERTY(QString username
             READ getUsername
             NOTIFY userChanged)
             
  Q_PROPERTY(QString displayName
             READ getDisplayName
             NOTIFY userChanged)
             
  Q_PROPERTY(QString role
             READ getRole
             NOTIFY userChanged)

  // Robot properties  
  Q_PROPERTY(bool hasRobotAssigned
             READ hasRobotAssigned
             NOTIFY robotChanged)
             
  Q_PROPERTY(QString robotNamespace
             READ getRobotNamespace
             NOTIFY robotChanged)
             
  Q_PROPERTY(QString robotDisplayName
             READ getRobotDisplayName
             NOTIFY robotChanged)

  // Patient properties
  Q_PROPERTY(bool hasPatientAssigned
             READ hasPatientAssigned
             NOTIFY patientChanged)
             
  Q_PROPERTY(ROBOGait::user::Patient* currentPatient
             READ getCurrentPatient
             NOTIFY patientChanged)

  // Map properties
  Q_PROPERTY(bool hasMapAssigned
             READ hasMapAssigned
             NOTIFY mapChanged)
             
  Q_PROPERTY(QString currentMapName
             READ getCurrentMapName
             NOTIFY mapChanged)

  // Session state
  Q_PROPERTY(bool isFullyConfigured
             READ isFullyConfigured
             NOTIFY sessionStateChanged)
             
  Q_PROPERTY(QString sessionSummary
             READ getSessionSummary
             NOTIFY sessionStateChanged)

  // Access to managers
  Q_PROPERTY(ROBOGait::db::DataBaseManager* database
             READ getDatabase
             CONSTANT)

  Q_PROPERTY(ROBOGait::ros::manager::RosNodeManager* rosManager
             READ getRosManager
             CONSTANT)
  // clang-format on

  /**
   * @brief Check if user is authenticated
   *
   * @return True if user is authenticated, false otherwise
   */
  bool isAuthenticated() const;

  /**
   * @brief Get current username
   *
   * @return Current username
   */
  QString getUsername() const;

  /**
   * @brief Get display name
   *
   * @return Current display name
   */
  QString getDisplayName() const;

  /**
   * @brief Get user role
   *
   * @return Current user role
   */
  QString getRole() const;

  /**
   * @brief Check if robot is assigned
   *
   * @return True if robot is assigned, false otherwise
   */
  bool hasRobotAssigned() const;

  /**
   * @brief Get robot namespace
   *
   * @return Current robot namespace
   */
  QString getRobotNamespace() const;

  /**
   * @brief Get robot display name
   *
   * @return Current robot display name
   */
  QString getRobotDisplayName() const;

  /**
   * @brief Check if patient is assigned
   *
   * @return True if patient is assigned, false otherwise
   */
  bool hasPatientAssigned() const;

  /**
   * @brief Get current patient
   *
   * @return Current patient object or nullptr if not assigned
   */
  ROBOGait::user::Patient* getCurrentPatient() const;

  /**
   * @brief Check if map is assigned
   *
   * @return True if map is assigned, false otherwise
   */
  bool hasMapAssigned() const;

  /**
   * @brief Get current map name
   *
   * @return Current map name
   */
  QString getCurrentMapName() const;

  /**
   * @brief Check if session is fully configured
   *
   * @return True if session is fully configured, false otherwise
   */
  bool isFullyConfigured() const;

  /**
   * @brief Get session summary for debugging/display
   *
   * @return Current session summary
   */
  QString getSessionSummary() const;

  /**
   * @brief Get database manager
   *
   * @return Pointer to database manager
   */
  ROBOGait::db::DataBaseManager* getDatabase() const;

  /**
   * @brief Get ROS manager
   *
   * @return Pointer to ROS manager
   */
  ROBOGait::ros::manager::RosNodeManager* getRosManager() const;

public slots:
  /**
   * @brief Authenticate user and clear previous session
   *
   * @param user_id User ID from database
   * @param username Username
   * @param display_name Display name
   * @param role User role
   */
  void authenticateUser(int user_id, const QString& username, const QString& display_name, const QString& role);

  /**
   * @brief Assign robot to current user
   *
   * @param robot_namespace ROS namespace of the robot
   */
  Q_INVOKABLE void assignRobot(const QString& robot_namespace);

  /**
   * @brief Assign patient to current user
   *
   * @param patient_id Patient ID from database
   * @param name Patient name
   * @param last_name Patient last name
   * @param display_name Patient display name
   */
  Q_INVOKABLE void assignPatient(int patient_id, const QString& name, const QString& last_name, const QString& display_name);

  /**
   * @brief Assign map to current user
   *
   * @param map_name Map name
   */
  Q_INVOKABLE void assignMap(const QString& map_name);

  /**
   * @brief Clear robot assignment
   */
  Q_INVOKABLE void clearRobot();

  /**
   * @brief Clear patient assignment
   */
  Q_INVOKABLE void clearPatient();

  /**
   * @brief Clear map assignment
   */
  Q_INVOKABLE void clearMap();

  /**
   * @brief Clear all assignments (keep user authenticated)
   */
  Q_INVOKABLE void clearAllAssignments();

  /**
   * @brief Logout user and clear entire session
   */
  Q_INVOKABLE void logout();

  /**
   * @brief Attempt login and setup user session
   *
   * @param username Username to login
   * @param password Password for authentication
   *
   * @return true if login successful, false otherwise
   */
  Q_INVOKABLE bool loginUser(const QString& username, const QString& password);

  /**
   * @brief Get patients list for current authenticated user
   *
   * @return QVariantList of patients associated with current user
   */
  Q_INVOKABLE QVariantList getUserPatients();

  /**
   * @brief Select patient by ID and assign to session
   *
   * @param patient_id Patient ID to select and assign
   *
   * @return true if patient found and assigned, false otherwise
   */
  Q_INVOKABLE bool selectPatientById(int patient_id);

  /**
   * @brief Quick setup: login user and assign robot in one call
   *
   * @param username Username
   * @param password Password
   * @param robot_namespace Robot namespace to assign
   *
   * @return true if both login and robot assignment successful
   */
  Q_INVOKABLE bool quickSetup(const QString& username, const QString& password, const QString& robot_namespace);

  /**
   * @brief Check if current session can start experimentation
   *
   * @return true if user, robot and patient are assigned
   */
  Q_INVOKABLE bool canStartExperiment();

signals:
  void authenticationChanged();
  void userChanged();
  void robotChanged();
  void robotDisconnectedWithName(QString robotDisplayName);
  void patientChanged();
  void mapChanged();
  void sessionStateChanged();

private slots:
  void onDatabaseAuthChanged();
  void onRobotManagerChanged();
  void onRobotDisconnected();

private:
  /**
   * @brief Clear all user-specific state
   */
  void clearUserState();

  /**
   * @brief Update session state and emit signals if needed
   */
  void updateSessionState();

  // Managers
  ROBOGait::db::DataBaseManager* db_manager_;           /**< Database manager */
  ROBOGait::ros::manager::RosNodeManager* ros_manager_; /**< ROS node manager */

  // User state
  bool is_authenticated_; /**< User authentication status */
  int user_id_;           /**< User ID */
  QString username_;      /**< Username */
  QString display_name_;  /**< Display name */
  QString role_;          /**< User role */

  // Robot state
  QString robot_namespace_; /**< Robot namespace */

  // Patient state
  std::unique_ptr<ROBOGait::user::Patient> current_patient_; /**< Current patient */

  // Map state
  QString current_map_name_; /**< Current map name */

  // Session state cache
  mutable bool session_state_dirty_;       /**< Session state dirty flag */
  mutable bool is_fully_configured_cache_; /**< Fully configured cache flag */
};

} // namespace session
} // namespace ROBOGait
