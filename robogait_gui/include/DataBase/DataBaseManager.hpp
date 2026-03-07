#pragma once

#include <QObject>
#include <QString>

#include "DataBase/Repository/ExperimentRepository.hpp"
#include "DataBase/Repository/MapRepository.hpp"
#include "DataBase/Repository/PatientRepository.hpp"
#include "DataBase/Repository/UserRepository.hpp"
#include "RoboGaitDb.hpp"

namespace ROBOGait
{
namespace db
{
/**
 * @brief Database manager for user authentication and repository access
 *
 * Singleton pattern ensures only one database connection throughout the application
 */
class DataBaseManager : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Get the singleton instance of DataBaseManager
   *
   * @return Reference to the singleton instance
   */
  static DataBaseManager& getInstance();

  // Delete copy constructor and assignment operator
  DataBaseManager(const DataBaseManager&) = delete;
  DataBaseManager& operator=(const DataBaseManager&) = delete;
  DataBaseManager(DataBaseManager&&) = delete;
  DataBaseManager& operator=(DataBaseManager&&) = delete;

  /**
   * @brief Initialize the database with the given path
   *
   * @param db_path Path to the database file
   * @return True if initialization successful, false otherwise
   */
  bool initialize(const QString& db_path);

  /**
   * @brief Check if the database has been initialized
   *
   * @return True if initialized, false otherwise
   */
  bool isInitialized() const;

  // clang-format off
  Q_PROPERTY(bool passLogin
             READ getPassLogin
             NOTIFY passLoginChanged)

  Q_PROPERTY(bool passCheckUserName
             READ getPassCheckUserName
             NOTIFY passCheckUserNameChanged)

  Q_PROPERTY(QString userName
             READ getUserName
             NOTIFY userNameChanged)
             
  Q_PROPERTY(QString displayName
             READ getDisplayName
             NOTIFY displayNameChanged)
  
  Q_PROPERTY(QString userRole
             READ getUserRole
             NOTIFY userRoleChanged)
    
  Q_PROPERTY(QString lastError
             READ getLastError
             NOTIFY lastErrorChanged)
  // clang-format on

  /**
   * @brief Get the login status
   *
   * @return True if the user is logged in, false otherwise
   */
  bool getPassLogin() const;

  /**
   * @brief Get the status of the user name availability check
   *
   * @return True if the user name is available, false otherwise
   */
  bool getPassCheckUserName() const;

  /**
   * @brief Get the user name
   *
   * @return The user name
   */
  QString getUserName() const;

  /**
   * @brief Get the user ID
   *
   * @return The user ID, -1 if not authenticated
   */
  int getUserId() const;

  /**
   * @brief Get the display name
   *
   * @return The display name
   */
  QString getDisplayName() const;

  /**
   * @brief Get the user role
   *
   * @return The user role
   */
  QString getUserRole() const;

  /**
   * @brief Get the last error message
   *
   * @return The last error message
   */
  QString getLastError() const;

  /**
   * @brief Open the database connection
   *
   * @param path The path to the database file
   * @return True if the database was opened successfully, false otherwise
   */
  Q_INVOKABLE bool openDatabase(const QString& path);

  /**
   * @brief Log in a user
   *
   * @param user_name The user name
   * @param password The password
   * @return True if the login was successful, false otherwise
   */
  Q_INVOKABLE bool login(const QString& user_name, const QString& password);

  /**
   * @brief Check if a user name is available
   *
   * @param user_name The user name to check
   * @return True if the user name is available, false otherwise
   */
  Q_INVOKABLE bool checkUserNameAvailable(const QString& user_name);

  /**
   * @brief Register a new user
   *
   * @param name The name of the user
   * @param last_name The last name of the user
   * @param user_name The user name
   * @param password The password
   * @param role The role of the user
   * @return True if the registration was successful, false otherwise
   */
  Q_INVOKABLE bool registerUser(const QString& name, const QString& last_name, const QString& user_name, const QString& password, const QString& role);

  /**
   * @brief Log in as a guest
   *
   * @param name The name of the guest
   */
  Q_INVOKABLE void loginGuest(const QString& name);

  /**
   * @brief Log out the current user
   */
  Q_INVOKABLE void logout();

  /**
   * @brief List all patients for the current user
   *
   * @return A list of patients
   */
  Q_INVOKABLE QVariantList listPatients();

  /**
   * @brief Get detailed information about a specific patient
   *
   * @param patient_id The ID of the patient
   * @return A map containing patient details
   */
  Q_INVOKABLE QVariantMap getPatientDetails(int patient_id);

  /**
   * @brief Get basic patient information by ID
   *
   * @param patient_id The ID of the patient
   *
   * @return A map containing basic patient info (id, name, last_name, display) or empty if not found
   */
  Q_INVOKABLE QVariantMap getPatientBasicInfo(int patient_id);

  /**
   * @brief Get doctor diagnostics for a patient (requires current user to be linked)
   *
   * @param patient_id The ID of the patient
   * @return A list of doctor diagnostics (doctor_id, doctor_name, doctor_last_name, display, create_day, description)
   */
  Q_INVOKABLE QVariantList getPatientDoctorDiagnostics(int patient_id);

  /**
   * @brief Register a new patient
   *
   * @param name The name of the patient
   * @param last_name The last name of the patient
   * @param age The age of the patient
   * @param weight The weight of the patient
   * @param height The height of the patient
   * @param description A description of the patient's condition
   * @return True if the registration was successful, false otherwise
   */
  Q_INVOKABLE bool registerPatient(const QString& name, const QString& last_name, int age, double weight, double height, const QString& description);

  /**
   * @brief Remove the current user's association with a patient (and delete if orphaned)
   *
   * @param patient_id The ID of the patient to delete
   * @return True if the deletion was successful, false otherwise
   */
  Q_INVOKABLE bool deletePatient(int patient_id);

  /**
   * @brief List all maps
   *
   * @return A list of maps
   */
  Q_INVOKABLE QVariantList listMaps();

  /**
   * @brief Get map details by name
   *
   * @param map_name The map name
   * @return A map containing map details
   */
  Q_INVOKABLE QVariantMap getMapDetails(const QString& map_name);

  /**
   * @brief Register a new map
   *
   * @param map_name The map name
   * @param location The map location
   * @param details Additional details/description
   * @return True if the registration was successful, false otherwise
   */
  Q_INVOKABLE bool registerMap(const QString& map_name, const QString& location, const QString& details);

  /**
   * @brief Delete a map by name
   *
   * @param map_name The map name
   * @return True if the deletion was successful, false otherwise
   */
  Q_INVOKABLE bool deleteMap(const QString& map_name);

signals:
  void passLoginChanged();
  void passCheckUserNameChanged();
  void userNameChanged();
  void displayNameChanged();
  void userRoleChanged();
  void lastErrorChanged();
  void userLoggedOut(); // Signal for explicit logout events

private:
  /**
   * @brief Hash a password using SHA-256
   *
   * @param password The password to hash
   * @return The hashed password
   */
  QString hashPasswordSha256Hex(const QString& password) const;

  /**
   * @brief Set the login status
   *
   * @param pass_login The login status
   */
  void setPassLogin(bool pass_login);

  /**
   * @brief Set the status of the user name availability check
   *
   * @param pass_check_user_name The status of the user name availability check
   */
  void setPassCheckUserName(bool pass_check_user_name);

  /**
   * @brief Set the user name
   *
   * @param user_name The user name
   */
  void setUserName(const QString& user_name);

  /**
   * @brief Set the display name
   *
   * @param display_name The display name
   */
  void setDisplayName(const QString& display_name);

  /**
   * @brief Set the user role
   *
   * @param user_role The user role
   */
  void setUserRole(const QString& user_role);

  /**
   * @brief Set the last error message
   *
   * @param last_error The last error message
   */
  void setLastError(const QString& last_error);

private:
  /**
   * @brief Private constructor for Singleton pattern
   */
  DataBaseManager();

  /**
   * @brief Destructor for the DataBaseManager class
   */
  ~DataBaseManager();

  bool is_initialized_; /**< Initialization status */

  RoboGaitDb db_;                              /**< Database connection */
  UserRepository user_repository_;             /**< User repository */
  PatientRepository patient_repository_;       /**< Patient repository */
  MapRepository map_repository_;               /**< Map repository */
  ExperimentRepository experiment_repository_; /**< Experiment repository */

  bool pass_login_;           /**< Login status */
  bool pass_check_user_name_; /**< User name availability status */
  int user_id_;               /**< Current user ID, -1 if not authenticated */
  QString user_name_;         /**< User name */
  QString display_name_;      /**< Display name */
  QString user_role_;         /**< User role */
  QString last_error_;        /**< Last error message */
};

} // namespace db
} // namespace ROBOGait
