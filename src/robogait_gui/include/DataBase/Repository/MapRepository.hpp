#pragma once
#pragma once

#include <QVector>

#include "DataBase/Common/DbTypes.hpp"
#include "DataBase/Repository/Repository.hpp"

namespace ROBOGait
{
namespace db
{
/**
 * @brief Repository for map-related database operations
 */
class MapRepository : public Repository
{
public:
  /**
   * @brief Constructor for the MapRepository class
   *
   * @param db Reference to the RoboGait database
   */
  MapRepository(RoboGaitDb& db);

  /**
   * @brief Destructor for the MapRepository class
   */
  ~MapRepository() override;

  /**
   * @brief Insert a map record for a specific user
   *
   * @param user_name The username of the user associated with the map
   * @param map_name The name of the map
   * @param location The location of the map
   * @param details Additional details about the map
   *
   * @return Result of the database operation
   */
  DbResultVoid insertMapForUserName(const QString& user_name, const QString& map_name, const QString& location, const QString& details);

  /**
   * @brief List all map names
   *
   * @return Result containing a vector of map names, or error if operation fails
   */
  DbResult<QVector<QString>> listMapNames();

  /**
   * @brief Get details of a specific map by name
   *
   * @param map_name The name of the map
   *
   * @return Result containing map details, or error if operation fails
   */
  DbResult<MapDetails> getMapDetailsByName(const QString& map_name);

  /**
   * @brief Set the saved status of a specific map
   *
   * @param map_name The name of the map
   * @param saved The saved status to set
   *
   * @return Result of the database operation
   */
  DbResultVoid setMapSaved(const QString& map_name, bool saved);

  /**
   * @brief Delete a specific map by name
   *
   * @param map_name The name of the map
   *
   * @return Result of the database operation
   */
  DbResultVoid deleteMapByName(const QString& map_name);
};
} // namespace db
} // namespace ROBOGait
