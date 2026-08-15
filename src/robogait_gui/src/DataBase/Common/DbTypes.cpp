#include <QStringLiteral>
#include <QStringView>

#include "DataBase/Common/DbTypes.hpp"

namespace ROBOGait
{
namespace db
{

QString userRoleToDbString(UserRole role)
{
  switch (role)
  {
    case UserRole::DOCTOR:
      return QStringLiteral("doctor");
    case UserRole::MANAGER:
      return QStringLiteral("manager");
    case UserRole::GUEST:
      return QStringLiteral("guest");
    default:
      return QStringLiteral("unknown");
  }
}

UserRole userRoleFromDbString(const QString& role)
{
  const auto role_view = QStringView{role}.trimmed();

  if (role_view.compare(u"doctor", Qt::CaseInsensitive) == 0)
  {
    return UserRole::DOCTOR;
  }
  else if (role_view.compare(u"manager", Qt::CaseInsensitive) == 0)
  {
    return UserRole::MANAGER;
  }
  else if (role_view.compare(u"guest", Qt::CaseInsensitive) == 0)
  {
    return UserRole::GUEST;
  }

  return UserRole::UNKNOWN;
}

QVariantMap toVariantMap(const PatientDetails& patient_details)
{
  QVariantMap map;
  map["first_name"] = patient_details.patient.name;
  map["last_name"] = patient_details.patient.last_name;
  map["id"] = patient_details.patient.id;
  map["age"] = patient_details.patient.age;
  map["weight"] = patient_details.patient.weight;
  map["height"] = patient_details.patient.height;
  map["description"] = patient_details.patient.description;
  map["create_day"] = patient_details.patient.create_day;
  map["user_name"] = patient_details.user_name;
  map["user_last_name"] = patient_details.user_last_name;
  map["doctor_names"] = patient_details.doctor_names;
  map["display_name"] = (patient_details.patient.last_name + ", " + patient_details.patient.name).trimmed();
  return map;
}

QVariantMap toVariantMap(const MapDetails& map_details)
{
  const QString create_by_name = (map_details.user_name + " " + map_details.user_last_name).trimmed();
  QVariantMap map;
  map["map_name"] = map_details.map.name;
  map["location"] = map_details.map.location;
  map["details"] = map_details.map.details;
  map["create_day"] = map_details.map.create_day;
  map["create_by_name"] = create_by_name;
  map["map_save"] = map_details.map.map_save;
  return map;
}
} // namespace db
} // namespace ROBOGait
