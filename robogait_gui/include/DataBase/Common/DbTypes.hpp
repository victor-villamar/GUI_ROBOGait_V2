#pragma once

#include <optional>

#include <QString>
#include <QVariantMap>

namespace ROBOGait
{
namespace db
{
namespace tables
{
static constexpr const char* USER = "user";
static constexpr const char* PATIENT = "patient";
static constexpr const char* MAP = "map";
static constexpr const char* EXPERIMENTS = "experiments";
} // namespace tables

/**
 * @brief Enumeration for user roles
 */
enum class UserRole
{
  DOCTOR,
  MANAGER,
  GUEST,
  UNKNOWN
};

/**
 * @brief Structure representing a user row in the database
 *
 * @param id The unique identifier for the user
 * @param name The name of the user
 * @param last_name The last name of the user
 * @param user_name The username of the user
 * @param password_hash The password hash of the user
 * @param role The role of the user
 */
struct UserRow
{
  int id = -1;
  QString name;
  QString last_name;
  QString user_name;
  QString password_hash;
  UserRole role = UserRole::UNKNOWN;
};

/**
 * @brief Structure representing a patient row in the database
 *
 * @param id The unique identifier for the patient
 * @param name The name of the patient
 * @param last_name The last name of the patient
 * @param age The age of the patient
 * @param weight The weight of the patient
 * @param height The height of the patient
 * @param description A description of the patient's condition
 * @param create_day The creation date of the patient record
 */
struct PatientRow
{
  int id = -1;
  QString name;
  QString last_name;
  int age = 0;
  double weight = 0.0;
  double height = 0.0;
  QString description;
  QString create_day;
};

/**
 * @brief Structure representing a map row in the database
 *
 * @param id The unique identifier for the map
 * @param name The name of the map
 * @param location The location of the map
 * @param details The details of the map
 * @param create_day The creation date of the map
 * @param id_user The unique identifier for the user who created the map
 */
struct MapRow
{
  int id = -1;
  QString name;
  QString location;
  QString details;
  QString create_day;
  int id_user = -1;
  bool map_save = false;
};

/**
 * @brief Structure representing an experiment row in the database
 *
 * @param id The unique identifier for the experiment
 * @param id_patient The unique identifier for the patient associated with the experiment
 * @param id_map The unique identifier for the map associated with the experiment
 * @param date The date when the experiment was conducted
 * @param comment Optional annotations for the experiment
 * @param id_user The unique identifier for the user who conducted the experiment
 */
struct ExperimentRow
{
  int id = -1;
  int id_patient = -1;
  int id_map = -1;
  QString date;
  QString comment;
  std::optional<int> id_user;
};

/**
 * @brief Structure representing patient details, including user information
 *
 * @param patient The patient information
 * @param user_name The username of the requesting user associated with the patient
 * @param user_last_name The last name of the requesting user associated with the patient
 * @param doctor_names Comma-separated list of doctors assigned to the patient
 */
struct PatientDetails
{
  PatientRow patient;
  QString user_name;
  QString user_last_name;
  QString doctor_names;
};

/**
 * @brief Structure representing a doctor associated with a patient
 *
 * @param doctor_id The unique identifier for the doctor
 * @param doctor_name The first name of the doctor
 * @param doctor_last_name The last name of the doctor
 * @param create_day The date when the relation was created
 * @param description The diagnosis/description for the patient by this doctor
 */
struct PatientDoctorInfo
{
  int doctor_id = -1;
  QString doctor_name;
  QString doctor_last_name;
  QString create_day;
  QString description;
};

/**
 * @brief Structure representing a map details, including user information
 *
 * @param map The map information
 * @param user_name The username of the user associated with the map
 * @param user_last_name The last name of the user associated with the map
 */
struct MapDetails
{
  MapRow map;
  QString user_name;
  QString user_last_name;
};

/**
 * @brief Convert a UserRole enum to its corresponding database string representation
 *
 * @param role The UserRole enum value
 * @return The corresponding string representation for the database
 */
QString userRoleToDbString(UserRole role);

/**
 * @brief Convert a database string representation to a UserRole enum
 *
 * @param role The string representation of the UserRole
 * @return The corresponding UserRole enum value
 */
UserRole userRoleFromDbString(const QString& role);

/**
 * @brief Convert a PatientDetails struct to a QVariantMap
 *
 * @param patient_details The patient information
 * @return The corresponding QVariantMap representation
 */
QVariantMap toVariantMap(const PatientDetails& patient_details);

/**
 * @brief Convert a MapDetails struct to a QVariantMap
 *
 * @param map_details The map information
 * @return The corresponding QVariantMap representation
 */
QVariantMap toVariantMap(const MapDetails& map_details);

} // namespace db
} // namespace ROBOGait
