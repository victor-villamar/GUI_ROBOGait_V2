#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QQmlContext>
#include <QStandardPaths>
#include <string>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <geometry_msgs/msg/twist.hpp>

#include "Core/RoboGaitApplication.hpp"
#include "Loader/YamlLoader.hpp"
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

bool RoboGaitApplication::initialize()
{
  // Load YAML configuration
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  const std::string config_path = ament_index_cpp::get_package_share_directory("robogait_gui") + "/params/config.yaml";

  if (!yaml_loader.loadConfig(config_path))
  {
    qCritical() << "[RoboGaitApplication::initialize] CRITICAL: Failed to load configuration from:" << QString::fromStdString(config_path);
    return false;
  }

  const std::string configured_path = yaml_loader.getValue<std::string>("database.path", "");

  if (configured_path.empty())
  {
    qCritical() << "[RoboGaitApplication::initialize] Database path not configured in YAML";
    return false;
  }

  // Setup application translator
  setupTranslator();

  // Setup database
  if (!setupDatabase(QString::fromStdString(configured_path)))
  {
    qCritical() << "[RoboGaitApplication::initialize] Failed to setup database";
    return false;
  }

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
  return true;
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

bool RoboGaitApplication::setupDatabase(const QString& db_path)
{
  auto& database = ROBOGait::db::DataBaseManager::getInstance();

  qInfo() << "[RoboGaitApplication::setupDatabase] Using database path from config:" << db_path;

  if (!database.initialize(db_path))
  {
    qCritical() << "[RoboGaitApplication::setupDatabase] Failed to initialize database at:" << db_path;
    return false;
  }

  qInfo() << "[RoboGaitApplication::setupDatabase] Database setup completed successfully";
  return true;
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
}

ROBOGait::core::RoboGaitApplication* roboGaitApp() { return ROBOGait::core::RoboGaitApplication::instance(); }
