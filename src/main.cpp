#include <QGuiApplication>
#include <QMetaType>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <geometry_msgs/msg/twist.hpp>

#include "RobotDiscovery.hpp"
#include "RobotManager.hpp"
#include "RosNodeManager.hpp"

#include <QLocale>
#include <QTranslator>
// #include "../include/Manage.h"

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

  qmlRegisterUncreatableType<ROBOGait::discovery::RobotDiscovery>("RobotDiscovery", 1, 0, "RobotDiscovery", "Enums Only");
  qRegisterMetaType<geometry_msgs::msg::Twist>("geometry_msgs::msg::Twist");

  QQmlApplicationEngine engine;

  engine.rootContext()->setContextProperty("rosManager", &rosNodeManager);

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
