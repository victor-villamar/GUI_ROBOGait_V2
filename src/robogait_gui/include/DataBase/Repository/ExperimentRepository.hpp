#pragma once

#include <optional>

#include "DataBase/Common/DbTypes.hpp"
#include "DataBase/Repository/Repository.hpp"

namespace ROBOGait
{
namespace db
{
/**
 * @brief Repository for experiment-related database operations
 */
class ExperimentRepository : public Repository
{
public:
  /**
   * @brief Constructor of ExperimentRepository class
   *
   * @param db Reference to the RoboGait database
   */
  ExperimentRepository(RoboGaitDb& db);

  /**
   * @brief Destructor of ExperimentRepository class
   */
  ~ExperimentRepository() override;

  /**
   * @brief Insert a new experiment into the database
   *
   * @param patient_name The name of the patient
   * @param patient_last_name The last name of the patient
   * @param map_name The name of the map
   * @param user_name The name of the user
   * @param comment Optional annotations for the experiment
   *
   * @return The ID of the inserted experiment
   */
  DbResult<int> insertExperiment(const QString& patient_name, const QString& patient_last_name, const QString& map_name, const QString& user_name,
                                 const std::optional<QString>& comment);

  /**
   * @brief Delete an experiment by ID for a given user
   *
   * @param experiment_id The ID of the experiment to delete
   * @param user_name The user name that owns the experiment
   *
   * @return Success or failure result
   */
  DbResultVoid deleteExperimentByIdForUserName(int experiment_id, const QString& user_name);

  /**
   * @brief List patient tests visible for the requesting user and patient
   *
   * @param patient_id The patient ID
   * @param user_name The requesting username
   * @return Result containing test entries
   */
  DbResult<QVector<PatientTestInfo>> listPatientTestsByPatientIdForUserName(int patient_id, const QString& user_name);
};
} // namespace db
} // namespace ROBOGait
