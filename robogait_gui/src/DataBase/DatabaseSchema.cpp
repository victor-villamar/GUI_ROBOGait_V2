#include <QCryptographicHash>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "DataBase/DatabaseSchema.hpp"

using namespace ROBOGait::db;

DbResultVoid DatabaseSchema::initializeSchema(const QSqlDatabase& db)
{
  if (auto result = createUserTable(db); !statusOk(result))
  {
    return result;
  }

  if (auto result = createPatientTable(db); !statusOk(result))
  {
    return result;
  }

  if (auto result = createMapTable(db); !statusOk(result))
  {
    return result;
  }

  if (auto result = createExperimentsTable(db); !statusOk(result))
  {
    return result;
  }

  if (auto result = createPatientDoctorTable(db); !statusOk(result))
  {
    return result;
  }

  if (auto result = createIndexes(db); !statusOk(result))
  {
    return result;
  }

  if (auto result = insertDefaultUsers(db); !statusOk(result))
  {
    return result;
  }

  return makeSuccess();
}

DbResultVoid DatabaseSchema::createUserTable(const QSqlDatabase& db)
{
  const QString sql = R"(
    CREATE TABLE IF NOT EXISTS "user" (
      id INTEGER PRIMARY KEY NOT NULL UNIQUE,
      name TEXT NOT NULL,
      lastname TEXT NOT NULL,
      username TEXT NOT NULL,
      password TEXT NOT NULL,
      role TEXT NOT NULL CHECK(role IN ('doctor', 'manager'))
    )
  )";

  return executeSql(db, sql, "user table");
}

DbResultVoid DatabaseSchema::createPatientTable(const QSqlDatabase& db)
{
  const QString sql = R"(
    CREATE TABLE IF NOT EXISTS patient (
      id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,
      name TEXT NOT NULL,
      lastname TEXT NOT NULL,
      age INTEGER NOT NULL,
      weight NUMERIC NOT NULL,
      height REAL NOT NULL
    )
  )";

  return executeSql(db, sql, "patient table");
}

DbResultVoid DatabaseSchema::createMapTable(const QSqlDatabase& db)
{
  const QString sql = R"(
    CREATE TABLE IF NOT EXISTS map (
      id INTEGER PRIMARY KEY NOT NULL,
      name TEXT NOT NULL,
      location TEXT NOT NULL,
      details TEXT,
      create_day TEXT NOT NULL,
      id_user INTEGER NOT NULL,
      map_save INTEGER DEFAULT 0,
      FOREIGN KEY (id_user) REFERENCES "user"(id)
    )
  )";

  return executeSql(db, sql, "map table");
}

DbResultVoid DatabaseSchema::createExperimentsTable(const QSqlDatabase& db)
{
  const QString sql = R"(
    CREATE TABLE IF NOT EXISTS experiments (
      id INTEGER PRIMARY KEY NOT NULL UNIQUE,
      id_patient INTEGER NOT NULL,
      id_map INTEGER NOT NULL,
      date TEXT NOT NULL,
      comment TEXT,
      id_user INTEGER,
      FOREIGN KEY (id_patient) REFERENCES patient(id),
      FOREIGN KEY (id_map) REFERENCES map(id),
      FOREIGN KEY (id_user) REFERENCES "user"(id)
    )
  )";

  return executeSql(db, sql, "experiments table");
}

DbResultVoid DatabaseSchema::createPatientDoctorTable(const QSqlDatabase& db)
{
  const QString sql = R"(
    CREATE TABLE IF NOT EXISTS patient_doctor (
      id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,
      id_patient INTEGER NOT NULL,
      id_doctor INTEGER NOT NULL,
      create_day TEXT NOT NULL,
      description TEXT DEFAULT '',
      UNIQUE(id_patient, id_doctor),
      FOREIGN KEY (id_patient) REFERENCES patient(id) ON DELETE CASCADE,
      FOREIGN KEY (id_doctor) REFERENCES "user"(id) ON DELETE CASCADE
    )
  )";

  return executeSql(db, sql, "patient_doctor table");
}

DbResultVoid DatabaseSchema::createIndexes(const QSqlDatabase& db)
{
  {
    const QString sql = R"(
      CREATE INDEX IF NOT EXISTS idx_patient_doctor_doctor 
      ON patient_doctor(id_doctor)
    )";

    if (auto result = executeSql(db, sql, "index on id_doctor"); !statusOk(result))
    {
      return result;
    }
  }

  {
    const QString sql = R"(
      CREATE INDEX IF NOT EXISTS idx_patient_doctor_patient 
      ON patient_doctor(id_patient)
    )";

    if (auto result = executeSql(db, sql, "index on id_patient"); !statusOk(result))
    {
      return result;
    }
  }

  return makeSuccess();
}

DbResultVoid DatabaseSchema::insertDefaultUsers(const QSqlDatabase& db)
{
  const QString password_plain = "a";
  const QString password_hash = QString(QCryptographicHash::hash(password_plain.toUtf8(), QCryptographicHash::Sha256).toHex());

  // User doctor: username=a, password=a, name=a, lastname=a
  {
    QSqlQuery query(db);
    const QString sql = R"(
      INSERT OR IGNORE INTO "user" (id, name, lastname, username, password, role)
      VALUES (1, 'a', 'a', 'a', :password, 'doctor')
    )";

    query.prepare(sql);
    query.bindValue(":password", password_hash);

    if (!query.exec())
    {
      const auto err = query.lastError();
      qCritical() << "[DatabaseSchema::insertDefaultUsers] Failed to insert doctor user";
      qCritical() << "  Error:" << err.text();

      return makeFailure(DbErrorCode::QUERY_EXECUTION_FAILED, "Failed to insert default doctor user", sql, err.driverText(), err.databaseText());
    }
  }

  // User manager: username=admin, password=a, name=admin, lastname=admin
  {
    QSqlQuery query(db);
    const QString sql = R"(
      INSERT OR IGNORE INTO "user" (id, name, lastname, username, password, role)
      VALUES (2, 'admin', 'admin', 'admin', :password, 'manager')
    )";

    query.prepare(sql);
    query.bindValue(":password", password_hash);

    if (!query.exec())
    {
      const auto err = query.lastError();
      qCritical() << "[DatabaseSchema::insertDefaultUsers] Failed to insert manager user";
      qCritical() << "  Error:" << err.text();

      return makeFailure(DbErrorCode::QUERY_EXECUTION_FAILED, "Failed to insert default manager user", sql, err.driverText(), err.databaseText());
    }
  }

  return makeSuccess();
}

DbResultVoid DatabaseSchema::executeSql(const QSqlDatabase& db, const QString& sql, const QString& description)
{
  QSqlQuery query(db);

  if (!query.exec(sql))
  {
    const auto err = query.lastError();
    qCritical() << "[DatabaseSchema::executeSql] Failed to create" << description;
    qCritical() << "  SQL:" << sql;
    qCritical() << "  Error:" << err.text();

    return makeFailure(DbErrorCode::QUERY_EXECUTION_FAILED, QString("Failed to create %1").arg(description), sql, err.driverText(), err.databaseText());
  }

  return makeSuccess();
}
