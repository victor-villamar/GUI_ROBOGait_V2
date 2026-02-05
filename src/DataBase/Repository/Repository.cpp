#include <QSqlDatabase>
#include <QSqlError>

#include "DataBase/Repository/Repository.hpp"

using namespace ROBOGait::db;

Repository::Repository(RoboGaitDb& db) : db_(&db) {}

Repository::~Repository() = default;

QSqlDatabase& Repository::getDataBase() const { return db_->getDataBase(); }

DbResultVoid Repository::prepareQuery(QSqlQuery& query, const QString& sql) const
{
  if (!query.prepare(sql))
  {
    const auto error = query.lastError();
    return makeFailure(DbErrorCode::QUERY_PREPARE_FAILED, "Failed to prepare SQL query", sql, error.driverText(), error.databaseText());
  }
  return makeSuccess();
}

DbResultVoid Repository::executeQuery(QSqlQuery& query, DbErrorCode error_code) const
{
  if (!query.exec())
  {
    const auto error = query.lastError();
    return makeFailure(error_code, "Failed to execute SQL query", query.lastQuery(), error.driverText(), error.databaseText());
  }
  return makeSuccess();
}
