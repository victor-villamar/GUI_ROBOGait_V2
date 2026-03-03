#pragma once

#include <QString>
#include <variant>

namespace ROBOGait
{
namespace db
{
/**
 * @brief Enumeration of database error codes
 */
enum class DbErrorCode
{
  OK = 0,
  DRIVER_NOT_AVAILABLE,
  OPEN_FAILED,
  QUERY_PREPARE_FAILED,
  QUERY_EXECUTION_FAILED,
  TRANSACTION_FAILED,
  NOT_FOUND,
  INVALID_INPUT,
  INVALID_CREDENTIALS,
  USERNAME_ALREADY_EXISTS,
  CONSTRAINT_VIOLATION,
  FOREIGN_KEY_VIOLATION,
  UNEXPECTED_RESULT,
  UNKNOWN
};

/**
 * @brief Structure representing a database error
 *
 * @param code The error code
 * @param message The error message
 * @param sql The SQL query that caused the error
 * @param driver_text The driver-specific error message
 * @param database_text The database-specific error message
 */
struct DbError
{
  DbErrorCode code = DbErrorCode::UNKNOWN;
  QString message;
  QString sql;
  QString driver_text;
  QString database_text;
};

/**
 * @brief Type alias for a database result that does not return a value.
 */
using DbResultVoid = std::variant<std::monostate, DbError>;

/**
 * @brief Type alias for a database result that returns a value.
 */
template <typename T> using DbResult = std::variant<T, DbError>;

/**
 * @brief Check if a database result is successful.
 *
 * @param result The database result to check.
 * @return True if the result is successful, false otherwise.
 */
bool statusOk(const DbResultVoid& result);

/**
 * @brief Create a successful database result.
 *
 * @return A successful database result.
 */
DbResultVoid makeSuccess();

/**
 * @brief Create a failed database result.
 *
 * @param code The error code.
 * @param message The error message.
 * @param sql The SQL query that caused the error.
 * @param driver_text The driver-specific error message.
 * @param database_text The database-specific error message.
 * @return A failed database result.
 */
DbResultVoid makeFailure(DbErrorCode code, const QString& message, const QString& sql = {}, const QString& driver_text = {}, const QString& database_text = {});

/**
 * @brief Check if a database result is successful.
 *
 * @param result The database result to check.
 * @return True if the result is successful, false otherwise.
 */
template <typename T> inline bool statusOk(const DbResult<T>& result) { return std::holds_alternative<T>(result); }

/**
 * @brief Create a failed database result.
 *
 * @param code The error code.
 * @param message The error message.
 * @param sql The SQL query that caused the error.
 * @param driver_text The driver-specific error message.
 * @param database_text The database-specific error message.
 * @return A failed database result.
 */
template <typename T>
inline DbResult<T> makeFailureT(DbErrorCode code, const QString& message, const QString& sql = {}, const QString& driver_text = {},
                                const QString& database_text = {})
{
  return DbResult<T>(DbError{code, message, sql, driver_text, database_text});
}

} // namespace db
} // namespace ROBOGait
