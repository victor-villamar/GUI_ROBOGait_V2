#include <memory>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include <gtest/gtest.h>

#include "DataBase/DatabaseSchema.hpp"

namespace
{
class DatabaseSchemaTest : public ::testing::Test
{
protected:
  static void SetUpTestSuite()
  {
    if (QCoreApplication::instance() == nullptr)
    {
      static int argc = 1;
      static char app_name[] = "test_database_schema";
      static char* argv[] = {app_name, nullptr};
      app_ = std::make_unique<QCoreApplication>(argc, argv);
    }
  }

  static void TearDownTestSuite() { app_.reset(); }

  void SetUp() override
  {
    connection_name_ = QString("database_schema_test_%1").arg(reinterpret_cast<quintptr>(this));
    db_ = QSqlDatabase::addDatabase("QSQLITE", connection_name_);
    db_.setDatabaseName(":memory:");
    ASSERT_TRUE(db_.open());
  }

  void TearDown() override
  {
    db_.close();
    db_ = QSqlDatabase();
    QSqlDatabase::removeDatabase(connection_name_);
  }

  bool tableExists(const QString& table_name)
  {
    QSqlQuery query(db_);
    query.prepare("SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = :name");
    query.bindValue(":name", table_name);
    if (!query.exec() || !query.next())
    {
      return false;
    }
    return query.value(0).toInt() == 1;
  }

  bool indexExists(const QString& index_name)
  {
    QSqlQuery query(db_);
    query.prepare("SELECT COUNT(*) FROM sqlite_master WHERE type = 'index' AND name = :name");
    query.bindValue(":name", index_name);
    if (!query.exec() || !query.next())
    {
      return false;
    }
    return query.value(0).toInt() == 1;
  }

  bool columnExists(const QString& table_name, const QString& column_name)
  {
    QSqlQuery query(db_);
    if (!query.exec(QString("PRAGMA table_info(%1)").arg(table_name)))
    {
      return false;
    }

    while (query.next())
    {
      if (query.value(1).toString() == column_name)
      {
        return true;
      }
    }
    return false;
  }

  QString patientForeignKeyDeleteAction()
  {
    QSqlQuery query(db_);
    if (!query.exec("PRAGMA foreign_key_list(experiments)"))
    {
      return {};
    }

    while (query.next())
    {
      if (query.value(2).toString() == "patient" && query.value(3).toString() == "id_patient")
      {
        return query.value(6).toString();
      }
    }
    return {};
  }

  QString connection_name_;
  QSqlDatabase db_;

private:
  static std::unique_ptr<QCoreApplication> app_;
};

std::unique_ptr<QCoreApplication> DatabaseSchemaTest::app_;
} // namespace

TEST_F(DatabaseSchemaTest, InitializesTablesIndexesAndDefaultUsers)
{
  const auto result = ROBOGait::db::DatabaseSchema::initializeSchema(db_);

  ASSERT_TRUE(ROBOGait::db::statusOk(result));
  EXPECT_TRUE(tableExists("user"));
  EXPECT_TRUE(tableExists("patient"));
  EXPECT_TRUE(tableExists("map"));
  EXPECT_TRUE(tableExists("experiments"));
  EXPECT_TRUE(tableExists("patient_doctor"));
  EXPECT_TRUE(columnExists("map", "archived_at"));
  EXPECT_EQ(patientForeignKeyDeleteAction(), "CASCADE");
  EXPECT_TRUE(indexExists("idx_patient_doctor_doctor"));
  EXPECT_TRUE(indexExists("idx_patient_doctor_patient"));
  EXPECT_TRUE(indexExists("idx_experiments_patient"));
  EXPECT_TRUE(indexExists("idx_experiments_map"));

  QSqlQuery query(db_);
  ASSERT_TRUE(query.exec("SELECT username, role, password FROM \"user\" ORDER BY id"));
  ASSERT_TRUE(query.next());
  EXPECT_EQ(query.value(0).toString(), "a");
  EXPECT_EQ(query.value(1).toString(), "doctor");
  EXPECT_EQ(query.value(2).toString(), QString(QCryptographicHash::hash(QStringLiteral("a").toUtf8(), QCryptographicHash::Sha256).toHex()));
  ASSERT_TRUE(query.next());
  EXPECT_EQ(query.value(0).toString(), "admin");
  EXPECT_EQ(query.value(1).toString(), "manager");
}

TEST_F(DatabaseSchemaTest, InitializeSchemaIsIdempotent)
{
  ASSERT_TRUE(ROBOGait::db::statusOk(ROBOGait::db::DatabaseSchema::initializeSchema(db_)));
  ASSERT_TRUE(ROBOGait::db::statusOk(ROBOGait::db::DatabaseSchema::initializeSchema(db_)));

  QSqlQuery query(db_);
  ASSERT_TRUE(query.exec("SELECT COUNT(*) FROM \"user\""));
  ASSERT_TRUE(query.next());
  EXPECT_EQ(query.value(0).toInt(), 2);
}