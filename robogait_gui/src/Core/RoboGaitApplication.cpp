#include <string>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QQmlContext>
#include <QStandardPaths>

#include <ament_index_cpp/get_package_share_directory.hpp>

#include <geometry_msgs/msg/twist.hpp>

#include "Core/RoboGaitApplication.hpp"
#include "Loader/YamlLoader.hpp"
#include "Map/Items/LaserLayerItem.hpp"
#include "Map/Items/MapLayerItem.hpp"
#include "Map/Items/ParticleCloudLayerItem.hpp"
#include "Map/Items/PathLayerItem.hpp"
#include "Map/Items/RobotLayerItem.hpp"
#include "Robot/RobotDiscovery.hpp"
#include "Ros/Define.hpp"
#include "Themes/AppTheme.hpp"

using namespace ROBOGait::core;

RoboGaitApplication* RoboGaitApplication::app_instance_ = nullptr;

RoboGaitApplication::RoboGaitApplication(int& argc, char* argv[]) :
    QApplication(argc, argv), ros_node_manager_(nullptr), user_session_(nullptr), qml_app_engine_(nullptr), translator_(), argc_(argc), argv_(argv)
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
  qmlRegisterUncreatableType<ROBOGait::qml::service::RobotServiceBridge>("RobotServiceBridge", 1, 0, "RobotServiceBridge", "Enums Only");

  qmlRegisterType<ROBOGait::map::item::MapLayerItem>("MapRendering", 1, 0, "MapLayerItem");
  qmlRegisterType<ROBOGait::map::item::RobotLayerItem>("MapRendering", 1, 0, "RobotLayerItem");
  qmlRegisterType<ROBOGait::map::item::LaserLayerItem>("MapRendering", 1, 0, "LaserLayerItem");
  qmlRegisterType<ROBOGait::map::item::ParticleCloudLayerItem>("MapRendering", 1, 0, "ParticleCloudLayerItem");
  qmlRegisterType<ROBOGait::map::item::PathLayerItem>("MapRendering", 1, 0, "PathLayerItem");

  qmlRegisterSingletonInstance("AppTheme", 1, 0, "AppTheme", &ROBOGait::settings::AppTheme::getInstance());

  qRegisterMetaType<geometry_msgs::msg::Twist>("geometry_msgs::msg::Twist");

  qInfo() << "[RoboGaitApplication::initCommon] QML types and metatypes registered";
}

bool RoboGaitApplication::initialize()
{
  // Load YAML configuration
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  const std::string config_path = ament_index_cpp::get_package_share_directory(ROBOGait::ros::define::ROBOGAIT_GUI) + "/params/config.yaml";

  if (!yaml_loader.loadConfig(config_path))
  {
    qCritical() << "[RoboGaitApplication::initialize] CRITICAL: Failed to load configuration from:" << QString::fromStdString(config_path);
    return false;
  }

  // Database Configuration
  const std::string configured_dir = yaml_loader.getValue<std::string>("database.path", ".local/default");
  const std::string configured_filename = yaml_loader.getValue<std::string>("database.filename", "default.db");

  if (configured_dir.empty())
  {
    qCritical() << "[RoboGaitApplication::initialize] Database path not configured in YAML";
    return false;
  }

  if (configured_filename.empty())
  {
    qCritical() << "[RoboGaitApplication::initialize] Database filename not configured in YAML";
    return false;
  }

  const QString full_db_path = QDir::homePath() + "/" + QString::fromStdString(configured_dir) + "/" + QString::fromStdString(configured_filename);

  // Setup application translator
  setupTranslator();

  // Setup database
  if (!setupDatabase(full_db_path))
  {
    qCritical() << "[RoboGaitApplication::initialize] Failed to setup database";
    return false;
  }

  // Initialize DeveloperSettings singleton
  auto& developer_settings = ROBOGait::settings::DeveloperSettings::getInstance();
  developer_settings.initializeDefaults();

  // Initialize UiSizingSettings singleton
  auto& ui_sizing_settings = ROBOGait::settings::UiSizingSettings::getInstance();
  ui_sizing_settings.initializeDefaults();

  // Create ROS node manager
  ros_node_manager_ = std::make_unique<ROBOGait::ros::manager::RosNodeManager>();

  // Initialize ROS with command line arguments
  int argc = static_cast<int>(arguments().size());
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

  ros_node_manager_->initialize(argc, args_char_ptr.data(), developer_settings.getRosDomainId());

  ros_node_manager_->setUseNamespaceDiscovery(developer_settings.getUseNamespaceDiscovery());
  ros_node_manager_->setUseTopicFilter(developer_settings.getUseTopicFilter());

  // Create user session
  user_session_ = std::make_unique<ROBOGait::session::UserSession>(ros_node_manager_.get());

  connectSignals();

  qInfo() << "[RoboGaitApplication::initialize] Application subsystems initialized";
  return true;
}

bool RoboGaitApplication::initForNormalAppBoot()
{
  // Needed to avoid cross-thread QBasicTimer
  qputenv("QSG_RENDER_LOOP", "basic");

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

ROBOGait::settings::UiSizingSettings* RoboGaitApplication::uiSizingSettings() { return &ROBOGait::settings::UiSizingSettings::getInstance(); }

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
  QFileInfo file_info(db_path);
  const QString dir_path = file_info.absolutePath();

  // Check if directory exists
  QDir db_dir(dir_path);
  if (!db_dir.exists())
  {
    qInfo() << "[RoboGaitApplication::setupDatabase] Directory does not exist, creating:" << dir_path;

    if (!QDir().mkpath(dir_path))
    {
      qCritical() << "[RoboGaitApplication::setupDatabase] Failed to create directory:" << dir_path;
      return false;
    }

    qInfo() << "[RoboGaitApplication::setupDatabase] Directory created successfully";
  }

  QFileInfo dir_info(dir_path);
  if (!dir_info.isWritable())
  {
    qWarning() << "[RoboGaitApplication::setupDatabase] Directory is not writable:" << dir_path;
    qWarning() << "[RoboGaitApplication::setupDatabase] This may cause database initialization to fail";
  }

  // Initialize database
  auto& database = ROBOGait::db::DataBaseManager::getInstance();

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
  qml_app_engine_->rootContext()->setContextProperty("uiSizingSettings", uiSizingSettings());

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
  const uint8_t new_domain_id = developer_settings.getRosDomainId();
  const bool use_topic_filter = developer_settings.getUseTopicFilter();

  if (ros_node_manager_ == nullptr)
  {
    qCritical() << "[RoboGaitApplication::onDeveloperSettingsApplied] RosNodeManager is null, cannot restart";
    return;
  }

  ros_node_manager_->setUseTopicFilter(use_topic_filter);

  const bool restart_success = ros_node_manager_->restartWithDomain(new_domain_id, argc_, argv_);

  if (!restart_success)
  {
    qCritical() << "[RoboGaitApplication::onDeveloperSettingsApplied] Failed to restart ROS with domain ID:" << new_domain_id;
    return;
  }
}

ROBOGait::core::RoboGaitApplication* roboGaitApp() { return ROBOGait::core::RoboGaitApplication::instance(); }
