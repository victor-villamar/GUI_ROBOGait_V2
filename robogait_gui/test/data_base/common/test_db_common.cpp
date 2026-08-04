#include <QString>
#include <QVariantMap>

#include <gtest/gtest.h>

#include "DataBase/Common/DbErrors.hpp"
#include "DataBase/Common/DbTypes.hpp"

TEST(DbErrorsTest, VoidResultReportsSuccessAndFailure)
{
  const ROBOGait::db::DbResultVoid success = ROBOGait::db::makeSuccess();
  const ROBOGait::db::DbResultVoid failure =
      ROBOGait::db::makeFailure(ROBOGait::db::DbErrorCode::INVALID_INPUT, "invalid input", "SELECT 1", "driver", "database");

  EXPECT_TRUE(ROBOGait::db::statusOk(success));
  EXPECT_FALSE(ROBOGait::db::statusOk(failure));

  const auto error = std::get<ROBOGait::db::DbError>(failure);
  EXPECT_EQ(error.code, ROBOGait::db::DbErrorCode::INVALID_INPUT);
  EXPECT_EQ(error.message, "invalid input");
  EXPECT_EQ(error.sql, "SELECT 1");
  EXPECT_EQ(error.driver_text, "driver");
  EXPECT_EQ(error.database_text, "database");
}

TEST(DbErrorsTest, TypedResultReportsSuccessAndFailure)
{
  const ROBOGait::db::DbResult<int> success = 42;
  const ROBOGait::db::DbResult<int> failure = ROBOGait::db::makeFailureT<int>(ROBOGait::db::DbErrorCode::NOT_FOUND, "missing");

  EXPECT_TRUE(ROBOGait::db::statusOk(success));
  EXPECT_FALSE(ROBOGait::db::statusOk(failure));
  EXPECT_EQ(std::get<int>(success), 42);
  EXPECT_EQ(std::get<ROBOGait::db::DbError>(failure).code, ROBOGait::db::DbErrorCode::NOT_FOUND);
}

TEST(DbTypesTest, ConvertsUserRolesToAndFromDatabaseStrings)
{
  EXPECT_EQ(ROBOGait::db::userRoleToDbString(ROBOGait::db::UserRole::DOCTOR), "doctor");
  EXPECT_EQ(ROBOGait::db::userRoleToDbString(ROBOGait::db::UserRole::MANAGER), "manager");
  EXPECT_EQ(ROBOGait::db::userRoleToDbString(ROBOGait::db::UserRole::GUEST), "guest");
  EXPECT_EQ(ROBOGait::db::userRoleToDbString(ROBOGait::db::UserRole::UNKNOWN), "unknown");

  EXPECT_EQ(ROBOGait::db::userRoleFromDbString(" doctor "), ROBOGait::db::UserRole::DOCTOR);
  EXPECT_EQ(ROBOGait::db::userRoleFromDbString("MANAGER"), ROBOGait::db::UserRole::MANAGER);
  EXPECT_EQ(ROBOGait::db::userRoleFromDbString("Guest"), ROBOGait::db::UserRole::GUEST);
  EXPECT_EQ(ROBOGait::db::userRoleFromDbString("admin"), ROBOGait::db::UserRole::UNKNOWN);
}

TEST(DbTypesTest, PatientDetailsVariantMapIncludesDisplayName)
{
  ROBOGait::db::PatientDetails patient_details;
  patient_details.patient.id = 7;
  patient_details.patient.name = "Ana";
  patient_details.patient.last_name = "Lopez";
  patient_details.patient.age = 34;
  patient_details.patient.weight = 62.5;
  patient_details.patient.height = 1.68;
  patient_details.patient.description = "diagnosis";
  patient_details.patient.create_day = "2026-08-04";
  patient_details.user_name = "Victor";
  patient_details.user_last_name = "Villamar";
  patient_details.doctor_names = "Victor Villamar";

  const QVariantMap map = ROBOGait::db::toVariantMap(patient_details);

  EXPECT_EQ(map.value("id").toInt(), 7);
  EXPECT_EQ(map.value("first_name").toString(), "Ana");
  EXPECT_EQ(map.value("last_name").toString(), "Lopez");
  EXPECT_EQ(map.value("age").toInt(), 34);
  EXPECT_DOUBLE_EQ(map.value("weight").toDouble(), 62.5);
  EXPECT_DOUBLE_EQ(map.value("height").toDouble(), 1.68);
  EXPECT_EQ(map.value("display_name").toString(), "Lopez, Ana");
  EXPECT_EQ(map.value("doctor_names").toString(), "Victor Villamar");
}

TEST(DbTypesTest, MapDetailsVariantMapIncludesCreatorAndSaveFlag)
{
  ROBOGait::db::MapDetails map_details;
  map_details.map.name = "Lab map";
  map_details.map.location = "Lab";
  map_details.map.details = "Ground floor";
  map_details.map.create_day = "2026-08-04";
  map_details.map.map_save = true;
  map_details.user_name = "Ana";
  map_details.user_last_name = "Lopez";

  const QVariantMap map = ROBOGait::db::toVariantMap(map_details);

  EXPECT_EQ(map.value("map_name").toString(), "Lab map");
  EXPECT_EQ(map.value("location").toString(), "Lab");
  EXPECT_EQ(map.value("details").toString(), "Ground floor");
  EXPECT_EQ(map.value("create_day").toString(), "2026-08-04");
  EXPECT_EQ(map.value("create_by_name").toString(), "Ana Lopez");
  EXPECT_TRUE(map.value("map_save").toBool());
}