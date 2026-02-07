#include "Core/RoboGaitApplication.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QQmlContext>
#include <QStandardPaths>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <geometry_msgs/msg/twist.hpp>

#include "Robot/ManualControl.hpp"
#include "Robot/RobotDiscovery.hpp"
#include "Robot/RobotManager.hpp"
#include "User/Patient.hpp"

using namespace ROBOGait::core;

RoboGaitApplication* RoboGaitApplication::app_instance_ = nullptr;

RoboGaitApplication::RoboGaitApplication(int& argc, char* argv[]) :
    QGuiApplication(argc, argv), ros_node_manager_(nullptr), user_session_(nullptr), qml_app_engine_(nullptr), translator_(), argc_(argc), argv_(argv)
{
  qInfo() << "************************ ROBOGait GUI ******************************";

  Q_ASSERT(app_instance_ == nullptr);
  app_instance_ = this;

  qInfo() << "[RoboGaitApplication::RoboGaitApplication] Application created";
}

RoboGaitApplication::~RoboGaitApplication()
{
  qInfo() << "******************* FINISH ROBOGait GUI *************************";

  if (qml_app_engine_)
  {
    delete qml_app_engine_;
    qml_app_engine_ = nullptr;
  }

  user_session_.reset();
  ros_node_manager_.reset();

  app_instance_ = nullptr;
}

void RoboGaitApplication::initCommon()
{
  qmlRegisterUncreatableType<ROBOGait::robot::discovery::RobotDiscovery>("RobotDiscovery", 1, 0, "RobotDiscovery", "Enums Only");
  qRegisterMetaType<geometry_msgs::msg::Twist>("geometry_msgs::msg::Twist");

  qInfo() << "[RoboGaitApplication::initCommon] QML types and metatypes registered";
}

void RoboGaitApplication::initialize()
{
  setupTranslator();
  setupDatabase();

  // Create ROS node manager
  ros_node_manager_ = std::make_unique<ROBOGait::ros::manager::RosNodeManager>();

  // Initialize ROS with command line arguments
  int argc = arguments().size();
  QList<QByteArray> args_byte_array;
  QList<char*> args_char_ptr;

  for (const QString& arg : arguments())
  {
    args_byte_array.append(arg.toLocal8Bit());
  }

  for (QByteArray& arg : args_byte_array)
  {
    args_char_ptr.append(arg.data());
  }

  ros_node_manager_->initialize(argc, args_char_ptr.data());

  // Create user session
  user_session_ = std::make_unique<ROBOGait::session::UserSession>(ros_node_manager_.get());

  // Initialize DeveloperSettings singleton
  auto& developer_settings = ROBOGait::settings::DeveloperSettings::getInstance();
  developer_settings.initializeDefaults();

  connectSignals();

  qInfo() << "[RoboGaitApplication::initialize] Application subsystems initialized";
}

bool RoboGaitApplication::initForNormalAppBoot()
{
  qml_app_engine_ = new QQmlApplicationEngine(this);

  setupQmlContext();

  const QUrl url(QStringLiteral("qrc:/main.qml"));

  QObject::connect(
      qml_app_engine_, &QQmlApplicationEngine::objectCreated, this,
      [url](QObject* obj, const QUrl& objUrl)
      {
        if (!obj && url == objUrl)
        {
          qCritical() << "[RoboGaitApplication::initForNormalAppBoot] Failed to load QML";
          QCoreApplication::exit(-1);
        }
      },
      Qt::QueuedConnection);

  qml_app_engine_->load(url);

  qInfo() << "[RoboGaitApplication::initForNormalAppBoot] QML engine initialized";
  return true;
}

ROBOGait::ros::manager::RosNodeManager* RoboGaitApplication::rosNodeManager() { return ros_node_manager_.get(); }

ROBOGait::db::DataBaseManager* RoboGaitApplication::databaseManager() { return &ROBOGait::db::DataBaseManager::getInstance(); }

ROBOGait::session::UserSession* RoboGaitApplication::userSession() { return user_session_.get(); }

ROBOGait::settings::DeveloperSettings* RoboGaitApplication::developerSettings() { return &ROBOGait::settings::DeveloperSettings::getInstance(); }

QQmlApplicationEngine* RoboGaitApplication::qmlEngine() { return qml_app_engine_; }

RoboGaitApplication* RoboGaitApplication::instance()
{
  Q_ASSERT(app_instance_ != nullptr);
  return app_instance_;
}

void RoboGaitApplication::setupTranslator()
{
  const QStringList ui_languages = QLocale::system().uiLanguages();

  for (const QString& locale : ui_languages)
  {
    const QString base_name = "gui_" + QLocale(locale).name();

    if (translator_.load(":/i18n/" + base_name))
    {
      installTranslator(&translator_);
      qInfo() << "[RoboGaitApplication::setupTranslator] Loaded translation:" << base_name;
      break;
    }
  }

  qInfo() << "[RoboGaitApplication::setupTranslator] Translator set up";
}

void RoboGaitApplication::setupDatabase()
{
  auto& database = ROBOGait::db::DataBaseManager::getInstance();

  // Get database template path from ROS package
  const QString db_path_template = QString::fromStdString(ament_index_cpp::get_package_share_directory("robogait_gui")) + "/database/db_robogait.db";

  // Get application data directory
  const QString app_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(app_dir);

  // Runtime database path
  const QString db_runtime = app_dir + "/db_robogait.db";

  // Check if we need to copy the template
  const bool needs_copy = !QFile::exists(db_runtime) || QFileInfo(db_runtime).size() == 0;

  if (needs_copy)
  {
    if (!QFile::exists(db_path_template))
    {
      qCritical() << "[RoboGaitApplication::setupDatabase] DB template not found at:" << db_path_template;
    }
    else
    {
      QFile::remove(db_runtime);

      if (!QFile::copy(db_path_template, db_runtime))
      {
        qCritical() << "[RoboGaitApplication::setupDatabase] Failed to copy DB template from" << db_path_template << "to" << db_runtime;
      }
      else
      {
        qInfo() << "[RoboGaitApplication::setupDatabase] Database template copied to:" << db_runtime;
      }
    }
  }

  // Initialize database
  database.initialize(db_runtime);
}

void RoboGaitApplication::setupQmlContext()
{
  Q_ASSERT(qml_app_engine_ != nullptr);

  qml_app_engine_->rootContext()->setContextProperty("userSession", user_session_.get());
  qml_app_engine_->rootContext()->setContextProperty("dbManager", databaseManager());
  qml_app_engine_->rootContext()->setContextProperty("developerSettings", developerSettings());

  qInfo() << "[RoboGaitApplication::setupQmlContext] QML context properties set";
}

void RoboGaitApplication::connectSignals()
{
  auto& developer_settings = ROBOGait::settings::DeveloperSettings::getInstance();

  // clang-format off
  // Connect DeveloperSettings signals for ROS domain ID changes
  connect(&developer_settings,
          &ROBOGait::settings::DeveloperSettings::settingsApplied,
          this,
          &RoboGaitApplication::onDeveloperSettingsApplied
  );
  // clang-format on
}

void RoboGaitApplication::onDeveloperSettingsApplied()
{

  auto& developer_settings = ROBOGait::settings::DeveloperSettings::getInstance();
  const uint32_t new_domain_id = developer_settings.getRosDomainId();

  if (ros_node_manager_ == nullptr)
  {
    qCritical() << "[RoboGaitApplication::onDeveloperSettingsApplied] RosNodeManager is null, cannot restart";
    return;
  }

  const bool restart_success = ros_node_manager_->restartWithDomain(new_domain_id, argc_, argv_);

  if (!restart_success)
  {
    qCritical() << "[RoboGaitApplication::onDeveloperSettingsApplied] Failed to restart ROS with domain ID:" << new_domain_id;
    return;
  }

  qInfo() << "[RoboGaitApplication::onDeveloperSettingsApplied] ROS restarted successfully with domain ID:" << new_domain_id;
}

ROBOGait::core::RoboGaitApplication* roboGaitApp() { return ROBOGait::core::RoboGaitApplication::instance(); }
