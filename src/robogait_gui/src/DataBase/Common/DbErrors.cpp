#include "DataBase/Common/DbErrors.hpp"

namespace ROBOGait
{
namespace db
{

bool statusOk(const DbResultVoid& result) { return std::holds_alternative<std::monostate>(result); }

DbResultVoid makeSuccess() { return std::monostate{}; }

DbResultVoid makeFailure(DbErrorCode code, const QString& message, const QString& sql, const QString& driver_text, const QString& database_text)
{
  return DbError{code, message, sql, driver_text, database_text};
}
} // namespace db
} // namespace ROBOGait
