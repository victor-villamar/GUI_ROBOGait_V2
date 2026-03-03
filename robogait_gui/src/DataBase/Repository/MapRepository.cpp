#include <QSqlQuery>
#include <QVariant>

#include "DataBase/Repository/MapRepository.hpp"

using namespace ROBOGait::db;

MapRepository::MapRepository(RoboGaitDb& db) : Repository(db) {}

MapRepository::~MapRepository() = default;

DbResultVoid MapRepository::insertMapForUserName(const QString& user_name, const QString& map_name, const QString& location, const QString& details)
{
  QSqlQuery query(getDataBase());

  // clang-format off
  const QString sql =
        "INSERT INTO map (name, map_save, location, details, create_day, id_user) "
        "SELECT :name, 0, :location, :details, CURRENT_TIMESTAMP, id "
        "FROM \"user\" WHERE username = :username;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    return result;
  }

  query.bindValue(":name", map_name);
  query.bindValue(":location", location);
  query.bindValue(":details", details);
  query.bindValue(":username", user_name);

  return executeQuery(query);
}

DbResult<QVector<QString>> MapRepository::listMapNames()
{
  QSqlQuery query(getDataBase());

  // clang-format off
    const QString sql =
        "SELECT name "
        "FROM map;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<QVector<QString>>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  if (auto result = executeQuery(query); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<QVector<QString>>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  QVector<QString> maps_output;
  while (query.next())
  {
    maps_output.append(query.value(0).toString());
  }
  return maps_output;
}

DbResult<MapDetails> MapRepository::getMapDetailsByName(const QString& map_name)
{
  QSqlQuery query(getDataBase());

  // clang-format off
  const QString sql =
      "SELECT m.id, m.name, m.location, m.details, m.create_day, m.id_user, m.map_save, u.name, u.lastname "
      "FROM map m "
      "JOIN \"user\" u ON m.id_user = u.id "
      "WHERE m.name = :name;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<MapDetails>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  query.bindValue(":name", map_name);

  if (auto result = executeQuery(query); !statusOk(result))
  {
    const auto error = std::get<DbError>(result);
    return makeFailureT<MapDetails>(error.code, error.message, error.sql, error.driver_text, error.database_text);
  }

  if (!query.next())
  {
    return makeFailureT<MapDetails>(DbErrorCode::NOT_FOUND, "Map not found", sql);
  }

  MapDetails details;
  details.map.id = query.value(0).toInt();
  details.map.name = query.value(1).toString();
  details.map.location = query.value(2).toString();
  details.map.details = query.value(3).toString();
  details.map.create_day = query.value(4).toString();
  details.map.id_user = query.value(5).toInt();
  details.map.map_save = (query.value(6).toInt() != 0);
  details.user_name = query.value(7).toString();
  details.user_last_name = query.value(8).toString();

  return details;
}

DbResultVoid MapRepository::setMapSaved(const QString& map_name, bool saved)
{
  QSqlQuery query(getDataBase());

  // clang-format off
    const QString sql =
        "UPDATE map "
        "SET map_save = :save "
        "WHERE name = :name;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    return result;
  }

  query.bindValue(":save", saved ? 1 : 0);
  query.bindValue(":name", map_name);

  return executeQuery(query);
}

DbResultVoid MapRepository::deleteMapByName(const QString& map_name)
{
  QSqlQuery query(getDataBase());

  // clang-format off
    const QString sql =
        "DELETE FROM map "
        "WHERE name = :name;";
  // clang-format on

  if (auto result = prepareQuery(query, sql); !statusOk(result))
  {
    return result;
  }

  query.bindValue(":name", map_name);

  return executeQuery(query);
}