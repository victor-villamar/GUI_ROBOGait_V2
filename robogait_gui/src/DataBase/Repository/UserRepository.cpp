#include <QSqlError>
#include <QVariant>

#include "DataBase/Common/DbTypes.hpp"
#include "DataBase/Repository/UserRepository.hpp"

using namespace ROBOGait::db;

UserRepository::UserRepository(RoboGaitDb& db) : Repository(db) {}

UserRepository::~UserRepository() = default;

DbResult<std::optional<UserRow>> UserRepository::findByCredentials(const QString& user_name, const QString& password_hash) const
{
  QSqlQuery query(getDataBase());

  // clang-format off
  const QString sql = 
        "SELECT id, name, lastname, username, password, role "
        "FROM \"user\" "
        "WHERE username = :username AND password = :password";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<std::optional<UserRow>>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  query.bindValue(":username", user_name);
  query.bindValue(":password", password_hash);

  if (auto result = executeQuery(query); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<std::optional<UserRow>>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  if (!query.next())
  {
    return std::optional<UserRow>{};
  }

  UserRow user;
  user.id = query.value(0).toInt();
  user.name = query.value(1).toString();
  user.last_name = query.value(2).toString();
  user.user_name = query.value(3).toString();
  user.password_hash = query.value(4).toString();
  user.role = userRoleFromDbString(query.value(5).toString());

  return std::optional<UserRow>{user};
}

DbResult<bool> UserRepository::userNameExists(const QString& user_name) const
{
  QSqlQuery query(getDataBase());

  // clang-format off
  const QString sql = 
        "SELECT COUNT(*) "
        "FROM \"user\" "
        "WHERE username = :username";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<bool>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  query.bindValue(":username", user_name);

  if (auto result = executeQuery(query); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<bool>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  if (!query.next())
  {
    return makeFailureT<bool>(DbErrorCode::UNEXPECTED_RESULT, "COUNT query returned no rows", sql);
  }

  const int count = query.value(0).toInt();

  return count > 0;
}

DbResult<int> UserRepository::getUserIdByUserName(const QString& user_name) const
{
  QSqlQuery query(getDataBase());

  // clang-format off
  const QString sql = 
        "SELECT id "
        "FROM \"user\" "
        "WHERE username = :username";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<int>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  query.bindValue(":username", user_name);

  if (auto result = executeQuery(query); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<int>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  if (!query.next())
  {
    return makeFailureT<int>(DbErrorCode::NOT_FOUND, "User not found", sql);
  }

  return query.value(0).toInt();
}

DbResultVoid UserRepository::insertUser(const QString& name, const QString& last_name, const QString& user_name, const QString& password_hash,
                                        UserRole role) const
{
  QSqlQuery query(getDataBase());

  // clang-format off
  const QString sql = 
        "INSERT INTO \"user\" (name, lastname, username, password, role) "
        "VALUES (:name, :lastname, :username, :password, :role)";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    return result;
  }

  query.bindValue(":name", name);
  query.bindValue(":lastname", last_name);
  query.bindValue(":username", user_name);
  query.bindValue(":password", password_hash);
  query.bindValue(":role", userRoleToDbString(role));

  return executeQuery(query);
}
