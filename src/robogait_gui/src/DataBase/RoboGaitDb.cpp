#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "DataBase/DatabaseSchema.hpp"
#include "DataBase/RoboGaitDb.hpp"

using namespace ROBOGait::db;

RoboGaitDb::RoboGaitDb() : connection_name_(QString("robogait_%1").arg(reinterpret_cast<quintptr>(this))) {}

DbResultVoid RoboGaitDb::open(const QString& db_path)
{
  if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
  {
    return makeFailure(DbErrorCode::DRIVER_NOT_AVAILABLE, "SQLite driver is not available");
  }

  if (db_.isValid() && db_.isOpen())
  {
    return makeSuccess();
  }

  db_ = QSqlDatabase::addDatabase("QSQLITE", connection_name_);
  db_.setDatabaseName(db_path);

  if (!db_.open())
  {
    const auto err = db_.lastError();
    return makeFailure(DbErrorCode::OPEN_FAILED, "Failed to open SQLite database", QString(), err.driverText(), err.databaseText());
  }

  {
    QSqlQuery pragma_query(db_);
    if (!pragma_query.exec("PRAGMA foreign_keys = ON"))
    {
      const auto err = pragma_query.lastError();
      return makeFailure(DbErrorCode::QUERY_EXECUTION_FAILED, "Failed to enable foreign key support", pragma_query.lastQuery(), err.driverText(),
                         err.databaseText());
    }
  }

  qDebug() << "[RoboGaitDb::open] Initializing/verifying database schema...";

  if (auto result = DatabaseSchema::initializeSchema(db_); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    qCritical() << "[RoboGaitDb::open] Failed to initialize database schema, error:" << error.message;
    return result;
  }

  qDebug() << "[RoboGaitDb::open] Database schema verified successfully";

  return makeSuccess();
}

void RoboGaitDb::close()
{
  if (!db_.isValid())
  {
    qCritical() << "[RoboGaitDb::close] Database is not valid";
    return;
  }

  const auto name = connection_name_;
  db_.close();
  db_ = QSqlDatabase();

  QSqlDatabase::removeDatabase(name);
  qDebug() << "[RoboGaitDb::close] Database connection closed and removed:" << name;
}

bool RoboGaitDb::isOpen() const { return db_.isOpen() && db_.isValid(); }

QSqlDatabase& RoboGaitDb::getDataBase() { return db_; }

const QSqlDatabase& RoboGaitDb::getDataBase() const { return db_; }
