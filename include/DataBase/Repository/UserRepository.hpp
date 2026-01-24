#pragma once

#include <optional>

#include "DataBase/Common/DbTypes.hpp"
#include "DataBase/Repository/Repository.hpp"

namespace ROBOGait
{
namespace db
{
/**
 * @brief Repository for user-related database operations
 */
class UserRepository : public Repository
{
public:
  /**
   * @brief Constructor of UserRepository class
   *
   * @param db Reference to the RoboGaitDb instance
   */
  UserRepository(RoboGaitDb& db);

  /**
   * @brief Destructor of UserRepository class
   */
  ~UserRepository() override;

  /**
   * @brief Find a user by their credentials
   *
   * @param user_name The username of the user
   * @param password_hash The password hash of the user
   * @return An optional UserRow if found, or an error
   */
  DbResult<std::optional<UserRow>> findByCredentials(const QString& user_name, const QString& password_hash) const;

  /**
   * @brief Check if a username already exists
   *
   * @param user_name The username to check
   * @return True if the username exists, false otherwise
   */
  DbResult<bool> userNameExists(const QString& user_name) const;

  /**
   * @brief Get the user ID by username
   *
   * @param user_name The username of the user
   * @return The user ID if found, or an error
   */
  DbResult<int> getUserIdByUserName(const QString& user_name) const;

  /**
   * @brief Insert a new user into the database
   *
   * @param name The name of the user
   * @param last_name The last name of the user
   * @param user_name The username of the user
   * @param password_hash The password hash of the user
   * @param role The role of the user
   * @return Success or failure
   */
  DbResultVoid insertUser(const QString& name, const QString& last_name, const QString& user_name, const QString& password_hash, UserRole role) const;
};
} // namespace db
} // namespace ROBOGait
