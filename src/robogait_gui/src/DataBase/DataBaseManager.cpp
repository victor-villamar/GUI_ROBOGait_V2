#include <QCryptographicHash>
#include <QDebug>

#include "DataBase/Common/DbTypes.hpp"
#include "DataBase/DataBaseManager.hpp"

using namespace ROBOGait::db;

DataBaseManager& DataBaseManager::getInstance()
{
  static DataBaseManager instance;
  return instance;
}

bool DataBaseManager::initialize(const QString& db_path)
{
  if (is_initialized_)
  {
    qWarning() << "[DataBaseManager::initialize] Already initialized";
    return true;
  }

  qDebug() << "[DataBaseManager::initialize] Initializing database with path:" << db_path;

  const bool result = openDatabase(db_path);

  if (!result)
  {
    qCritical() << "[DataBaseManager::initialize] Failed to initialize database";
    return false;
  }

  is_initialized_ = true;
  return true;
}

bool DataBaseManager::isInitialized() const { return is_initialized_; }

void DataBaseManager::shutdown()
{
  if (!is_initialized_)
  {
    return;
  }

  db_.close();
  is_initialized_ = false;
}

DataBaseManager::DataBaseManager() :
    is_initialized_(false), db_(), user_repository_(db_), patient_repository_(db_), map_repository_(db_), experiment_repository_(db_)
{
  qInfo() << "[DataBaseManager::DataBaseManager] DataBaseManager created";

  pass_login_ = false;
  pass_check_user_name_ = false;
  user_id_ = -1;
  user_name_ = "";
  display_name_ = "";
  user_role_ = "";
  last_error_ = "";
  last_experiment_id_ = -1;
}

DataBaseManager::~DataBaseManager() { qInfo() << "[DataBaseManager::~DataBaseManager] DataBaseManager destroyed"; }

bool DataBaseManager::getPassLogin() const { return pass_login_; }

bool DataBaseManager::getPassCheckUserName() const { return pass_check_user_name_; }

QString DataBaseManager::getUserName() const { return user_name_; }

int DataBaseManager::getUserId() const { return user_id_; }

QString DataBaseManager::getDisplayName() const { return display_name_; }

QString DataBaseManager::getUserRole() const { return user_role_; }

QString DataBaseManager::getLastError() const { return last_error_; }

int DataBaseManager::getLastExperimentId() const { return last_experiment_id_; }

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
    setLastError("La base de datos no esta abierta");
    return false;
  }

  const auto password_hash = hashPasswordSha256Hex(password);
  const auto result = user_repository_.findByCredentials(username, password_hash);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    qCritical() << "[DataBaseManager::login] Failed to login:" << error.message;
    setLastError(error.message);
    return false;
  }

  const auto maybe_user = std::get<std::optional<UserRow>>(result);

  if (!maybe_user)
  {
    qCritical() << "[DataBaseManager::login] Invalid credentials";
    setLastError("Credenciales invalidas");
    return false;
  }

  user_id_ = maybe_user->id;
  setUserName(maybe_user->user_name);
  setDisplayName(maybe_user->name);
  setUserRole(userRoleToDbString(maybe_user->role));
  setLastError("");
  setLastExperimentId(-1);
  setPassLogin(true);
  qDebug() << "[DataBaseManager::login] Login successful for user:" << maybe_user->user_name << "ID:" << maybe_user->id;
  return true;
}

bool DataBaseManager::checkUserNameAvailable(const QString& user_name)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::checkUserNameAvailable] Database is not open";
    setLastError("La base de datos no esta abierta");
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
  qDebug() << "[DataBaseManager::checkUserNameAvailable] Username" << user_name << (exists ? "is not available" : "is available");
  return !exists;
}

bool DataBaseManager::registerUser(const QString& name, const QString& last_name, const QString& user_name, const QString& password, const QString& role)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::registerUser] Database is not open";
    setLastError("La base de datos no esta abierta");
    return false;
  }

  const auto role_enum = userRoleFromDbString(role);

  if (role_enum != UserRole::DOCTOR && role_enum != UserRole::MANAGER)
  {
    qCritical() << "[DataBaseManager::registerUser] Invalid role (must be doctor/manager):" << role;
    setLastError("Rol invalido (debe ser doctor/gerente)");
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
    setLastError("El nombre de usuario ya existe");
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
  qDebug() << "[DataBaseManager::registerUser] User registered successfully:" << user_name;
  return true;
}

void DataBaseManager::loginGuest(const QString& name)
{
  setUserName("");
  setDisplayName(name.trimmed());
  setUserRole("guest");
  setLastError("");
  setLastExperimentId(-1);
  setPassLogin(true);
  qDebug() << "[DataBaseManager::loginGuest] Guest login successfully";
}

void DataBaseManager::logout()
{
  user_id_ = -1;
  setUserName("");
  setDisplayName("");
  setUserRole("");
  setLastError("");
  setLastExperimentId(-1);
  setPassLogin(false);
  setPassCheckUserName(false);
  qDebug() << "[DataBaseManager::logout] User logged out";
}

QVariantList DataBaseManager::listPatients()
{
  QVariantList patients_list;

  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::listPatients] Database is not open";
    setLastError("La base de datos no esta abierta");
    return patients_list;
  }

  if (user_role_ == "guest" || user_name_.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::listPatients] Action not allowed for guest users";
    setLastError("Accion no permitida para usuario invitado");
    return patients_list;
  }

  const auto result = patient_repository_.listPatientsForUserName(user_name_);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return patients_list;
  }

  const auto patients = std::get<QVector<PatientRow>>(result);

  for (const auto& patient : patients)
  {
    QVariantMap patient_map;
    patient_map["id"] = patient.id;
    patient_map["name"] = patient.name;
    patient_map["last_name"] = patient.last_name;
    patient_map["display"] = (patient.last_name + ", " + patient.name).trimmed();
    patients_list.append(patient_map);
  }

  setLastError("");
  qDebug() << "[DataBaseManager::listPatients] Listed" << patients.size() << "patients for user:" << user_name_;
  return patients_list;
}

QVariantMap DataBaseManager::getPatientDetails(int patient_id)
{
  QVariantMap patient_map;

  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::getPatientDetails] Database is not open";
    setLastError("La base de datos no esta abierta");
    return patient_map;
  }

  if (user_role_ == "guest" || user_name_.trimmed().isEmpty())
  {
    qWarning() << "[DataBaseManager::getPatientDetails] Action not allowed for guest users";
    setLastError("Accion no permitida para usuario invitado");
    return patient_map;
  }

  if (patient_id <= 0)
  {
    qWarning() << "[DataBaseManager::getPatientDetails] Invalid patient ID";
    setLastError("ID de paciente invalido");
    return patient_map;
  }

  const auto result = patient_repository_.getPatientDetailsByIdForUserName(patient_id, user_name_);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return patient_map;
  }

  const auto patient_details = std::get<PatientDetails>(result);

  setLastError("");
  qDebug() << "[DataBaseManager::getPatientDetails] Retrieved details for patient ID:" << patient_id;
  return toVariantMap(std::get<PatientDetails>(result));
}

QVariantMap DataBaseManager::getPatientBasicInfo(int patient_id)
{
  QVariantMap patient_map;

  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::getPatientBasicInfo] Database is not open";
    setLastError("La base de datos no esta abierta");
    return patient_map;
  }

  if (user_role_ == "guest" || user_name_.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::getPatientBasicInfo] Action not allowed for guest users";
    setLastError("Accion no permitida para usuario invitado");
    return patient_map;
  }

  if (patient_id <= 0)
  {
    qCritical() << "[DataBaseManager::getPatientBasicInfo] Invalid patient ID";
    setLastError("ID de paciente invalido");
    return patient_map;
  }

  const auto result = patient_repository_.getPatientBasicInfoByIdForUserName(patient_id, user_name_);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return patient_map;
  }

  const auto patient_row = std::get<PatientRow>(result);

  patient_map["id"] = patient_row.id;
  patient_map["name"] = patient_row.name;
  patient_map["last_name"] = patient_row.last_name;
  patient_map["display"] = (patient_row.last_name + ", " + patient_row.name).trimmed();

  setLastError("");
  qDebug() << "[DataBaseManager::getPatientBasicInfo] Retrieved basic info for patient ID:" << patient_id;
  return patient_map;
}

QVariantList DataBaseManager::getPatientDoctorDiagnostics(int patient_id)
{
  QVariantList diagnostics_list;

  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::getPatientDoctorDiagnostics] Database is not open";
    setLastError("La base de datos no esta abierta");
    return diagnostics_list;
  }

  if (user_role_ == "guest" || user_name_.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::getPatientDoctorDiagnostics] Action not allowed for guest users";
    setLastError("Accion no permitida para usuario invitado");
    return diagnostics_list;
  }

  if (patient_id <= 0)
  {
    qCritical() << "[DataBaseManager::getPatientDoctorDiagnostics] Invalid patient ID";
    setLastError("ID de paciente invalido");
    return diagnostics_list;
  }

  const auto result = patient_repository_.listPatientDoctorsByPatientIdForUserName(patient_id, user_name_);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return diagnostics_list;
  }

  const auto doctors = std::get<QVector<PatientDoctorInfo>>(result);
  for (const auto& doctor : doctors)
  {
    QVariantMap map;
    map["doctor_id"] = doctor.doctor_id;
    map["doctor_name"] = doctor.doctor_name;
    map["doctor_last_name"] = doctor.doctor_last_name;
    map["display"] = (doctor.doctor_last_name + ", " + doctor.doctor_name).trimmed();
    map["create_day"] = doctor.create_day;
    map["description"] = doctor.description;
    diagnostics_list.append(map);
  }

  setLastError("");
  qDebug() << "[DataBaseManager::getPatientDoctorDiagnostics] Retrieved" << doctors.size() << "doctors for patient ID:" << patient_id;
  return diagnostics_list;
}

QVariantList DataBaseManager::getPatientTests(int patient_id)
{
  QVariantList tests_list;

  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::getPatientTests] Database is not open";
    setLastError("La base de datos no esta abierta");
    return tests_list;
  }

  if (user_role_ == "guest" || user_name_.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::getPatientTests] Action not allowed for guest users";
    setLastError("Accion no permitida para usuario invitado");
    return tests_list;
  }

  if (patient_id <= 0)
  {
    qCritical() << "[DataBaseManager::getPatientTests] Invalid patient ID";
    setLastError("ID de paciente invalido");
    return tests_list;
  }

  const auto result = experiment_repository_.listPatientTestsByPatientIdForUserName(patient_id, user_name_);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return tests_list;
  }

  const auto tests = std::get<QVector<PatientTestInfo>>(result);
  for (const auto& test : tests)
  {
    QVariantMap map;
    map["experiment_id"] = test.experiment_id;
    map["patient_name"] = test.patient_name;
    map["patient_last_name"] = test.patient_last_name;
    map["doctor_name"] = test.doctor_name;
    map["doctor_last_name"] = test.doctor_last_name;
    map["date"] = test.date;
    map["map_name"] = test.map_name;
    map["location"] = test.map_location;
    map["comment"] = test.comment;
    tests_list.append(map);
  }

  setLastError("");
  qDebug() << "[DataBaseManager::getPatientTests] Retrieved" << tests.size() << "tests for patient ID:" << patient_id;
  return tests_list;
}

bool DataBaseManager::registerPatient(const QString& name, const QString& last_name, int age, double weight, double height, const QString& description)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::registerPatient] Database is not open";
    setLastError("La base de datos no esta abierta");
    return false;
  }

  if (user_role_ == "guest" || user_name_.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::registerPatient] Action not allowed for guest users";
    setLastError("Accion no permitida para usuario invitado");
    return false;
  }

  if (name.isEmpty() || last_name.isEmpty())
  {
    qCritical() << "[DataBaseManager::registerPatient] Name or last name is empty";
    setLastError("Nombres y apellidos son obligatorios");
    return false;
  }

  if (age <= 0 || age > 120)
  {
    qCritical() << "[DataBaseManager::registerPatient] Invalid age";
    setLastError("Edad invalida, debe estar entre 0 y 120 años");
    return false;
  }

  if (weight <= 0 || weight > 200)
  {
    qCritical() << "[DataBaseManager::registerPatient] Invalid weight";
    setLastError("Peso invalido, debe estar entre 0 y 200 kg");
    return false;
  }

  if (height <= 0 || height > 250)
  {
    qCritical() << "[DataBaseManager::registerPatient] Invalid height";
    setLastError("Altura invalida, debe estar entre 0 y 250 cm");
    return false;
  }

  const auto result = patient_repository_.insertPatientForUserName(name, last_name, age, weight, height, user_name_, description);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return false;
  }

  setLastError("");
  qDebug() << "[DataBaseManager::registerPatient] Patient registered and linked successfully for user:" << user_name_;
  return true;
}

bool DataBaseManager::deletePatient(int patient_id)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::deletePatient] Database is not open";
    setLastError("La base de datos no esta abierta");
    return false;
  }

  if (user_role_ == "guest" || user_name_.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::deletePatient] Action not allowed for guest users";
    setLastError("Accion no permitida para usuario invitado");
    return false;
  }

  if (patient_id <= 0)
  {
    qCritical() << "[DataBaseManager::deletePatient] Invalid patient ID";
    setLastError("ID de paciente invalido");
    return false;
  }

  const auto result = patient_repository_.deletePatientByIdForUserName(patient_id, user_name_);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return false;
  }

  setLastError("");
  qDebug() << "[DataBaseManager::deletePatient] Patient unlinked successfully for user:" << user_name_;
  return true;
}

bool DataBaseManager::deleteExperiment(int experiment_id)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::deleteExperiment] Database is not open";
    setLastError("La base de datos no esta abierta");
    return false;
  }

  if (user_role_ == "guest" || user_name_.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::deleteExperiment] Action not allowed for guest users";
    setLastError("Accion no permitida para usuario invitado");
    return false;
  }

  if (experiment_id <= 0)
  {
    qCritical() << "[DataBaseManager::deleteExperiment] Invalid experiment ID";
    setLastError("ID de experimento invalido");
    return false;
  }

  const auto result = experiment_repository_.deleteExperimentByIdForUserName(experiment_id, user_name_);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return false;
  }

  setLastError("");
  setLastExperimentId(-1);
  qDebug() << "[DataBaseManager::deleteExperiment] Experiment deleted successfully:" << experiment_id;
  return true;
}

bool DataBaseManager::saveExperiment(const QString& patient_name, const QString& patient_last_name, const QString& map_name, const QString& annotations)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::saveExperiment] Database is not open";
    setLastError("La base de datos no esta abierta");
    return false;
  }

  if (user_role_ == "guest" || user_name_.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::saveExperiment] Action not allowed for guest users";
    setLastError("Accion no permitida para usuario invitado");
    return false;
  }

  if (patient_name.trimmed().isEmpty() || patient_last_name.trimmed().isEmpty() || map_name.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::saveExperiment] Patient name, last name or map name is empty";
    setLastError("Nombre del paciente, apellidos y mapa son obligatorios");
    return false;
  }

  const QString trimmed_annotations = annotations.trimmed();
  const std::optional<QString> comment = trimmed_annotations.isEmpty() ? std::nullopt : std::optional<QString>(trimmed_annotations);

  const auto result = experiment_repository_.insertExperiment(patient_name.trimmed(), patient_last_name.trimmed(), map_name.trimmed(), user_name_, comment);
  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return false;
  }

  const int experiment_id = std::get<int>(result);

  setLastError("");
  setLastExperimentId(experiment_id);
  qDebug() << "[DataBaseManager::saveExperiment] Experiment saved successfully:" << experiment_id;
  return true;
}

QVariantList DataBaseManager::listMaps()
{
  QVariantList maps_list;

  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::listMaps] Database is not open";
    setLastError("La base de datos no esta abierta");
    return maps_list;
  }

  const auto result = map_repository_.listMapNames();

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return maps_list;
  }

  const auto maps = std::get<QVector<QString>>(result);

  for (const auto& map_name : maps)
  {
    QVariantMap map;
    map["map_name"] = map_name;
    map["display"] = map_name;
    maps_list.append(map);
  }

  setLastError("");
  qDebug() << "[DataBaseManager::listMaps] Listed" << maps.size() << "maps";
  return maps_list;
}

QVariantMap DataBaseManager::getMapDetails(const QString& map_name)
{
  QVariantMap map;

  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::getMapDetails] Database is not open";
    setLastError("La base de datos no esta abierta");
    return map;
  }

  if (map_name.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::getMapDetails] Invalid map name";
    setLastError("Nombre de mapa invalido");
    return map;
  }

  const auto result = map_repository_.getMapDetailsByName(map_name);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return map;
  }

  setLastError("");
  qDebug() << "[DataBaseManager::getMapDetails] Retrieved details for map:" << map_name;
  return toVariantMap(std::get<MapDetails>(result));
}

bool DataBaseManager::mapExists(const QString& map_name)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::mapExists] Database is not open";
    setLastError("La base de datos no esta abierta");
    return false;
  }

  const QString trimmed = map_name.trimmed();
  if (trimmed.isEmpty())
  {
    qCritical() << "[DataBaseManager::mapExists] Invalid map name";
    setLastError("Nombre de mapa invalido");
    return false;
  }

  const auto result = map_repository_.getMapDetailsByName(trimmed);

  if (statusOk(result))
  {
    setLastError("");
    return true;
  }

  const auto error = std::get<DbError>(result);
  if (error.code == DbErrorCode::NOT_FOUND)
  {
    setLastError("");
    return false;
  }

  setLastError(error.message);
  return false;
}

bool DataBaseManager::registerMap(const QString& map_name, const QString& location, const QString& details)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::registerMap] Database is not open";
    setLastError("La base de datos no esta abierta");
    return false;
  }

  if (user_role_ == "guest" || user_name_.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::registerMap] Action not allowed for guest users";
    setLastError("Accion no permitida para usuario invitado");
    return false;
  }

  if (map_name.trimmed().isEmpty() || location.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::registerMap] Map name or location is empty";
    setLastError("Nombre y localizacion del mapa son obligatorios");
    return false;
  }

  const auto result = map_repository_.insertMapForUserName(user_name_, map_name, location, details);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return false;
  }

  setLastError("");
  qDebug() << "[DataBaseManager::registerMap] Map registered successfully:" << map_name;
  return true;
}

bool DataBaseManager::deleteMap(const QString& map_name)
{
  if (!db_.isOpen())
  {
    qCritical() << "[DataBaseManager::deleteMap] Database is not open";
    setLastError("La base de datos no esta abierta");
    return false;
  }

  if (map_name.trimmed().isEmpty())
  {
    qCritical() << "[DataBaseManager::deleteMap] Map name is empty";
    setLastError("Nombre de mapa invalido");
    return false;
  }

  const auto result = map_repository_.deleteMapByName(map_name);

  if (!statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    setLastError(error.message);
    return false;
  }

  setLastError("");
  qDebug() << "[DataBaseManager::deleteMap] Map deleted successfully:" << map_name;
  return true;
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

void DataBaseManager::setLastExperimentId(int experiment_id)
{
  if (last_experiment_id_ == experiment_id)
  {
    return;
  }

  last_experiment_id_ = experiment_id;
  emit lastExperimentIdChanged();
}
