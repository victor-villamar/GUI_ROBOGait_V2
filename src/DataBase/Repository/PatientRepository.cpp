#include <QSqlQuery>
#include <QVariant>

#include "DataBase/Repository/PatientRepository.hpp"

using namespace ROBOGait::db;

PatientRepository::PatientRepository(RoboGaitDb& db) : Repository(db) {}

PatientRepository::~PatientRepository() = default;

DbResultVoid PatientRepository::insertPatientForUserName(const QString& name, const QString& last_name, int age, double weight, double height,
                                                         const QString& user_name, const QString& description)
{
  QSqlQuery query(getDataBase());

  // clang-format off
    const QString sql =
        "INSERT INTO patient (name, lastname, age, weight, height, description, create_day, id_user) "
        "SELECT :name, :lastname, :age, :weight, :height, :description, CURRENT_TIMESTAMP, id "
        "FROM \"user\" WHERE username = :username;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    return result;
  }

  query.bindValue(":name", name);
  query.bindValue(":lastname", last_name);
  query.bindValue(":age", age);
  query.bindValue(":weight", weight);
  query.bindValue(":height", height);
  query.bindValue(":description", description);
  query.bindValue(":username", user_name);

  return executeQuery(query);
}

DbResultVoid PatientRepository::deletePatientByIdForUserName(int patient_id, const QString& user_name)
{
  QSqlQuery query(getDataBase());

  // clang-format off
    const QString sql =
        "DELETE FROM patient "
        "WHERE id = :patient_id "
        "  AND id_user = (SELECT id FROM \"user\" WHERE username = :username);";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    return result;
  }

  query.bindValue(":patient_id", patient_id);
  query.bindValue(":username", user_name);

  if (auto result = executeQuery(query); !statusOk(result))
  {
    return result;
  }

  if (query.numRowsAffected() == 0)
  {
    return makeFailure(DbErrorCode::NOT_FOUND, "Patient not found or does not belong to the user");
  }

  return makeSuccess();
}

DbResult<QVector<PatientRow>> PatientRepository::listPatientsForUserName(const QString& user_name)
{
  QSqlQuery query(getDataBase());

  // clang-format off
    const QString sql =
        "SELECT id, name, lastname "
        "FROM patient "
        "WHERE id_user = (SELECT id FROM \"user\" WHERE username = :username) "
        "ORDER BY lastname, name;";
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
        "SELECT p.id, p.name, p.lastname, p.age, p.weight, p.height, p.description, p.create_day, p.id_user, "
        "       u.name, u.lastname "
        "FROM patient p "
        "JOIN \"user\" u ON p.id_user = u.id "
        "WHERE p.id = :patient_id "
        "  AND p.id_user = (SELECT id FROM \"user\" WHERE username = :username);";
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
  details.patient.id_user = query.value(8).toInt();
  details.user_name = query.value(9).toString();
  details.user_last_name = query.value(10).toString();

  return details;
}
