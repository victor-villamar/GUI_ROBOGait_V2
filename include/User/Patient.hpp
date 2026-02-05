#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

namespace ROBOGait
{
namespace user
{
class Patient : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Construct of Patient class.
   */
  Patient();

  // clang-format off
  Q_PROPERTY(int id
             READ getId
             NOTIFY patientChanged
  )
  Q_PROPERTY(QString name
             READ getName
             NOTIFY patientChanged
  )
  Q_PROPERTY(QString lastName
             READ getLastName
             NOTIFY patientChanged
  )
  Q_PROPERTY(QString displayName
             READ getDisplayName
             NOTIFY patientChanged
  )
  Q_PROPERTY(bool isActive
             READ isActive
             NOTIFY patientChanged
  )

  Q_PROPERTY(QString statusRichText
             READ getStatusRichText
             NOTIFY patientChanged
  )

  Q_PROPERTY(QVariantList doctorDiagnostics
             READ getDoctorDiagnostics
             NOTIFY patientChanged
  )

  // clang-format on

  /**
   * @brief Get the patient ID
   *
   * @return The patient ID
   */
  int getId() const;

  /**
   * @brief Get the patient name
   *
   * @return The patient name
   */
  QString getName() const;

  /**
   * @brief Get the patient last name
   *
   * @return The patient last name
   */
  QString getLastName() const;

  /**
   * @brief Get the patient display name
   *
   * @return The patient display name
   */
  QString getDisplayName() const;

  /**
   * @brief Check if the patient is active
   *
   * @return True if the patient is active, false otherwise
   */
  bool isActive() const;

  /**
   * @brief Get the patient status as rich text
   *
   * @return The patient status rich text
   */
  QString getStatusRichText() const;

  /**
   * @brief Get diagnostics for doctors associated with the patient
   *
   * @return List of doctor diagnostics
   */
  QVariantList getDoctorDiagnostics() const;

  /**
   * @brief Select a patient.
   * @param id The patient ID.
   * @param name The patient name.
   * @param lastName The patient last name.
   * @param displayName The patient display name.
   */
  void selectPatient(int id, const QString& name, const QString& lastName, const QString& displayName);

  /**
   * @brief Set doctor diagnostics list for the patient
   *
   * @param doctor_diagnostics The diagnostics list
   */
  void setDoctorDiagnostics(const QVariantList& doctor_diagnostics);

  /**
   * @brief Clear the patient selection.
   */
  void clear();

signals:
  void patientChanged();

private:
  int id_;                          /**< The patient ID */
  QString name_;                    /**< The patient name */
  QString last_name_;               /**< The patient last name */
  QString display_name_;            /**< The patient display name */
  QVariantList doctor_diagnostics_; /**< The patient doctors diagnostics */
};
} // namespace user
} // namespace ROBOGait
