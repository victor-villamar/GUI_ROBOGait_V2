#pragma once

#include <QSqlDatabase>
#include <QString>

#include "DataBase/Common/DbErrors.hpp"

namespace ROBOGait
{
namespace db
{

/**
 * @brief Manages database schema creation and versioning
 *
 * This class is responsible for creating the database schema
 * and ensuring it is up to date.
 */
class DatabaseSchema
{
public:
  /**
   * @brief Initialize database schema (create all tables and default data)
   *
   * @param db Reference to the database
   * @return DbResultVoid indicating success or failure
   */
  static DbResultVoid initializeSchema(const QSqlDatabase& db);

private:
  /**
   * @brief Create user table
   *
   * @param db Reference to the database
   * @return DbResultVoid indicating success or failure
   */
  static DbResultVoid createUserTable(const QSqlDatabase& db);

  /**
   * @brief Create patient table
   *
   * @param db Reference to the database
   * @return DbResultVoid indicating success or failure
   */
  static DbResultVoid createPatientTable(const QSqlDatabase& db);

  /**
   * @brief Create map table
   *
   * @param db Reference to the database
   * @return DbResultVoid indicating success or failure
   */
  static DbResultVoid createMapTable(const QSqlDatabase& db);

  /**
   * @brief Create experiments table
   *
   * @param db Reference to the database
   * @return DbResultVoid indicating success or failure
   */
  static DbResultVoid createExperimentsTable(const QSqlDatabase& db);

  /**
   * @brief Create patient_doctor junction table
   *
   * @param db Reference to the database
   * @return DbResultVoid indicating success or failure
   */
  static DbResultVoid createPatientDoctorTable(const QSqlDatabase& db);

  /**
   * @brief Create indexes for optimization
   *
   * @param db Reference to the database
   * @return DbResultVoid indicating success or failure
   */
  static DbResultVoid createIndexes(const QSqlDatabase& db);

  /**
   * @brief Insert default users (doctor and manager)
   *
   * @param db Reference to the database
   * @return DbResultVoid indicating success or failure
   */
  static DbResultVoid insertDefaultUsers(const QSqlDatabase& db);

  /**
   * @brief Execute a SQL statement
   *
   * @param db Reference to the database
   * @param sql The SQL statement to execute
   * @param description Description of what is being executed (for logging)
   * @return DbResultVoid indicating success or failure
   */
  static DbResultVoid executeSql(const QSqlDatabase& db, const QString& sql, const QString& description);

  static constexpr int CURRENT_SCHEMA_VERSION = 1; /**< Current schema version */
};

} // namespace db
} // namespace ROBOGait
