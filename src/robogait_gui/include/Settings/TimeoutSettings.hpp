#pragma once

#include <QObject>

namespace ROBOGait
{
namespace settings
{

/**
 * @brief Singleton class for managing UI timeout settings
 *
 * This class centralizes timeout and timer values used by QML views/components.
 * Values are loaded from YAML and exposed to QML through readonly properties.
 */
class TimeoutSettings : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Get the singleton instance
   *
   * @return Reference to the singleton instance
   */
  static TimeoutSettings& getInstance();

  /**
   * @brief Delete copy constructor and assignment operator
   */
  TimeoutSettings(const TimeoutSettings&) = delete;
  TimeoutSettings& operator=(const TimeoutSettings&) = delete;

  // clang-format off
  Q_PROPERTY(int appShutdownDelayMs
             READ getAppShutdownDelayMs
             NOTIFY timeoutsChanged)

  Q_PROPERTY(int authModeDropdownAutoCloseMs
             READ getAuthModeDropdownAutoCloseMs
             NOTIFY timeoutsChanged)

  Q_PROPERTY(int registerDefaultIdleTimeoutMs
             READ getRegisterDefaultIdleTimeoutMs
             NOTIFY timeoutsChanged)

  Q_PROPERTY(int registerSignInIdleTimeoutMs
             READ getRegisterSignInIdleTimeoutMs
             NOTIFY timeoutsChanged)

  Q_PROPERTY(int badgeAutoCloseMs
             READ getBadgeAutoCloseMs
             NOTIFY timeoutsChanged)

  Q_PROPERTY(int robotBadgeAutoCloseMs
             READ getRobotBadgeAutoCloseMs
             NOTIFY timeoutsChanged)

  Q_PROPERTY(int errorRectangleDelayedCloseMs
             READ getErrorRectangleDelayedCloseMs
             NOTIFY timeoutsChanged)

  Q_PROPERTY(int testBusyTimeoutMs
             READ getTestBusyTimeoutMs
             NOTIFY timeoutsChanged)

  Q_PROPERTY(int autoLocalizationSpinTimeoutMs
             READ getAutoLocalizationSpinTimeoutMs
             NOTIFY timeoutsChanged)

  Q_PROPERTY(int autoLocalizationSpinPublishPeriodMs
             READ getAutoLocalizationSpinPublishPeriodMs
             NOTIFY timeoutsChanged)

  Q_PROPERTY(int experimentSavePromptDelayMs
             READ getExperimentSavePromptDelayMs
             NOTIFY timeoutsChanged)
  // clang-format on

  int getAppShutdownDelayMs() const;
  int getAuthModeDropdownAutoCloseMs() const;
  int getRegisterDefaultIdleTimeoutMs() const;
  int getRegisterSignInIdleTimeoutMs() const;
  int getBadgeAutoCloseMs() const;
  int getRobotBadgeAutoCloseMs() const;
  int getErrorRectangleDelayedCloseMs() const;
  int getTestBusyTimeoutMs() const;
  int getAutoLocalizationSpinTimeoutMs() const;
  int getAutoLocalizationSpinPublishPeriodMs() const;
  int getExperimentSavePromptDelayMs() const;

  /**
   * @brief Initialize timeout values from YAML with fallback to defaults
   */
  void initializeDefaults();

signals:
  void timeoutsChanged();

private:
  /**
   * @brief Private constructor for Singleton pattern
   */
  TimeoutSettings();

  /**
   * @brief Destructor of TimeoutSettings class
   */
  ~TimeoutSettings() override = default;

  int sanitizePositive(int value, int fallback) const;

  int app_shutdown_delay_ms_;
  int auth_mode_dropdown_auto_close_ms_;
  int register_default_idle_timeout_ms_;
  int register_sign_in_idle_timeout_ms_;
  int badge_auto_close_ms_;
  int robot_badge_auto_close_ms_;
  int error_rectangle_delayed_close_ms_;
  int test_busy_timeout_ms_;
  int auto_localization_spin_timeout_ms_;
  int auto_localization_spin_publish_period_ms_;
  int experiment_save_prompt_delay_ms_;

  static constexpr int DEFAULT_APP_SHUTDOWN_DELAY_MS = 3000;
  static constexpr int DEFAULT_AUTH_MODE_DROPDOWN_AUTO_CLOSE_MS = 4000;
  static constexpr int DEFAULT_REGISTER_DEFAULT_IDLE_TIMEOUT_MS = 10000;
  static constexpr int DEFAULT_REGISTER_SIGN_IN_IDLE_TIMEOUT_MS = 60000;
  static constexpr int DEFAULT_BADGE_AUTO_CLOSE_MS = 3000;
  static constexpr int DEFAULT_ROBOT_BADGE_AUTO_CLOSE_MS = 6000;
  static constexpr int DEFAULT_ERROR_RECTANGLE_DELAYED_CLOSE_MS = 5000;
  static constexpr int DEFAULT_TEST_BUSY_TIMEOUT_MS = 10000;
  static constexpr int DEFAULT_AUTO_LOCALIZATION_SPIN_TIMEOUT_MS = 30000;
  static constexpr int DEFAULT_AUTO_LOCALIZATION_SPIN_PUBLISH_PERIOD_MS = 100;
  static constexpr int DEFAULT_EXPERIMENT_SAVE_PROMPT_DELAY_MS = 3000;
};

} // namespace settings
} // namespace ROBOGait
