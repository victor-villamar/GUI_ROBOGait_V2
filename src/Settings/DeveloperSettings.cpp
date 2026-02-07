#include <QDebug>

#include "Settings/DeveloperSettings.hpp"

using namespace ROBOGait::settings;

DeveloperSettings& DeveloperSettings::getInstance()
{
  static DeveloperSettings instance;
  return instance;
}

DeveloperSettings::DeveloperSettings() :
    current_use_namespace_discovery_(false), pending_use_namespace_discovery_(false), current_ros_domain_id_(0), pending_ros_domain_id_(0)
{
  qInfo() << "[DeveloperSettings::DeveloperSettings] DeveloperSettings created";
}

bool DeveloperSettings::getUseNamespaceDiscovery() const { return pending_use_namespace_discovery_; }

uint32_t DeveloperSettings::getRosDomainId() const { return pending_ros_domain_id_; }

bool DeveloperSettings::getHasPendingChanges() const
{
  return (pending_use_namespace_discovery_ != current_use_namespace_discovery_) || (pending_ros_domain_id_ != current_ros_domain_id_);
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

void DeveloperSettings::setRosDomainId(uint32_t domain_id)
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

bool DeveloperSettings::applyChanges()
{
  if (!getHasPendingChanges())
  {
    qInfo() << "[DeveloperSettings::applyChanges] No pending changes to apply";
    return true;
  }

  current_use_namespace_discovery_ = pending_use_namespace_discovery_;
  current_ros_domain_id_ = pending_ros_domain_id_;

  updatePendingChangesState();
  emit settingsApplied();

  qInfo() << "[DeveloperSettings::applyChanges] Changes applied - Domain ID:" << current_ros_domain_id_
          << "Namespace Discovery:" << current_use_namespace_discovery_;

  return true;
}

void DeveloperSettings::resetChanges()
{
  bool had_pending = getHasPendingChanges();

  pending_use_namespace_discovery_ = current_use_namespace_discovery_;
  pending_ros_domain_id_ = current_ros_domain_id_;

  if (had_pending)
  {
    emit useNamespaceDiscoveryChanged();
    emit rosDomainIdChanged();
    updatePendingChangesState();
    emit settingsReset();

    qInfo() << "[DeveloperSettings::resetChanges] Pending changes reset to current values";
  }
}

void DeveloperSettings::initializeDefaults()
{
  current_use_namespace_discovery_ = true;
  pending_use_namespace_discovery_ = true;
  current_ros_domain_id_ = 0;
  pending_ros_domain_id_ = 0;

  emit useNamespaceDiscoveryChanged();
  emit rosDomainIdChanged();
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