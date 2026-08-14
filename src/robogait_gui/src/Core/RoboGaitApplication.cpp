#include <filesystem>
#include <string>
#include <system_error>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QQmlContext>
#include <QStandardPaths>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <yaml-cpp/yaml.h>

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
    QApplication(argc, argv),
    ros_node_manager_(nullptr),
    user_session_(nullptr),
    qml_app_engine_(nullptr),
    translator_(),
    is_database_setup_(false),
    argc_(argc),
    argv_(argv)
{
  // Activate logs colors for better visibility in the console
  qSetMessagePattern("%{if-debug}\033[32m%{message}\033[0m%{endif}"
                     "%{if-info}\033[37m%{message}\033[0m%{endif}"
                     "%{if-warning}\033[33m%{message}\033[0m%{endif}"
                     "%{if-critical}\033[31m%{message}\033[0m%{endif}"
                     "%{if-fatal}\033[31m%{message}\033[0m%{endif}");

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

  if (is_database_setup_)
  {
    ROBOGait::db::DataBaseManager::getInstance().shutdown();
    is_database_setup_ = false;
  }

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
  // Load Boostrap YAML configuration
  const std::filesystem::path shared_dir = ament_index_cpp::get_package_share_directory(ROBOGait::ros::define::ROBOGAIT_GUI);
  const std::filesystem::path bootstrap_path = (shared_dir / "params" / "bootstrap.yaml");
  const auto bootstrap_config = getBootstrapConfig(QString::fromStdString(bootstrap_path));

  if (!bootstrap_config)
  {
    qCritical() << "[RoboGaitApplication::initialize] BootStrap config is not available";
    return false;
  }

  const QString shared_params_dir = QString::fromStdString((shared_dir / "params").string());

  if (!ensureUserConfigFiles(shared_params_dir, bootstrap_config.value()))
  {
    qCritical() << "[RoboGaitApplication::initialize] Failed to ensure user configuration files";
    return false;
  }

  const std::filesystem::path config_path =
      std::filesystem::path(QDir::homePath().toStdString()) / bootstrap_config->user_config_root_path / bootstrap_config->config_file_name;

  const auto db_file = getDatabaseFile(QString::fromStdString(config_path));

  if (!db_file)
  {
    qCritical() << "[RoboGaitApplication::initialize] Database file is not available";
    return false;
  }

  // Setup database
  if (!setupDatabase(db_file.value()))
  {
    qCritical() << "[RoboGaitApplication::initialize] Failed to setup database";
    return false;
  }

  // Setup application translator
  setupTranslator();

  // Initialize DeveloperSettings singleton
  auto& developer_settings = ROBOGait::settings::DeveloperSettings::getInstance();
  developer_settings.initializeDefaults();

  // Initialize UiSizingSettings singleton
  auto& ui_sizing_settings = ROBOGait::settings::UiSizingSettings::getInstance();
  ui_sizing_settings.initializeDefaults();

  // Initialize TimeoutSettings singleton
  auto& timeout_settings = ROBOGait::settings::TimeoutSettings::getInstance();
  timeout_settings.initializeDefaults();

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

  ros_node_manager_->initialize(argc, args_char_ptr.data(), static_cast<uint8_t>(developer_settings.getRosDomainId()));

  ros_node_manager_->setUseNamespaceDiscovery(developer_settings.getUseNamespaceDiscovery());
  ros_node_manager_->setUseTopicFilter(developer_settings.getUseTopicFilter());

  // Create user session
  user_session_ = std::make_unique<ROBOGait::session::UserSession>(ros_node_manager_.get());

  connectSignals();

  qInfo() << "[RoboGaitApplication::initialize] Application subsystems initialized";
  return true;
}

std::optional<ROBOGait::loader::BootStrapLoader::BootStrapConfig> RoboGaitApplication::getBootstrapConfig(const QString& config_path) const
{
  auto& bootstrap_loader = ROBOGait::loader::BootStrapLoader::getInstance();
  const auto bootstrap_result = bootstrap_loader.loadBootStrapConfig(std::filesystem::path(config_path.toStdString()));

  if (!bootstrap_result)
  {
    qCritical().noquote() << QString("[RoboGaitApplication::getBootstrapConfig] Failed to load bootstrap configuration from: %1\n%2")
                                 .arg(config_path, QString::fromStdString(bootstrap_result.error));
    return std::nullopt;
  }

  return bootstrap_result.config.value();
}

bool RoboGaitApplication::ensureUserConfigFiles(const QString& shared_params_dir,
                                                const ROBOGait::loader::BootStrapLoader::BootStrapConfig& bootstrap_config) const
{
  const std::filesystem::path user_config_dir = std::filesystem::path(QDir::homePath().toStdString()) / bootstrap_config.user_config_root_path;

  std::error_code error_code;
  const bool directory_created = std::filesystem::create_directories(user_config_dir, error_code);

  if (error_code)
  {
    qCritical() << "[RoboGaitApplication::ensureUserConfigFiles] Failed to create user configuration directory:"
                << QString::fromStdString(user_config_dir.string()) << QString::fromStdString(error_code.message());
    return false;
  }

  if (directory_created)
  {
    qInfo() << "[RoboGaitApplication::ensureUserConfigFiles] Created user configuration directory:" << QString::fromStdString(user_config_dir.string());
  }

  const std::filesystem::path shared_params_path(shared_params_dir.toStdString());
  const QString default_config_file = QString::fromStdString((shared_params_path / bootstrap_config.config_file_name).string());
  const QString user_config_file = QString::fromStdString((user_config_dir / bootstrap_config.config_file_name).string());

  if (!ensureUserConfigFile(default_config_file, user_config_file))
  {
    return false;
  }

  const QString default_commands_file = QString::fromStdString((shared_params_path / bootstrap_config.commands_file_name).string());
  const QString user_commands_file = QString::fromStdString((user_config_dir / bootstrap_config.commands_file_name).string());

  return ensureUserConfigFile(default_commands_file, user_commands_file);
}

bool RoboGaitApplication::ensureUserConfigFile(const QString& source_file, const QString& target_file) const
{
  if (QFileInfo::exists(target_file))
  {
    return true;
  }

  if (!QFileInfo::exists(source_file))
  {
    qCritical() << "[RoboGaitApplication::ensureUserConfigFile] Default configuration file does not exist:" << source_file;
    return false;
  }

  if (!QFile::copy(source_file, target_file))
  {
    qCritical() << "[RoboGaitApplication::ensureUserConfigFile] Failed to copy default configuration from:" << source_file << "to:" << target_file;
    return false;
  }

  qInfo() << "[RoboGaitApplication::ensureUserConfigFile] Copied default configuration to:" << target_file;
  return true;
}

std::optional<QString> RoboGaitApplication::getDatabaseFile(const QString& config_path) const
{
  if (!std::filesystem::exists(config_path.toStdString()))
  {
    qCritical() << "[RoboGaitApplication::getDatabaseFile] Config file does not exist:" << config_path;
    return std::nullopt;
  }

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  const auto yaml_result = yaml_loader.loadConfig(config_path.toStdString());

  if (!yaml_result)
  {
    qCritical().noquote() << QString("[RoboGaitApplication::getDatabaseFile] Failed to load config YAML configuration from: %1\n%2")
                                 .arg(config_path, QString::fromStdString(yaml_result.error));
    return std::nullopt;
  }

  // Get Database Configuration
  const std::string database_dir = yaml_loader.getValue<std::string>("database.path", ".local/robogait");
  const std::string database_filename = yaml_loader.getValue<std::string>("database.filename", "db_robogait.db");

  if (database_dir.empty())
  {
    qCritical() << "[RoboGaitApplication::getDatabaseFile] Database path not configured in YAML";
    return std::nullopt;
  }

  if (database_filename.empty())
  {
    qCritical() << "[RoboGaitApplication::getDatabaseFile] Database filename not configured in YAML";
    return std::nullopt;
  }

  const std::filesystem::path db_path = std::filesystem::path(QDir::homePath().toStdString()) / database_dir / database_filename;

  return QString::fromStdString(db_path);
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

ROBOGait::settings::TimeoutSettings* RoboGaitApplication::timeoutSettings() { return &ROBOGait::settings::TimeoutSettings::getInstance(); }

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

  is_database_setup_ = true;

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
  qml_app_engine_->rootContext()->setContextProperty("timeoutSettings", timeoutSettings());

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
  const auto new_domain_id = static_cast<uint8_t>(developer_settings.getRosDomainId());
  const bool use_namespace_discovery = developer_settings.getUseNamespaceDiscovery();
  const bool use_topic_filter = developer_settings.getUseTopicFilter();

  if (ros_node_manager_ == nullptr)
  {
    qCritical() << "[RoboGaitApplication::onDeveloperSettingsApplied] RosNodeManager is null, cannot restart";
    return;
  }

  ros_node_manager_->setUseNamespaceDiscovery(use_namespace_discovery);
  ros_node_manager_->setUseTopicFilter(use_topic_filter);

  const bool restart_success = ros_node_manager_->restartWithDomain(new_domain_id, argc_, argv_);

  if (!restart_success)
  {
    qCritical() << "[RoboGaitApplication::onDeveloperSettingsApplied] Failed to restart ROS with domain ID:" << new_domain_id;
    return;
  }
}

ROBOGait::core::RoboGaitApplication* roboGaitApp() { return ROBOGait::core::RoboGaitApplication::instance(); }
