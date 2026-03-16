#include "User/Patient.hpp"

using namespace ROBOGait::user;

Patient::Patient() : id_(-1), name_(""), last_name_(""), display_name_(""), doctor_diagnostics_() {}

int Patient::getId() const { return id_; }

QString Patient::getName() const { return name_; }

QString Patient::getLastName() const { return last_name_; }

QString Patient::getDisplayName() const { return display_name_; }

bool Patient::isActive() const { return id_ >= 0; }

QString Patient::getStatusRichText() const
{
  if (isActive())
  {
    return "<span style='color:#cce54d'>HAY PACIENTE ACTIVO</span>";
  }

  return "<span style='color:#cc0000'>NO</span><span style='color:#ffffff'> HAY PACIENTE ACTIVO</span>";
}

QVariantList Patient::getDoctorDiagnostics() const { return doctor_diagnostics_; }

void Patient::selectPatient(int id, const QString& name, const QString& lastName, const QString& displayName)
{
  id_ = id;
  name_ = name;
  last_name_ = lastName;
  display_name_ = displayName;
  doctor_diagnostics_.clear();
  emit patientChanged();
}

void Patient::setDoctorDiagnostics(const QVariantList& doctor_diagnostics)
{
  if (doctor_diagnostics != doctor_diagnostics_)
  {
    doctor_diagnostics_ = doctor_diagnostics;
    emit patientChanged();
  }
}

void Patient::clear()
{
  id_ = -1;
  name_.clear();
  last_name_.clear();
  display_name_.clear();
  doctor_diagnostics_.clear();
  emit patientChanged();
}
