#pragma once

#include <QSqlDatabase>
#include <QString>

#include "DataBase/Common/DbErrors.hpp"

namespace ROBOGait
{
namespace db
{
/**
 * @brief Manages SQLite database connections for the RoboGait application
 */
class RoboGaitDb
{
public:
  /**
   * @brief Constructor for the RoboGaitDb class
   */
  RoboGaitDb();

  /**
   * @brief Destructor for the RoboGaitDb class
   */
  ~RoboGaitDb() = default;

  /**
   * @brief Open a connection to the database
   * @param db_path The path to the database file
   * @return A DbResultVoid indicating success or failure
   */
  DbResultVoid open(const QString& db_path);

  /**
   * @brief Close the connection to the database
   */
  void close();

  /**
   * @brief Check if the database connection is open
   * @return True if the connection is open, false otherwise
   */
  bool isOpen() const;

  /**
   * @brief Get a reference to the underlying QSqlDatabase
   * @return A reference to the QSqlDatabase
   */
  QSqlDatabase& getDataBase();

  /**
   * @brief Get a const reference to the underlying QSqlDatabase
   * @return A const reference to the QSqlDatabase
   */
  const QSqlDatabase& getDataBase() const;

private:
  QSqlDatabase db_;         /**< The underlying QSqlDatabase instance */
  QString connection_name_; /**< The name of the database connection */
};
} // namespace db
} // namespace ROBOGait