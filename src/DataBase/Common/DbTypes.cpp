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
    {
      return "doctor";
    }
    case UserRole::MANAGER:
    {
      return "manager";
    }
    case UserRole::GUEST:
    {
      return "guest";
    }
    default:
    {
      return "unknown";
    }
  }
}

UserRole userRoleFromDbString(const QString& role)
{
  const auto roleStr = role.trimmed().toLower();
  if (roleStr == "doctor")
  {
    return UserRole::DOCTOR;
  }
  else if (roleStr == "manager")
  {
    return UserRole::MANAGER;
  }
  else if (roleStr == "guest")
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
  map["id_user"] = patient_details.patient.id_user;
  map["user_name"] = patient_details.user_name;
  map["user_last_name"] = patient_details.user_last_name;
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