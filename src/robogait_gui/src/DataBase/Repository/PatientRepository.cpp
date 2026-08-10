#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "DataBase/Repository/PatientRepository.hpp"

using namespace ROBOGait::db;

PatientRepository::PatientRepository(RoboGaitDb& db) : Repository(db) {}

PatientRepository::~PatientRepository() = default;

DbResultVoid PatientRepository::insertPatientForUserName(const QString& name, const QString& last_name, int age, double weight, double height,
                                                         const QString& user_name, const QString& description)
{
  QSqlDatabase& db = getDataBase();

  if (!db.transaction())
  {
    const auto error = db.lastError();
    return makeFailure(DbErrorCode::TRANSACTION_FAILED, "Failed to start transaction", {}, error.driverText(), error.databaseText());
  }

  int patient_id = -1;

  QSqlQuery find_patient_query(db);

  // clang-format off
  const QString find_patient_sql =
      "SELECT id "
      "FROM patient "
      "WHERE name = :name AND lastname = :lastname "
      "ORDER BY id ASC "
      "LIMIT 1;";
  // clang-format on

  if (auto result = prepareQuery(find_patient_query, find_patient_sql); !statusOk(result))
  {
    db.rollback();
    return result;
  }

  find_patient_query.bindValue(":name", name);
  find_patient_query.bindValue(":lastname", last_name);

  if (auto result = executeQuery(find_patient_query); !statusOk(result))
  {
    db.rollback();
    return result;
  }

  if (find_patient_query.next())
  {
    patient_id = find_patient_query.value(0).toInt();
  }
  else
  {
    QSqlQuery insert_patient_query(db);

    // clang-format off
    const QString patient_sql =
        "INSERT INTO patient (name, lastname, age, weight, height) "
        "VALUES (:name, :lastname, :age, :weight, :height);";
    // clang-format on

    if (auto result = prepareQuery(insert_patient_query, patient_sql); !statusOk(result))
    {
      db.rollback();
      return result;
    }

    insert_patient_query.bindValue(":name", name);
    insert_patient_query.bindValue(":lastname", last_name);
    insert_patient_query.bindValue(":age", age);
    insert_patient_query.bindValue(":weight", weight);
    insert_patient_query.bindValue(":height", height);

    if (auto result = executeQuery(insert_patient_query); !statusOk(result))
    {
      db.rollback();
      return result;
    }

    QSqlQuery last_id_query(db);

    if (!last_id_query.exec("SELECT last_insert_rowid();") || !last_id_query.next())
    {
      const auto error = last_id_query.lastError();
      db.rollback();
      return makeFailure(DbErrorCode::UNEXPECTED_RESULT, "Failed to fetch last insert ID", last_id_query.lastQuery(), error.driverText(), error.databaseText());
    }

    patient_id = last_id_query.value(0).toInt();
  }

  QSqlQuery relation_query(db);

  // clang-format off
  const QString relation_sql =
      "INSERT INTO patient_doctor (id_patient, id_doctor, create_day, description) "
      "SELECT :patient_id, id, CURRENT_TIMESTAMP, :description "
      "FROM \"user\" WHERE username = :username "
      "ON CONFLICT(id_patient, id_doctor) DO UPDATE SET description = excluded.description;";
  // clang-format on

  if (auto result = prepareQuery(relation_query, relation_sql); !statusOk(result))
  {
    db.rollback();
    return result;
  }

  relation_query.bindValue(":patient_id", patient_id);
  relation_query.bindValue(":description", description);
  relation_query.bindValue(":username", user_name);

  if (auto result = executeQuery(relation_query); !statusOk(result))
  {
    db.rollback();
    return result;
  }

  if (relation_query.numRowsAffected() == 0)
  {
    QSqlQuery user_check_query(db);
    if (!user_check_query.prepare("SELECT id FROM \"user\" WHERE username = :username LIMIT 1;"))
    {
      const auto error = user_check_query.lastError();
      db.rollback();
      return makeFailure(DbErrorCode::QUERY_PREPARE_FAILED, "Failed to check user existence", {}, error.driverText(), error.databaseText());
    }
    user_check_query.bindValue(":username", user_name);
    if (!user_check_query.exec())
    {
      const auto error = user_check_query.lastError();
      db.rollback();
      return makeFailure(DbErrorCode::QUERY_EXECUTION_FAILED, "Failed to check user existence", user_check_query.lastQuery(), error.driverText(),
                         error.databaseText());
    }
    if (!user_check_query.next())
    {
      db.rollback();
      return makeFailure(DbErrorCode::NOT_FOUND, "User not found when linking patient", relation_sql);
    }
  }

  if (!db.commit())
  {
    const auto error = db.lastError();
    db.rollback();
    return makeFailure(DbErrorCode::TRANSACTION_FAILED, "Failed to commit transaction", {}, error.driverText(), error.databaseText());
  }

  return makeSuccess();
}

DbResultVoid PatientRepository::deletePatientByIdForUserName(int patient_id, const QString& user_name)
{
  QSqlDatabase& db = getDataBase();

  if (!db.transaction())
  {
    const auto error = db.lastError();
    return makeFailure(DbErrorCode::TRANSACTION_FAILED, "Failed to start transaction", {}, error.driverText(), error.databaseText());
  }

  QSqlQuery unlink_query(db);

  // clang-format off
  const QString unlink_sql =
      "DELETE FROM patient_doctor "
      "WHERE id_patient = :patient_id "
      "  AND id_doctor = (SELECT id FROM \"user\" WHERE username = :username);";
  // clang-format on

  if (auto result = prepareQuery(unlink_query, unlink_sql); !statusOk(result))
  {
    db.rollback();
    return result;
  }

  unlink_query.bindValue(":patient_id", patient_id);
  unlink_query.bindValue(":username", user_name);

  if (auto result = executeQuery(unlink_query); !statusOk(result))
  {
    db.rollback();
    return result;
  }

  if (unlink_query.numRowsAffected() == 0)
  {
    db.rollback();
    return makeFailure(DbErrorCode::NOT_FOUND, "Patient not found or does not belong to the user");
  }

  QSqlQuery delete_patient_query(db);

  // clang-format off
  const QString delete_patient_sql =
      "DELETE FROM patient "
      "WHERE id = :patient_id "
      "  AND NOT EXISTS (SELECT 1 FROM patient_doctor WHERE id_patient = :patient_id);";
  // clang-format on

  if (auto result = prepareQuery(delete_patient_query, delete_patient_sql); !statusOk(result))
  {
    db.rollback();
    return result;
  }

  delete_patient_query.bindValue(":patient_id", patient_id);

  if (auto result = executeQuery(delete_patient_query); !statusOk(result))
  {
    db.rollback();
    return result;
  }

  if (!db.commit())
  {
    const auto error = db.lastError();
    db.rollback();
    return makeFailure(DbErrorCode::TRANSACTION_FAILED, "Failed to commit transaction", {}, error.driverText(), error.databaseText());
  }

  return makeSuccess();
}

DbResult<QVector<PatientRow>> PatientRepository::listPatientsForUserName(const QString& user_name)
{
  QSqlQuery query(getDataBase());

  // clang-format off
    const QString sql =
        "SELECT DISTINCT p.id, p.name, p.lastname "
        "FROM patient p "
        "JOIN patient_doctor pd ON pd.id_patient = p.id "
        "JOIN \"user\" u ON pd.id_doctor = u.id "
        "WHERE u.username = :username "
        "ORDER BY p.lastname, p.name;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<QVector<PatientRow>>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  query.bindValue(":username", user_name);

  if (auto result = executeQuery(query); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<QVector<PatientRow>>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  QVector<PatientRow> patients_output;

  while (query.next())
  {
    PatientRow patient;
    patient.id = query.value(0).toInt();
    patient.name = query.value(1).toString();
    patient.last_name = query.value(2).toString();

    patients_output.append(patient);
  }

  return patients_output;
}

DbResult<PatientDetails> PatientRepository::getPatientDetailsByIdForUserName(int patient_id, const QString& user_name)
{
  QSqlQuery query(getDataBase());

  // clang-format off
    const QString sql =
        "SELECT p.id, p.name, p.lastname, p.age, p.weight, p.height, pd_auth.description, pd_auth.create_day, "
        "       u_auth.name, u_auth.lastname, "
        "       GROUP_CONCAT(u_all.name || ' ' || u_all.lastname, ', ') AS doctor_names "
        "FROM patient p "
        "JOIN patient_doctor pd_auth ON pd_auth.id_patient = p.id "
        "JOIN \"user\" u_auth ON pd_auth.id_doctor = u_auth.id "
        "JOIN patient_doctor pd_all ON pd_all.id_patient = p.id "
        "JOIN \"user\" u_all ON pd_all.id_doctor = u_all.id "
        "WHERE p.id = :patient_id "
        "  AND u_auth.username = :username "
        "GROUP BY p.id, u_auth.name, u_auth.lastname;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<PatientDetails>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  query.bindValue(":patient_id", patient_id);
  query.bindValue(":username", user_name);

  if (auto result = executeQuery(query); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<PatientDetails>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  if (!query.next())
  {
    return makeFailureT<PatientDetails>(DbErrorCode::NOT_FOUND, "Patient not found");
  }

  PatientDetails details;
  details.patient.id = query.value(0).toInt();
  details.patient.name = query.value(1).toString();
  details.patient.last_name = query.value(2).toString();
  details.patient.age = query.value(3).toInt();
  details.patient.weight = query.value(4).toDouble();
  details.patient.height = query.value(5).toDouble();
  details.patient.description = query.value(6).toString();
  details.patient.create_day = query.value(7).toString();
  details.user_name = query.value(8).toString();
  details.user_last_name = query.value(9).toString();
  details.doctor_names = query.value(10).toString();

  return details;
}

DbResult<PatientRow> PatientRepository::getPatientBasicInfoByIdForUserName(int patient_id, const QString& user_name)
{
  QSqlQuery query(getDataBase());

  // clang-format off
  const QString sql =
      "SELECT p.id, p.name, p.lastname "
      "FROM patient p "
      "JOIN patient_doctor pd ON pd.id_patient = p.id "
      "JOIN \"user\" u ON pd.id_doctor = u.id "
      "WHERE p.id = :patient_id "
      "  AND u.username = :username;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<PatientRow>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  query.bindValue(":patient_id", patient_id);
  query.bindValue(":username", user_name);

  if (auto result = executeQuery(query); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<PatientRow>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  if (!query.next())
  {
    return makeFailureT<PatientRow>(DbErrorCode::NOT_FOUND, "Patient not found");
  }

  PatientRow patient_row;
  patient_row.id = query.value(0).toInt();
  patient_row.name = query.value(1).toString();
  patient_row.last_name = query.value(2).toString();

  return patient_row;
}

DbResult<QVector<PatientDoctorInfo>> PatientRepository::listPatientDoctorsByPatientIdForUserName(int patient_id, const QString& user_name)
{
  QSqlQuery query(getDataBase());

  // clang-format off
  const QString sql =
      "SELECT u.id, u.name, u.lastname, pd.create_day, pd.description "
      "FROM patient_doctor pd "
      "JOIN \"user\" u ON pd.id_doctor = u.id "
      "WHERE pd.id_patient = :patient_id "
      "  AND EXISTS ("
      "    SELECT 1 "
      "    FROM patient_doctor pd2 "
      "    JOIN \"user\" u2 ON pd2.id_doctor = u2.id "
      "    WHERE pd2.id_patient = :patient_id "
      "      AND u2.username = :username"
      "  ) "
      "ORDER BY u.lastname, u.name;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<QVector<PatientDoctorInfo>>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  query.bindValue(":patient_id", patient_id);
  query.bindValue(":username", user_name);

  if (auto result = executeQuery(query); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<QVector<PatientDoctorInfo>>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  QVector<PatientDoctorInfo> doctors_output;

  while (query.next())
  {
    PatientDoctorInfo info;
    info.doctor_id = query.value(0).toInt();
    info.doctor_name = query.value(1).toString();
    info.doctor_last_name = query.value(2).toString();
    info.create_day = query.value(3).toString();
    info.description = query.value(4).toString();
    doctors_output.append(info);
  }

  return doctors_output;
}
