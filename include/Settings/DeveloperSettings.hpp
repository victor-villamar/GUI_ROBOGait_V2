#pragma once

#include <QObject>

namespace ROBOGait
{
namespace settings
{

/**
 * @brief Singleton class for managing developer settings
 *
 * This class maintains developer configuration settings independently of other components.
 * It stores both current and pending values
 */
class DeveloperSettings : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Get the singleton instance
   *
   * @return Reference to the singleton instance
   */
  static DeveloperSettings& getInstance();

  /**
   * @brief Delete copy constructor and assignment operator
   */
  DeveloperSettings(const DeveloperSettings&) = delete;
  DeveloperSettings& operator=(const DeveloperSettings&) = delete;

  // clang-format off
  Q_PROPERTY(bool useNamespaceDiscovery
             READ getUseNamespaceDiscovery
             WRITE setUseNamespaceDiscovery
             NOTIFY useNamespaceDiscoveryChanged)

  Q_PROPERTY(uint8_t rosDomainId
             READ getRosDomainId
             WRITE setRosDomainId
             NOTIFY rosDomainIdChanged)

  Q_PROPERTY(bool useTopicFilter
             READ getUseTopicFilter
             WRITE setUseTopicFilter
             NOTIFY useTopicFilterChanged)

  Q_PROPERTY(bool hasPendingChanges
             READ getHasPendingChanges
             NOTIFY hasPendingChangesChanged)
  // clang-format on

  /**
   * @brief Get current pending value for namespace discovery
   *
   * @return True if namespace discovery should be enabled
   */
  bool getUseNamespaceDiscovery() const;

  /**
   * @brief Get current pending value for ROS domain ID
   *
   * @return ROS domain ID (0-232)
   */
  uint8_t getRosDomainId() const;

  /**
   * @brief Get current pending value for topic filter
   *
   * @return True if topic filter should be enabled
   */
  bool getUseTopicFilter() const;

  /**
   * @brief Check if there are pending changes to apply
   *
   * @return True if there are unapplied changes
   */
  bool getHasPendingChanges() const;

  /**
   * @brief Set pending value for namespace discovery
   *
   * @param value New pending value
   */
  void setUseNamespaceDiscovery(bool value);

  /**
   * @brief Set pending value for ROS domain ID
   *
   * @param domain_id New pending domain ID (0-232)
   */
  void setRosDomainId(uint8_t domain_id);

  /**
   * @brief Set pending value for topic filter
   *
   * @param value New pending value
   */
  void setUseTopicFilter(bool value);

public slots:
  /**
   * @brief Mark pending changes as applied (update current values)
   *
   * @return True if changes were marked as applied successfully
   */
  bool applyChanges();

  /**
   * @brief Reset pending changes to current values
   */
  void resetChanges();

  /**
   * @brief Initialize settings with default values
   */
  void initializeDefaults();

signals:
  void useNamespaceDiscoveryChanged();
  void rosDomainIdChanged();
  void useTopicFilterChanged();
  void hasPendingChangesChanged();
  void settingsApplied();
  void settingsReset();

private:
  /**
   * @brief Private constructor for Singleton pattern
   */
  DeveloperSettings();

  /**
   * @brief Destructor of DeveloperSettings class
   */
  ~DeveloperSettings() override = default;

  /**
   * @brief Update the pending changes state and emit signal if needed
   */
  void updatePendingChangesState();

  bool current_use_namespace_discovery_; /**< Current namespace discovery setting */
  bool pending_use_namespace_discovery_; /**< Pending namespace discovery setting */
  uint8_t current_ros_domain_id_;        /**< Current ROS domain ID */
  uint8_t pending_ros_domain_id_;        /**< Pending ROS domain ID */
  bool current_use_topic_filter_;        /**< Current topic filter setting */
  bool pending_use_topic_filter_;        /**< Pending topic filter setting */
};

} // namespace settings
} // namespace ROBOGait
