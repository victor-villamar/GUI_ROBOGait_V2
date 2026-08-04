#include <memory>
#include <optional>

#include <QCoreApplication>
#include <QVector>

#include <gtest/gtest.h>

#include "DataBase/Repository/ExperimentRepository.hpp"
#include "DataBase/Repository/MapRepository.hpp"
#include "DataBase/Repository/PatientRepository.hpp"
#include "DataBase/Repository/UserRepository.hpp"
#include "DataBase/RoboGaitDb.hpp"

namespace
{
class RepositoryTest : public ::testing::Test
{
protected:
  static void SetUpTestSuite()
  {
    if (QCoreApplication::instance() == nullptr)
    {
      static int argc = 1;
      static char app_name[] = "test_repositories";
      static char* argv[] = {app_name, nullptr};
      app_ = std::make_unique<QCoreApplication>(argc, argv);
    }
  }

  static void TearDownTestSuite() { app_.reset(); }

  void SetUp() override { ASSERT_TRUE(ROBOGait::db::statusOk(db_.open(":memory:"))); }

  void TearDown() override { db_.close(); }

  ROBOGait::db::RoboGaitDb db_;

private:
  static std::unique_ptr<QCoreApplication> app_;
};

std::unique_ptr<QCoreApplication> RepositoryTest::app_;
} // namespace

TEST_F(RepositoryTest, UserRepositoryInsertsAndFindsUser)
{
  ROBOGait::db::UserRepository user_repository(db_);

  ASSERT_TRUE(ROBOGait::db::statusOk(user_repository.insertUser("Doctor", "One", "doctor_one", "hash", ROBOGait::db::UserRole::DOCTOR)));

  const auto exists_result = user_repository.userNameExists("doctor_one");
  ASSERT_TRUE(ROBOGait::db::statusOk(exists_result));
  EXPECT_TRUE(std::get<bool>(exists_result));

  const auto id_result = user_repository.getUserIdByUserName("doctor_one");
  ASSERT_TRUE(ROBOGait::db::statusOk(id_result));
  EXPECT_GT(std::get<int>(id_result), 0);

  const auto user_result = user_repository.findByCredentials("doctor_one", "hash");
  ASSERT_TRUE(ROBOGait::db::statusOk(user_result));
  const auto user = std::get<std::optional<ROBOGait::db::UserRow>>(user_result);
  ASSERT_TRUE(user.has_value());
  EXPECT_EQ(user->name, "Doctor");
  EXPECT_EQ(user->last_name, "One");
  EXPECT_EQ(user->user_name, "doctor_one");
  EXPECT_EQ(user->role, ROBOGait::db::UserRole::DOCTOR);

  const auto missing_result = user_repository.getUserIdByUserName("missing");
  EXPECT_FALSE(ROBOGait::db::statusOk(missing_result));
  EXPECT_EQ(std::get<ROBOGait::db::DbError>(missing_result).code, ROBOGait::db::DbErrorCode::NOT_FOUND);
}

TEST_F(RepositoryTest, PatientRepositoryCreatesListsAndDeletesUserAssociation)
{
  ROBOGait::db::PatientRepository patient_repository(db_);

  ASSERT_TRUE(ROBOGait::db::statusOk(patient_repository.insertPatientForUserName("Ana", "Lopez", 34, 62.5, 1.68, "a", "left knee")));

  const auto patients_result = patient_repository.listPatientsForUserName("a");
  ASSERT_TRUE(ROBOGait::db::statusOk(patients_result));
  const QVector<ROBOGait::db::PatientRow> patients = std::get<QVector<ROBOGait::db::PatientRow>>(patients_result);
  ASSERT_EQ(patients.size(), 1);
  EXPECT_EQ(patients[0].name, "Ana");
  EXPECT_EQ(patients[0].last_name, "Lopez");

  const int patient_id = patients[0].id;
  const auto details_result = patient_repository.getPatientDetailsByIdForUserName(patient_id, "a");
  ASSERT_TRUE(ROBOGait::db::statusOk(details_result));
  const auto details = std::get<ROBOGait::db::PatientDetails>(details_result);
  EXPECT_EQ(details.patient.id, patient_id);
  EXPECT_EQ(details.patient.description, "left knee");
  EXPECT_EQ(details.user_name, "a");
  EXPECT_EQ(details.user_last_name, "a");
  EXPECT_EQ(details.doctor_names, "a a");

  const auto doctors_result = patient_repository.listPatientDoctorsByPatientIdForUserName(patient_id, "a");
  ASSERT_TRUE(ROBOGait::db::statusOk(doctors_result));
  const QVector<ROBOGait::db::PatientDoctorInfo> doctors = std::get<QVector<ROBOGait::db::PatientDoctorInfo>>(doctors_result);
  ASSERT_EQ(doctors.size(), 1);
  EXPECT_EQ(doctors[0].doctor_name, "a");
  EXPECT_EQ(doctors[0].description, "left knee");

  ASSERT_TRUE(ROBOGait::db::statusOk(patient_repository.deletePatientByIdForUserName(patient_id, "a")));
  const auto deleted_details_result = patient_repository.getPatientDetailsByIdForUserName(patient_id, "a");
  EXPECT_FALSE(ROBOGait::db::statusOk(deleted_details_result));
  EXPECT_EQ(std::get<ROBOGait::db::DbError>(deleted_details_result).code, ROBOGait::db::DbErrorCode::NOT_FOUND);
}

TEST_F(RepositoryTest, MapRepositoryCreatesUpdatesAndDeletesMap)
{
  ROBOGait::db::MapRepository map_repository(db_);

  ASSERT_TRUE(ROBOGait::db::statusOk(map_repository.insertMapForUserName("a", "Lab", "Room 1", "first floor")));

  const auto names_result = map_repository.listMapNames();
  ASSERT_TRUE(ROBOGait::db::statusOk(names_result));
  const QVector<QString> names = std::get<QVector<QString>>(names_result);
  ASSERT_EQ(names.size(), 1);
  EXPECT_EQ(names[0], "Lab");

  ASSERT_TRUE(ROBOGait::db::statusOk(map_repository.setMapSaved("Lab", true)));
  const auto details_result = map_repository.getMapDetailsByName("Lab");
  ASSERT_TRUE(ROBOGait::db::statusOk(details_result));
  const auto details = std::get<ROBOGait::db::MapDetails>(details_result);
  EXPECT_EQ(details.map.name, "Lab");
  EXPECT_EQ(details.map.location, "Room 1");
  EXPECT_EQ(details.map.details, "first floor");
  EXPECT_TRUE(details.map.map_save);
  EXPECT_EQ(details.user_name, "a");
  EXPECT_EQ(details.user_last_name, "a");

  ASSERT_TRUE(ROBOGait::db::statusOk(map_repository.deleteMapByName("Lab")));
  const auto deleted_details_result = map_repository.getMapDetailsByName("Lab");
  EXPECT_FALSE(ROBOGait::db::statusOk(deleted_details_result));
  EXPECT_EQ(std::get<ROBOGait::db::DbError>(deleted_details_result).code, ROBOGait::db::DbErrorCode::NOT_FOUND);
}

TEST_F(RepositoryTest, ExperimentRepositoryCreatesListsAndDeletesExperiments)
{
  ROBOGait::db::PatientRepository patient_repository(db_);
  ROBOGait::db::MapRepository map_repository(db_);
  ROBOGait::db::ExperimentRepository experiment_repository(db_);

  ASSERT_TRUE(ROBOGait::db::statusOk(patient_repository.insertPatientForUserName("Ana", "Lopez", 34, 62.5, 1.68, "a", "left knee")));
  ASSERT_TRUE(ROBOGait::db::statusOk(map_repository.insertMapForUserName("a", "Lab", "Room 1", "first floor")));

  const auto patients_result = patient_repository.listPatientsForUserName("a");
  ASSERT_TRUE(ROBOGait::db::statusOk(patients_result));
  const QVector<ROBOGait::db::PatientRow> patients = std::get<QVector<ROBOGait::db::PatientRow>>(patients_result);
  ASSERT_EQ(patients.size(), 1);
  const int patient_id = patients[0].id;

  const auto experiment_id_result = experiment_repository.insertExperiment("Ana", "Lopez", "Lab", "a", QString("baseline"));
  ASSERT_TRUE(ROBOGait::db::statusOk(experiment_id_result));
  const int experiment_id = std::get<int>(experiment_id_result);
  EXPECT_GT(experiment_id, 0);

  const auto tests_result = experiment_repository.listPatientTestsByPatientIdForUserName(patient_id, "a");
  ASSERT_TRUE(ROBOGait::db::statusOk(tests_result));
  const QVector<ROBOGait::db::PatientTestInfo> tests = std::get<QVector<ROBOGait::db::PatientTestInfo>>(tests_result);
  ASSERT_EQ(tests.size(), 1);
  EXPECT_EQ(tests[0].experiment_id, experiment_id);
  EXPECT_EQ(tests[0].patient_name, "Ana");
  EXPECT_EQ(tests[0].patient_last_name, "Lopez");
  EXPECT_EQ(tests[0].doctor_name, "a");
  EXPECT_EQ(tests[0].doctor_last_name, "a");
  EXPECT_EQ(tests[0].map_name, "Lab");
  EXPECT_EQ(tests[0].map_location, "Room 1");
  EXPECT_EQ(tests[0].comment, "baseline");

  ASSERT_TRUE(ROBOGait::db::statusOk(experiment_repository.deleteExperimentByIdForUserName(experiment_id, "a")));
  const auto empty_tests_result = experiment_repository.listPatientTestsByPatientIdForUserName(patient_id, "a");
  ASSERT_TRUE(ROBOGait::db::statusOk(empty_tests_result));
  EXPECT_TRUE(std::get<QVector<ROBOGait::db::PatientTestInfo>>(empty_tests_result).empty());
}