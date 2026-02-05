#include <QDebug>

#include "Settings/DeveloperSettings.hpp"

using namespace ROBOGait::settings;

DeveloperSettings& DeveloperSettings::getInstance()
{
  static DeveloperSettings instance;
  return instance;
}

DeveloperSettings::DeveloperSettings() : current_use_namespace_discovery_(false), pending_use_namespace_discovery_(false)
{
  qInfo() << "[DeveloperSettings::DeveloperSettings] DeveloperSettings created";
}

bool DeveloperSettings::getUseNamespaceDiscovery() const { return pending_use_namespace_discovery_; }

bool DeveloperSettings::getHasPendingChanges() const { return pending_use_namespace_discovery_ != current_use_namespace_discovery_; }

void DeveloperSettings::setUseNamespaceDiscovery(bool value)
{
  if (pending_use_namespace_discovery_ != value)
  {
    pending_use_namespace_discovery_ = value;
    emit useNamespaceDiscoveryChanged();
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

  updatePendingChangesState();
  emit settingsApplied();

  return true;
}

void DeveloperSettings::resetChanges()
{
  bool had_pending = getHasPendingChanges();

  pending_use_namespace_discovery_ = current_use_namespace_discovery_;

  if (had_pending)
  {
    emit useNamespaceDiscoveryChanged();
    updatePendingChangesState();
    emit settingsReset();

    qInfo() << "[DeveloperSettings::resetChanges] Pending changes reset to current values";
  }
}

void DeveloperSettings::initializeDefaults()
{
  current_use_namespace_discovery_ = true;
  pending_use_namespace_discovery_ = true;

  emit useNamespaceDiscoveryChanged();
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