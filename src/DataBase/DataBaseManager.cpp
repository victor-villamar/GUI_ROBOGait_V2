#include <QCryptographicHash>
#include <QDebug>

#include "DataBase/Common/DbTypes.hpp"
#include "DataBase/DataBaseManager.hpp"

using namespace ROBOGait::db;

DataBaseManager::DataBaseManager() : db_(), user_repository_(db_), patient_repository_(db_), map_repository_(db_), experiment_repository_(db_)
{
  qInfo() << "[DataBaseManager::DataBaseManager] DataBaseManager created";

  pass_login_ = false;
  pass_check_user_name_ = false;
  user_name_ = "";
  display_name_ = "";
  user_role_ = "";
  last_error_ = "";
}

DataBaseManager::~DataBaseManager() { qInfo() << "[DataBaseManager::~DataBaseManager] DataBaseManager destroyed"; }

bool DataBaseManager::getPassLogin() const { return pass_login_; }

bool DataBaseManager::getPassCheckUserName() const { return pass_check_user_name_; }

QString DataBaseManager::getUserName() const { return user_name_; }

QString DataBaseManager::getDisplayName() const { return display_name_; }

QString DataBaseManager::getUserRole() const { return user_role_; }

QString DataBaseManager::getLastError() const { return last_error_; }

bool DataBaseManager::openDatabase(const QString& db_path)
{
  const auto result = db_.open(db_path);
  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    qCritical() << "[DataBaseManager::openDatabase] Failed to open database:" << error.message;
    return false;
  }

  setLastError("");
  qInfo() << "[DataBaseManager::openDatabase] Database opened successfully at path:" << db_path;
  return true;
}

bool DataBaseManager::login(const QString& username, const QString& password)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::login] Database is not open";
    setLastError("Database is not open");
    setPassLogin(false);
    return false;
  }
  const auto password_hash = hashPasswordSha256Hex(password);
  const auto result = user_repository_.findByCredentials(username, password_hash);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    qCritical() << "[DataBaseManager::login] Failed to login:" << error.message;
    setLastError(error.message);
    setPassLogin(false);
    return false;
  }

  const auto maybe_user = std::get<std::optional<UserRow>>(result);

  if (!maybe_user)
  {
    qCritical() << "[DataBaseManager::login] Invalid credentials";
    setLastError("Invalid credentials");
    setPassLogin(false);
    return false;
  }

  setUserName(maybe_user->user_name);
  setDisplayName(maybe_user->name);
  setUserRole(userRoleToDbString(maybe_user->role));
  setLastError("");
  setPassLogin(true);
  qInfo() << "[DataBaseManager::login] Login successful for user:" << maybe_user->user_name;
  return true;
}

bool DataBaseManager::checkUserNameAvailable(const QString& user_name)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::checkUserNameAvailable] Database is not open";
    setLastError("Database is not open");
    setPassCheckUserName(false);
    return false;
  }

  const auto result = user_repository_.userNameExists(user_name);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    qCritical() << "[DataBaseManager::checkUserNameAvailable] Failed to check username availability:" << error.message;
    setLastError(error.message);
    setPassCheckUserName(false);
    return false;
  }

  const bool exists = std::get<bool>(result);
  setPassCheckUserName(!exists);
  setLastError("");
  qInfo() << "[DataBaseManager::checkUserNameAvailable] Username" << user_name << (exists ? "is not available" : "is available");
  return !exists;
}

bool DataBaseManager::registerUser(const QString& name, const QString& last_name, const QString& user_name, const QString& password, const QString& role)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::registerUser] Database is not open";
    setLastError("Database is not open");
    return false;
  }

  const auto role_enum = userRoleFromDbString(role);

  if (role_enum != UserRole::DOCTOR && role_enum != UserRole::MANAGER)
  {
    qCritical() << "[DataBaseManager::registerUser] Invalid role (must be doctor/manager):" << role;
    setLastError("Invalid role (must be doctor/manager)");
    return false;
  }

  const auto exists_result = user_repository_.userNameExists(user_name);

  if (!statusOk(exists_result))
  {
    const auto error = std::get<DbError>(exists_result);
    qCritical() << "[DataBaseManager::registerUser] Failed to check username availability:" << error.message;
    setLastError(error.message);
    return false;
  }

  if (std::get<bool>(exists_result))
  {
    qCritical() << "[DataBaseManager::registerUser] Username already exists:" << user_name;
    setLastError("Username already exists");
    return false;
  }

  const auto password_hash = hashPasswordSha256Hex(password);

  const auto insert_result = user_repository_.insertUser(name, last_name, user_name, password_hash, role_enum);

  if (!statusOk(insert_result))
  {
    const auto error = std::get<DbError>(insert_result);
    qCritical() << "[DataBaseManager::registerUser] Failed to register user:" << error.message;
    setLastError(error.message);
    return false;
  }

  setLastError("");
  qInfo() << "[DataBaseManager::registerUser] User registered successfully:" << user_name;
  return true;
}

void DataBaseManager::loginGuest(const QString& name)
{
  setUserName("");
  setDisplayName(name.trimmed());
  setUserRole("guest");
  setLastError("");
  setPassLogin(true);
  qInfo() << "[DataBaseManager::loginGuest] Guest login successfully";
}

void DataBaseManager::logout()
{
  setUserName("");
  setDisplayName("");
  setUserRole("");
  setLastError("");
  setPassLogin(false);
  setPassCheckUserName(false);
  qInfo() << "[DataBaseManager::logout] User logged out";
}

QString DataBaseManager::hashPasswordSha256Hex(const QString& password) const
{
  const QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
  return QString::fromLatin1(hash.toHex());
}

void DataBaseManager::setPassLogin(bool pass_login)
{
  if (pass_login_ != pass_login)
  {
    pass_login_ = pass_login;
    emit passLoginChanged();
  }
}

void DataBaseManager::setPassCheckUserName(bool pass_check_user_name)
{
  if (pass_check_user_name_ != pass_check_user_name)
  {
    pass_check_user_name_ = pass_check_user_name;
    emit passCheckUserNameChanged();
  }
}

void DataBaseManager::setUserName(const QString& user_name)
{
  if (user_name_ != user_name)
  {
    user_name_ = user_name;
    emit userNameChanged();
  }
}

void DataBaseManager::setDisplayName(const QString& display_name)
{
  if (display_name_ != display_name)
  {
    display_name_ = display_name;
    emit displayNameChanged();
  }
}

void DataBaseManager::setUserRole(const QString& user_role)
{
  if (user_role_ != user_role)
  {
    user_role_ = user_role;
    emit userRoleChanged();
  }
}

void DataBaseManager::setLastError(const QString& last_error)
{
  if (last_error_ != last_error)
  {
    last_error_ = last_error;
    emit lastErrorChanged();
  }
}
