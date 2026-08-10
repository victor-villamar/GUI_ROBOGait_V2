#include <filesystem>
#include <memory>

#include <QCoreApplication>
#include <QVariantList>
#include <QVariantMap>

#include <gtest/gtest.h>

#include "DataBase/DataBaseManager.hpp"

namespace
{
class DataBaseManagerTest : public ::testing::Test
{
protected:
  static void SetUpTestSuite()
  {
    if (QCoreApplication::instance() == nullptr)
    {
      static int argc = 1;
      static char app_name[] = "test_database_manager";
      static char* argv[] = {app_name, nullptr};
      app_ = std::make_unique<QCoreApplication>(argc, argv);
    }
  }

  static void TearDownTestSuite() { app_.reset(); }

  void SetUp() override
  {
    db_path_ = std::filesystem::temp_directory_path() / ("robogait_gui_database_manager_test_" + std::to_string(test_index_) + ".db");
    ++test_index_;
    std::filesystem::remove(db_path_);

    auto& manager = ROBOGait::db::DataBaseManager::getInstance();
    manager.logout();
    manager.shutdown();
  }

  void TearDown() override
  {
    auto& manager = ROBOGait::db::DataBaseManager::getInstance();
    manager.logout();
    manager.shutdown();
    std::filesystem::remove(db_path_);
  }

  QString databasePath() const { return QString::fromStdString(db_path_.string()); }

private:
  static std::unique_ptr<QCoreApplication> app_;
  static int test_index_;
  std::filesystem::path db_path_;
};

std::unique_ptr<QCoreApplication> DataBaseManagerTest::app_;
int DataBaseManagerTest::test_index_ = 0;
} // namespace

TEST_F(DataBaseManagerTest, ManagesUserPatientMapAndExperimentFlowWithTemporaryDatabase)
{
  auto& manager = ROBOGait::db::DataBaseManager::getInstance();

  ASSERT_TRUE(manager.initialize(databasePath()));
  EXPECT_TRUE(manager.isInitialized());
  EXPECT_TRUE(std::filesystem::exists(databasePath().toStdString()));

  EXPECT_FALSE(manager.checkUserNameAvailable("a"));
  EXPECT_FALSE(manager.getPassCheckUserName());
  EXPECT_TRUE(manager.checkUserNameAvailable("doctor_test"));
  EXPECT_TRUE(manager.getPassCheckUserName());

  ASSERT_TRUE(manager.registerUser("Doctor", "Test", "doctor_test", "secret", "doctor"));
  EXPECT_FALSE(manager.registerUser("Doctor", "Test", "doctor_test", "secret", "doctor"));
  EXPECT_FALSE(manager.getLastError().isEmpty());

  ASSERT_TRUE(manager.login("doctor_test", "secret"));
  EXPECT_TRUE(manager.getPassLogin());
  EXPECT_GT(manager.getUserId(), 0);
  EXPECT_EQ(manager.getUserName(), "doctor_test");
  EXPECT_EQ(manager.getDisplayName(), "Doctor");
  EXPECT_EQ(manager.getUserRole(), "doctor");
  EXPECT_TRUE(manager.getLastError().isEmpty());

  ASSERT_TRUE(manager.registerPatient("Ana", "Lopez", 34, 62.5, 168.0, "left knee"));
  const QVariantList patients = manager.listPatients();
  ASSERT_EQ(patients.size(), 1);

  const QVariantMap patient = patients[0].toMap();
  const int patient_id = patient["id"].toInt();
  EXPECT_GT(patient_id, 0);
  EXPECT_EQ(patient["name"].toString(), "Ana");
  EXPECT_EQ(patient["last_name"].toString(), "Lopez");
  EXPECT_EQ(patient["display"].toString(), "Lopez, Ana");

  const QVariantMap patient_details = manager.getPatientDetails(patient_id);
  EXPECT_EQ(patient_details["first_name"].toString(), "Ana");
  EXPECT_EQ(patient_details["last_name"].toString(), "Lopez");
  EXPECT_EQ(patient_details["description"].toString(), "left knee");
  EXPECT_EQ(patient_details["doctor_names"].toString(), "Doctor Test");

  ASSERT_TRUE(manager.registerMap("Lab", "Room 1", "first floor"));
  EXPECT_TRUE(manager.mapExists("Lab"));

  const QVariantList maps = manager.listMaps();
  ASSERT_EQ(maps.size(), 1);
  EXPECT_EQ(maps[0].toMap()["map_name"].toString(), "Lab");

  const QVariantMap map_details = manager.getMapDetails("Lab");
  EXPECT_EQ(map_details["map_name"].toString(), "Lab");
  EXPECT_EQ(map_details["location"].toString(), "Room 1");
  EXPECT_EQ(map_details["details"].toString(), "first floor");
  EXPECT_FALSE(map_details["map_save"].toBool());

  ASSERT_TRUE(manager.saveExperiment("Ana", "Lopez", "Lab", "baseline"));
  const int experiment_id = manager.getLastExperimentId();
  EXPECT_GT(experiment_id, 0);

  const QVariantList tests = manager.getPatientTests(patient_id);
  ASSERT_EQ(tests.size(), 1);
  EXPECT_EQ(tests[0].toMap()["experiment_id"].toInt(), experiment_id);
  EXPECT_EQ(tests[0].toMap()["map_name"].toString(), "Lab");
  EXPECT_EQ(tests[0].toMap()["comment"].toString(), "baseline");

  EXPECT_TRUE(manager.deleteExperiment(experiment_id));
  EXPECT_TRUE(manager.deleteMap("Lab"));
  EXPECT_FALSE(manager.mapExists("Lab"));
  EXPECT_TRUE(manager.deletePatient(patient_id));
  EXPECT_TRUE(manager.listPatients().isEmpty());
}

TEST_F(DataBaseManagerTest, GuestAndInvalidOperationsReturnErrors)
{
  auto& manager = ROBOGait::db::DataBaseManager::getInstance();

  ASSERT_TRUE(manager.initialize(databasePath()));

  EXPECT_FALSE(manager.login("missing", "wrong"));
  EXPECT_FALSE(manager.getPassLogin());
  EXPECT_EQ(manager.getLastError(), "Credenciales invalidas");

  manager.loginGuest("Guest");
  EXPECT_TRUE(manager.getPassLogin());
  EXPECT_EQ(manager.getDisplayName(), "Guest");
  EXPECT_EQ(manager.getUserRole(), "guest");

  EXPECT_FALSE(manager.registerPatient("Ana", "Lopez", 34, 62.5, 168.0, "left knee"));
  EXPECT_EQ(manager.getLastError(), "Accion no permitida para usuario invitado");

  EXPECT_FALSE(manager.registerMap("Lab", "Room 1", "first floor"));
  EXPECT_EQ(manager.getLastError(), "Accion no permitida para usuario invitado");

  EXPECT_TRUE(manager.listPatients().isEmpty());
  EXPECT_EQ(manager.getLastError(), "Accion no permitida para usuario invitado");
}