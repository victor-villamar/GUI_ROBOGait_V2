#pragma once

#include <QSqlQuery>
#include <QString>

#include "DataBase/Common/DbErrors.hpp"
#include "DataBase/RoboGaitDb.hpp"

namespace ROBOGait
{
namespace db
{
/**
 * @brief Base class for database repositories
 */
class Repository
{
public:
  /**
   * @brief Construct a new Repository object
   *
   * @param db Reference to the RoboGaitDb instance
   */
  Repository(RoboGaitDb& db);

  /**
   * @brief Destroy the Repository object
   */
  virtual ~Repository();

protected:
  /**
   * @brief Get a reference to the database connection
   *
   * @return Reference to the database connection
   */
  QSqlDatabase& getDataBase() const;

  /**
   * @brief Prepare a SQL query
   *
   * @param query Reference to the QSqlQuery object
   * @param sql SQL query string
   * @return Result indicating success or failure of the query preparation
   */
  DbResultVoid prepareQuery(QSqlQuery& query, const QString& sql) const;

  /**
   * @brief Execute a SQL query
   *
   * @param query Reference to the QSqlQuery object
   * @param error_code Error code to use if execution fails
   * @return Result indicating success or failure of the query execution
   */
  DbResultVoid executeQuery(QSqlQuery& query, DbErrorCode error_code = DbErrorCode::QUERY_EXECUTION_FAILED) const;

private:
  RoboGaitDb* db_; /**< Reference to the database connection */
};
} // namespace db
} // namespace ROBOGait
