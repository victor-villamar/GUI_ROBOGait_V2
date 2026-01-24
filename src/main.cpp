#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMetaType>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <geometry_msgs/msg/twist.hpp>

#include "DataBase/DataBaseManager.hpp"
#include "RobotDiscovery.hpp"
#include "RobotManager.hpp"
#include "RosNodeManager.hpp"

#include <QLocale>
#include <QTranslator>

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString& locale : uiLanguages)
  {
    const QString baseName = "gui_" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName))
    {
      app.installTranslator(&translator);
      break;
    }
  }

  // ROS manager
  ROBOGait::manager::RosNodeManager rosNodeManager;
  rosNodeManager.initialize(argc, argv);

  // Database
  ROBOGait::db::DataBaseManager database;
  const QString db_path_template = QString::fromStdString(ament_index_cpp::get_package_share_directory("robogait_gui")) + "/database/db_robogait.db";

  const QString app_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(app_dir);

  const QString db_runtime = app_dir + "/db_robogait.db";

  const bool needs_copy = !QFile::exists(db_runtime) || QFileInfo(db_runtime).size() == 0;
  if (needs_copy)
  {
    if (!QFile::exists(db_path_template))
    {
      qCritical() << "[main] DB template not found at:" << db_path_template;
    }
    else
    {
      QFile::remove(db_runtime);
      if (!QFile::copy(db_path_template, db_runtime))
      {
        qCritical() << "[main] Failed to copy DB template from" << db_path_template << "to" << db_runtime;
      }
    }
  }

  database.openDatabase(db_runtime);

  qmlRegisterUncreatableType<ROBOGait::discovery::RobotDiscovery>("RobotDiscovery", 1, 0, "RobotDiscovery", "Enums Only");
  qRegisterMetaType<geometry_msgs::msg::Twist>("geometry_msgs::msg::Twist");

  QQmlApplicationEngine engine;

  engine.rootContext()->setContextProperty("rosManager", &rosNodeManager);
  engine.rootContext()->setContextProperty("dbManager", &database);

  const QUrl url(QStringLiteral("qrc:/main.qml"));
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject* obj, const QUrl& objUrl)
      {
        if (!obj && url == objUrl)
          QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);
  engine.load(url);

  return app.exec();
}
