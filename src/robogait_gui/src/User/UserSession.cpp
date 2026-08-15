#include <QDebug>

#include "User/UserSession.hpp"

using namespace ROBOGait::session;

UserSession::UserSession(ROBOGait::ros::manager::RosNodeManager* ros_manager) :
    db_manager_(&ROBOGait::db::DataBaseManager::getInstance()),
    ros_manager_(ros_manager),
    is_authenticated_(false),
    user_id_(-1),
    username_(""),
    display_name_(""),
    role_(""),
    robot_namespace_(""),
    current_patient_(nullptr),
    current_map_name_(""),
    session_state_dirty_(true),
    is_fully_configured_cache_(false)
{
  qInfo() << "[UserSession::UserSession] UserSession created";

  if (db_manager_)
  {
    // clang-format off
    connect(db_manager_,
            &ROBOGait::db::DataBaseManager::passLoginChanged,
            this,
            &UserSession::onDatabaseAuthChanged
    );
    // clang-format on
  }

  if (ros_manager_ && ros_manager_->getRobotManager())
  {
    // clang-format off
    connect(ros_manager_->getRobotManager(),
            &ROBOGait::robot::manager::RobotManager::selectedRobotNamespaceChanged,
            this,
            &UserSession::onRobotManagerChanged
    );

    connect(ros_manager_->getRobotManager(),
            &ROBOGait::robot::manager::RobotManager::robotDisconnected,
            this,
            &UserSession::onRobotDisconnected
    );
    // clang-format on
  }

  current_patient_ = std::make_unique<ROBOGait::user::Patient>();

  if (current_patient_)
  {
    // clang-format off
    connect(current_patient_.get(),
            &ROBOGait::user::Patient::patientChanged,
            this,
            &UserSession::patientChanged
    );
    // clang-format on
  }
}

UserSession::~UserSession() { qInfo() << "[UserSession::~UserSession] UserSession destroyed"; }

bool UserSession::isAuthenticated() const { return is_authenticated_; }

QString UserSession::getUsername() const { return username_; }

QString UserSession::getDisplayName() const { return display_name_; }

QString UserSession::getRole() const { return role_; }

bool UserSession::hasRobotAssigned() const { return !robot_namespace_.isEmpty(); }

QString UserSession::getRobotNamespace() const { return robot_namespace_; }

QString UserSession::getRobotDisplayName() const
{
  if (robot_namespace_.isEmpty())
  {
    return "";
  }

  QString display = robot_namespace_;
  if (display.startsWith('/'))
  {
    display.remove(0, 1);
  }
  return display.replace("_", " ");
}

bool UserSession::hasPatientAssigned() const { return current_patient_ && current_patient_->isActive(); }

ROBOGait::user::Patient* UserSession::getCurrentPatient() const { return current_patient_.get(); }

bool UserSession::hasMapAssigned() const { return !current_map_name_.isEmpty(); }

QString UserSession::getCurrentMapName() const { return current_map_name_; }

bool UserSession::isFullyConfigured() const
{
  if (session_state_dirty_)
  {
    is_fully_configured_cache_ = isAuthenticated() && hasRobotAssigned() && hasPatientAssigned();
    session_state_dirty_ = false;
  }
  return is_fully_configured_cache_;
}

QString UserSession::getSessionSummary() const
{
  QStringList summary;

  if (isAuthenticated())
  {
    summary << QString("User: %1 (%2)").arg(getDisplayName(), getRole());
  }
  else
  {
    summary << "User: Not authenticated";
  }

  if (hasRobotAssigned())
  {
    summary << QString("Robot: %1").arg(getRobotDisplayName());
  }
  else
  {
    summary << "Robot: Not assigned";
  }

  if (hasPatientAssigned())
  {
    summary << QString("Patient: %1").arg(current_patient_->getDisplayName());
  }
  else
  {
    summary << "Patient: Not assigned";
  }

  if (hasMapAssigned())
  {
    summary << QString("Map: %1").arg(getCurrentMapName());
  }
  else
  {
    summary << "Map: Not assigned";
  }

  summary << QString("Status: %1").arg(isFullyConfigured() ? "Ready" : "Incomplete");

  return summary.join(" | ");
}

ROBOGait::db::DataBaseManager* UserSession::getDatabase() const { return db_manager_; }

ROBOGait::ros::manager::RosNodeManager* UserSession::getRosManager() const { return ros_manager_; }

void UserSession::authenticateUser(int user_id, const QString& username, const QString& display_name, const QString& role)
{
  if (is_authenticated_ && (user_id_ != user_id || username_ != username))
  {
    qDebug() << "[UserSession::authenticateUser] User change detected, clearing previous state";
    clearUserState();
  }

  // Set new user state
  user_id_ = user_id;
  username_ = username;
  display_name_ = display_name;
  role_ = role;
  is_authenticated_ = true;

  updateSessionState();

  emit authenticationChanged();
  emit userChanged();

  qDebug() << "[UserSession::authenticateUser] User authenticated successfully:" << getSessionSummary();
}

void UserSession::assignRobot(const QString& robot_namespace)
{
  if (!isAuthenticated())
  {
    qCritical() << "[UserSession::assignRobot] Cannot assign robot: user not authenticated";
    return;
  }

  if (robot_namespace_ != robot_namespace)
  {
    qDebug() << "[UserSession::assignRobot] Assigning robot:" << robot_namespace << "to user:" << username_;

    robot_namespace_ = robot_namespace;

    if (ros_manager_ && ros_manager_->getRobotManager())
    {
      ros_manager_->getRobotManager()->selectRobot(robot_namespace, ros_manager_->getUseNamespaceDiscovery());
    }

    updateSessionState();
    emit robotChanged();

    qDebug() << "[UserSession::assignRobot] Robot assigned successfully:" << getSessionSummary();
  }
}

void UserSession::assignPatient(int patient_id, const QString& name, const QString& last_name, const QString& display_name)
{
  if (!isAuthenticated())
  {
    qCritical() << "[UserSession::assignPatient] Cannot assign patient: user not authenticated";
    return;
  }

  qDebug() << "[UserSession::assignPatient] Assigning patient:" << display_name << "to user:" << username_;

  if (current_patient_)
  {
    current_patient_->selectPatient(patient_id, name, last_name, display_name);

    if (db_manager_)
    {
      current_patient_->setDoctorDiagnostics(db_manager_->getPatientDoctorDiagnostics(patient_id));
      current_patient_->setTests(db_manager_->getPatientTests(patient_id));
    }
  }

  updateSessionState();
  emit patientChanged();

  qDebug() << "[UserSession::assignPatient] Patient assigned successfully:" << getSessionSummary();
}

void UserSession::assignMap(const QString& map_name)
{
  if (!isAuthenticated())
  {
    qCritical() << "[UserSession::assignMap] Cannot assign map: user not authenticated";
    return;
  }

  if (current_map_name_ != map_name)
  {
    qDebug() << "[UserSession::assignMap] Assigning map:" << map_name << "to user:" << username_;

    current_map_name_ = map_name;

    updateSessionState();
    emit mapChanged();

    qDebug() << "[UserSession::assignMap] Map assigned successfully:" << getSessionSummary();
  }
}

void UserSession::clearRobot()
{
  if (!robot_namespace_.isEmpty())
  {
    qDebug() << "[UserSession::clearRobot] Clearing robot assignment for user:" << username_;

    robot_namespace_.clear();

    if (ros_manager_ && ros_manager_->getRobotManager())
    {
      ros_manager_->getRobotManager()->clearSelection();
    }

    updateSessionState();
    emit robotChanged();
  }
}

void UserSession::clearPatient()
{
  if (hasPatientAssigned())
  {
    qDebug() << "[UserSession::clearPatient] Clearing patient assignment for user:" << username_;

    if (current_patient_)
    {
      current_patient_->clear();
    }

    updateSessionState();
    emit patientChanged();
  }
}

void UserSession::clearMap()
{
  if (!current_map_name_.isEmpty())
  {
    qDebug() << "[UserSession::clearMap] Clearing map assignment for user:" << username_;

    current_map_name_.clear();

    updateSessionState();
    emit mapChanged();
  }
}

void UserSession::clearAllAssignments()
{
  qDebug() << "[UserSession::clearAllAssignments] Clearing all assignments for user:" << username_;

  clearRobot();
  clearPatient();
  clearMap();

  updateSessionState();
  emit sessionStateChanged();
}

void UserSession::logout()
{
  const QString username = username_;

  clearUserState();

  if (db_manager_)
  {
    db_manager_->logout();
    emit db_manager_->userLoggedOut();
  }

  emit authenticationChanged();
  emit userChanged();
  emit sessionStateChanged();

  qDebug() << "[UserSession::logout] Logging out user:" << username;
}

void UserSession::onDatabaseAuthChanged()
{
  if (!db_manager_)
  {
    qCritical() << "[UserSession::onDatabaseAuthChanged] Database manager not available";
    return;
  }

  // Update our state from database
  bool new_auth_state = db_manager_->getPassLogin();
  int new_user_id = db_manager_->getUserId();
  QString new_username = db_manager_->getUserName();
  QString new_display_name = db_manager_->getDisplayName();
  QString new_role = db_manager_->getUserRole();

  if (!new_auth_state)
  {
    if (is_authenticated_)
    {
      clearUserState();
      emit authenticationChanged();
      emit userChanged();
      emit sessionStateChanged();
    }
    return;
  }

  if (!is_authenticated_)
  {
    authenticateUser(new_user_id, new_username, new_display_name, new_role);
    return;
  }

  if (user_id_ != new_user_id || username_ != new_username || display_name_ != new_display_name || role_ != new_role)
  {
    user_id_ = new_user_id;
    username_ = new_username;
    display_name_ = new_display_name;
    role_ = new_role;

    updateSessionState();
    emit userChanged();
  }
}

void UserSession::onRobotManagerChanged()
{
  if (ros_manager_ == nullptr)
  {
    qCritical() << "[UserSession::onRobotManagerChanged] ROS manager not available";
    return;
  }

  if (ros_manager_->getRobotManager() == nullptr)
  {
    qCritical() << "[UserSession::onRobotManagerChanged] Robot manager not available";
    return;
  }

  QString new_robot_namespace = ros_manager_->getRobotManager()->getSelectedRobotNamespace();

  if (robot_namespace_ != new_robot_namespace)
  {
    robot_namespace_ = new_robot_namespace;
    updateSessionState();
    emit robotChanged();
  }
}

void UserSession::onRobotDisconnected()
{
  qDebug() << "[UserSession::onRobotDisconnected] Robot disconnected, clearing robot assignment";

  QString disconnected_robot_name = getRobotDisplayName();

  if (!disconnected_robot_name.isEmpty())
  {
    emit robotDisconnectedWithName(disconnected_robot_name);
  }

  robot_namespace_.clear();

  if (ros_manager_ && ros_manager_->getRobotManager())
  {
    ros_manager_->getRobotManager()->clearSelection();
  }

  updateSessionState();
  emit robotChanged();
}

void UserSession::clearUserState()
{
  clearPatient();
  clearMap();

  // Clear authentication
  is_authenticated_ = false;
  user_id_ = -1;
  username_.clear();
  display_name_.clear();
  role_.clear();

  updateSessionState();
}

void UserSession::updateSessionState()
{
  session_state_dirty_ = true;
  emit sessionStateChanged();
}

bool UserSession::loginUser(const QString& username, const QString& password)
{
  if (!db_manager_)
  {
    qCritical() << "[UserSession::loginUser] Database manager not available";
    return false;
  }

  bool login_success = db_manager_->login(username, password);

  if (!login_success)
  {
    qCritical() << "[UserSession::loginUser] Login failed for user:" << username;
    return false;
  }

  if (!is_authenticated_ || user_id_ != db_manager_->getUserId() || username_ != db_manager_->getUserName() || display_name_ != db_manager_->getDisplayName() ||
      role_ != db_manager_->getUserRole())
  {
    authenticateUser(db_manager_->getUserId(), db_manager_->getUserName(), db_manager_->getDisplayName(), db_manager_->getUserRole());
  }

  return true;
}

bool UserSession::loginGuest(const QString& display_name)
{
  if (!db_manager_)
  {
    qCritical() << "[UserSession::loginGuest] Database manager not available";
    return false;
  }

  const QString trimmed = display_name.trimmed();
  if (trimmed.isEmpty())
  {
    qCritical() << "[UserSession::loginGuest] Display name is empty";
    return false;
  }

  db_manager_->loginGuest(trimmed);
  if (!is_authenticated_ || user_id_ != db_manager_->getUserId() || username_ != db_manager_->getUserName() || display_name_ != db_manager_->getDisplayName() ||
      role_ != db_manager_->getUserRole())
  {
    authenticateUser(db_manager_->getUserId(), db_manager_->getUserName(), db_manager_->getDisplayName(), db_manager_->getUserRole());
  }
  return true;
}

QVariantList UserSession::getUserPatients()
{
  if (!db_manager_)
  {
    qCritical() << "[UserSession::getUserPatients] Database manager not available";
    return QVariantList();
  }

  if (!is_authenticated_)
  {
    qCritical() << "[UserSession::getUserPatients] User not authenticated";
    return QVariantList();
  }

  QVariantList patients_list = db_manager_->listPatients();

  qDebug() << "[UserSession::getUserPatients] Retrieved" << patients_list.size() << "patients for user:" << username_;

  return patients_list;
}

bool UserSession::selectPatientById(int patient_id)
{
  if (!db_manager_)
  {
    qCritical() << "[UserSession::selectPatientById] Database manager not available";
    return false;
  }

  if (!is_authenticated_)
  {
    qCritical() << "[UserSession::selectPatientById] User not authenticated";
    return false;
  }

  QVariantMap patient_details = db_manager_->getPatientDetails(patient_id);

  if (patient_details.isEmpty())
  {
    qCritical() << "[UserSession::selectPatientById] Patient not found with ID:" << patient_id;
    return false;
  }

  QString name = patient_details.value("first_name").toString();
  QString last_name = patient_details.value("last_name").toString();
  QString display_name = patient_details.value("display_name").toString();

  if (display_name.isEmpty())
  {
    display_name = QString("%1 %2").arg(name, last_name);
  }

  assignPatient(patient_id, name, last_name, display_name);

  qDebug() << "[UserSession::selectPatientById] Patient selected:" << display_name << "(ID:" << patient_id << ")";

  return true;
}

bool UserSession::quickSetup(const QString& username, const QString& password, const QString& robot_namespace)
{
  if (!db_manager_)
  {
    qCritical() << "[UserSession::quickSetup] Database manager not available";
    return false;
  }

  if (!loginUser(username, password))
  {
    qCritical() << "[UserSession::quickSetup] Login failed for user:" << username;
    return false;
  }

  assignRobot(robot_namespace);

  qDebug() << "[UserSession::quickSetup] Quick setup completed for user:" << username << "with robot:" << robot_namespace;

  return true;
}

bool UserSession::canStartExperiment()
{
  bool can_start = is_authenticated_ && !robot_namespace_.isEmpty() && hasPatientAssigned();

  qDebug() << "[UserSession::canStartExperiment] Can start experiment:" << can_start << "- Auth:" << is_authenticated_
           << "- Robot:" << !robot_namespace_.isEmpty() << "- Patient:" << hasPatientAssigned();

  return can_start;
}
