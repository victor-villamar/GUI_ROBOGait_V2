#pragma once

#include <QVector>

#include "DataBase/Common/DbTypes.hpp"
#include "DataBase/Repository/Repository.hpp"

namespace ROBOGait
{
namespace db
{
/**
 * @brief Repository for patient-related database operations
 */
class PatientRepository : public Repository
{
public:
  /**
   * @brief Constructor of PatientRepository class
   *
   * @param db Reference to the RoboGait database
   */
  PatientRepository(RoboGaitDb& db);

  /**
   * @brief Destructor of PatientRepository class
   */
  ~PatientRepository() override;

  /**
   * @brief Insert a patient record for a specific user
   *
   * @param name The name of the patient
   * @param last_name The last name of the patient
   * @param age The age of the patient
   * @param weight The weight of the patient
   * @param height The height of the patient
   * @param user_name The username of the user associated with the patient
   * @param description A description of the patient's condition
   * @return Result of the insert operation
   */
  DbResultVoid insertPatientForUserName(const QString& name, const QString& last_name, int age, double weight, double height, const QString& user_name,
                                        const QString& description);

  /**
   * @brief Delete a patient record by ID
   *
   * @param patient_id The ID of the patient to delete
   * @param user_name The username of the user associated with the patient
   * @return Result of the delete operation
   */
  DbResultVoid deletePatientByIdForUserName(int patient_id, const QString& user_name);

  /**
   * @brief List all patients for a specific user
   *
   * @param user_name The username of the user associated with the patients
   * @return Result containing a vector of patient rows if successful, or error otherwise
   */
  DbResult<QVector<PatientRow>> listPatientsForUserName(const QString& user_name);

  /**
   * @brief Get detailed information about a specific patient
   *
   * @param patient_id The ID of the patient
   * @param user_name The username of the user associated with the patient
   * @return Result containing patient details if found, or error otherwise
   */
  DbResult<PatientDetails> getPatientDetailsByIdForUserName(int patient_id, const QString& user_name);

  /**
   * @brief Get basic information about a specific patient (id, name, last_name)
   *
   * @param patient_id The ID of the patient
   * @param user_name The username of the user associated with the patient
   * @return Result containing basic patient info if found, or error otherwise
   */
  DbResult<PatientRow> getPatientBasicInfoByIdForUserName(int patient_id, const QString& user_name);
};
} // namespace db
} // namespace ROBOGait
