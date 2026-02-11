#include <QDebug>

#include "Loader/YamlLoader.hpp"
#include "Settings/DeveloperSettings.hpp"

using namespace ROBOGait::settings;

DeveloperSettings& DeveloperSettings::getInstance()
{
  static DeveloperSettings instance;
  return instance;
}

DeveloperSettings::DeveloperSettings() :
    current_use_namespace_discovery_(false),
    pending_use_namespace_discovery_(false),
    current_ros_domain_id_(0),
    pending_ros_domain_id_(0),
    current_use_topic_filter_(true),
    pending_use_topic_filter_(true)
{
  qInfo() << "[DeveloperSettings::DeveloperSettings] DeveloperSettings created";
}

bool DeveloperSettings::getUseNamespaceDiscovery() const { return pending_use_namespace_discovery_; }

uint8_t DeveloperSettings::getRosDomainId() const { return pending_ros_domain_id_; }

bool DeveloperSettings::getUseTopicFilter() const { return pending_use_topic_filter_; }

bool DeveloperSettings::getHasPendingChanges() const
{
  return (pending_use_namespace_discovery_ != current_use_namespace_discovery_) || (pending_ros_domain_id_ != current_ros_domain_id_) ||
         (pending_use_topic_filter_ != current_use_topic_filter_);
}

void DeveloperSettings::setUseNamespaceDiscovery(bool value)
{
  if (pending_use_namespace_discovery_ != value)
  {
    pending_use_namespace_discovery_ = value;
    emit useNamespaceDiscoveryChanged();
    updatePendingChangesState();
  }
}

void DeveloperSettings::setRosDomainId(uint8_t domain_id)
{
  if (domain_id > 232)
  {
    qWarning() << "[DeveloperSettings::setRosDomainId] Invalid domain ID:" << domain_id << "(must be 0-232)";
    return;
  }

  if (pending_ros_domain_id_ != domain_id)
  {
    pending_ros_domain_id_ = domain_id;
    emit rosDomainIdChanged();
    updatePendingChangesState();
  }
}

void DeveloperSettings::setUseTopicFilter(bool value)
{
  if (pending_use_topic_filter_ != value)
  {
    pending_use_topic_filter_ = value;
    emit useTopicFilterChanged();
    updatePendingChangesState();
  }
}

bool DeveloperSettings::applyChanges()
{
  if (!getHasPendingChanges())
  {
    qInfo() << "[DeveloperSettings::applyChanges] No pending changes to apply";
    return true;
  }

  current_use_namespace_discovery_ = pending_use_namespace_discovery_;
  current_ros_domain_id_ = pending_ros_domain_id_;
  current_use_topic_filter_ = pending_use_topic_filter_;

  updatePendingChangesState();
  emit settingsApplied();

  qInfo() << "[DeveloperSettings::applyChanges] Changes applied - Domain ID:" << current_ros_domain_id_
          << "Namespace Discovery:" << current_use_namespace_discovery_ << "Topic Filter:" << current_use_topic_filter_;

  return true;
}

void DeveloperSettings::resetChanges()
{
  bool had_pending = getHasPendingChanges();

  pending_use_namespace_discovery_ = current_use_namespace_discovery_;
  pending_ros_domain_id_ = current_ros_domain_id_;
  pending_use_topic_filter_ = current_use_topic_filter_;

  if (had_pending)
  {
    emit useNamespaceDiscoveryChanged();
    emit rosDomainIdChanged();
    emit useTopicFilterChanged();
    updatePendingChangesState();
    emit settingsReset();

    qInfo() << "[DeveloperSettings::resetChanges] Pending changes reset to current values";
  }
}

void DeveloperSettings::initializeDefaults()
{

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qWarning() << "[DeveloperSettings::initializeDefaults] YAML configuration not loaded, using defaults";
    return;
  }

  uint8_t ros_domain_id = yaml_loader.getValue<uint8_t>("ros2.domain_id", 0);
  bool use_namespace_discovery = yaml_loader.getValue<bool>("ros2.use_namespace_discovery", true);
  bool use_topic_filter = yaml_loader.getValue<bool>("ros2.use_topic_filter", true);

  if (ros_domain_id > 232)
  {
    qWarning() << "[DeveloperSettings::initializeDefaults] Invalid ROS domain ID:" << ros_domain_id << "(must be 0-232), setting to 0";
    ros_domain_id = 0;
  }

  current_use_namespace_discovery_ = use_namespace_discovery;
  pending_use_namespace_discovery_ = use_namespace_discovery;
  current_ros_domain_id_ = ros_domain_id;
  pending_ros_domain_id_ = ros_domain_id;
  current_use_topic_filter_ = use_topic_filter;
  pending_use_topic_filter_ = use_topic_filter;

  emit useNamespaceDiscoveryChanged();
  emit rosDomainIdChanged();
  emit useTopicFilterChanged();
  updatePendingChangesState();

  qInfo() << "[DeveloperSettings::initializeDefaults] Settings initialized with defaults";
}

void DeveloperSettings::updatePendingChangesState()
{
  static bool last_pending_state = false;
  bool current_pending_state = getHasPendingChanges();

  if (last_pending_state != current_pending_state)
  {
    last_pending_state = current_pending_state;
    emit hasPendingChangesChanged();
  }
}