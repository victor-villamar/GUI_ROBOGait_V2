#include "User/Patient.hpp"
#include "Themes/AppTheme.hpp"
#include "Themes/ThemeMainMenu.hpp"

using namespace ROBOGait::user;

Patient::Patient() : id_(-1), name_(""), last_name_(""), display_name_(""), doctor_diagnostics_() {}

int Patient::getId() const { return id_; }

QString Patient::getName() const { return name_; }

QString Patient::getLastName() const { return last_name_; }

QString Patient::getDisplayName() const { return display_name_; }

bool Patient::isActive() const { return id_ >= 0; }

QString Patient::getStatusRichText() const
{
  auto& theme = ROBOGait::settings::AppTheme::getInstance();
  const auto* main_menu_theme = qobject_cast<const ROBOGait::settings::ThemeMainMenu*>(theme.getMainMenu());

  const QString active_color = main_menu_theme ? main_menu_theme->getStatusMapActive().name() : QStringLiteral("#cce54d");
  const QString error_color = main_menu_theme ? main_menu_theme->getStatusError().name() : QStringLiteral("#cc0000");
  const QString text_primary_color = main_menu_theme ? main_menu_theme->getTextPrimary().name() : QStringLiteral("#ffffff");

  if (isActive())
  {
    const QString display = display_name_.isEmpty() ? (last_name_ + ", " + name_).trimmed() : display_name_;
    return QString("<span style='color:%1'>PACIENTE: </span><span style='color:%2'>%3</span>").arg(active_color, text_primary_color, display);
  }

  return QString("<span style='color:%1'>NO</span><span style='color:%2'> HAY PACIENTE ACTIVO</span>").arg(error_color, text_primary_color);
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
