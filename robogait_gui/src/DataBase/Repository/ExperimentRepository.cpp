#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "DataBase/Repository/ExperimentRepository.hpp"

using namespace ROBOGait::db;

ExperimentRepository::ExperimentRepository(RoboGaitDb& db) : Repository(db) {}

ExperimentRepository::~ExperimentRepository() = default;

DbResult<int> ExperimentRepository::insertExperiment(const QString& patient_name, const QString& patient_last_name, const QString& map_name,
                                                     const QString& user_name, const std::optional<QString>& comment)
{
  QSqlQuery query(getDataBase());

  // clang-format off
    const QString sql =
        "INSERT INTO experiments (id_patient, id_map, id_user, date, comment) "
        "SELECT p.id, m.id, u.id, CURRENT_TIMESTAMP, :comment "
        "FROM patient p, map m, \"user\" u "
        "WHERE p.name = :p_name AND p.lastname = :p_lastname "
        "  AND m.name = :m_name "
        "  AND u.username = :u_name;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<int>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  query.bindValue(":p_name", patient_name);
  query.bindValue(":p_lastname", patient_last_name);
  query.bindValue(":m_name", map_name);
  query.bindValue(":u_name", user_name);
  query.bindValue(":comment", comment.has_value() ? QVariant(*comment) : QVariant());

  if (auto result = executeQuery(query); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<int>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  QSqlQuery last_id_query(getDataBase());

  if (!last_id_query.exec("SELECT last_insert_rowid();") || !last_id_query.next())
  {
    const auto error = last_id_query.lastError();
    return makeFailureT<int>(DbErrorCode::UNEXPECTED_RESULT, "Failed to fetch last insert ID", last_id_query.lastQuery(), error.driverText(),
                             error.databaseText());
  }

  return last_id_query.value(0).toInt();
}

DbResultVoid ExperimentRepository::deleteExperimentByIdForUserName(int experiment_id, const QString& user_name)
{
  QSqlQuery delete_experiment_query(getDataBase());

  // clang-format off
  const QString delete_experiment_sql =
      "DELETE FROM experiments "
      "WHERE id = :experiment_id "
      "  AND id_user = (SELECT id FROM \"user\" WHERE username = :username);";
  // clang-format on

  if (auto result = prepareQuery(delete_experiment_query, delete_experiment_sql); !statusOk(result))
  {
    return result;
  }

  delete_experiment_query.bindValue(":experiment_id", experiment_id);
  delete_experiment_query.bindValue(":username", user_name);

  if (auto result = executeQuery(delete_experiment_query); !statusOk(result))
  {
    return result;
  }

  if (delete_experiment_query.numRowsAffected() == 0)
  {
    return makeFailure(DbErrorCode::NOT_FOUND, "Experiment not found", delete_experiment_sql);
  }

  return makeSuccess();
}
