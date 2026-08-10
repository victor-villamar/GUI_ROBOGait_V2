#include <QDebug>

#include "Loader/YamlLoader.hpp"
#include "Settings/TimeoutSettings.hpp"

using namespace ROBOGait::settings;

TimeoutSettings& TimeoutSettings::getInstance()
{
  static TimeoutSettings instance;
  return instance;
}

TimeoutSettings::TimeoutSettings() :
    app_shutdown_delay_ms_(DEFAULT_APP_SHUTDOWN_DELAY_MS),
    auth_mode_dropdown_auto_close_ms_(DEFAULT_AUTH_MODE_DROPDOWN_AUTO_CLOSE_MS),
    register_default_idle_timeout_ms_(DEFAULT_REGISTER_DEFAULT_IDLE_TIMEOUT_MS),
    register_sign_in_idle_timeout_ms_(DEFAULT_REGISTER_SIGN_IN_IDLE_TIMEOUT_MS),
    badge_auto_close_ms_(DEFAULT_BADGE_AUTO_CLOSE_MS),
    robot_badge_auto_close_ms_(DEFAULT_ROBOT_BADGE_AUTO_CLOSE_MS),
    error_rectangle_delayed_close_ms_(DEFAULT_ERROR_RECTANGLE_DELAYED_CLOSE_MS),
    test_busy_timeout_ms_(DEFAULT_TEST_BUSY_TIMEOUT_MS),
    auto_localization_spin_timeout_ms_(DEFAULT_AUTO_LOCALIZATION_SPIN_TIMEOUT_MS),
    auto_localization_spin_publish_period_ms_(DEFAULT_AUTO_LOCALIZATION_SPIN_PUBLISH_PERIOD_MS),
    experiment_save_prompt_delay_ms_(DEFAULT_EXPERIMENT_SAVE_PROMPT_DELAY_MS)
{
  qInfo() << "[TimeoutSettings::TimeoutSettings] TimeoutSettings created";
}

int TimeoutSettings::getAppShutdownDelayMs() const { return app_shutdown_delay_ms_; }

int TimeoutSettings::getAuthModeDropdownAutoCloseMs() const { return auth_mode_dropdown_auto_close_ms_; }

int TimeoutSettings::getRegisterDefaultIdleTimeoutMs() const { return register_default_idle_timeout_ms_; }

int TimeoutSettings::getRegisterSignInIdleTimeoutMs() const { return register_sign_in_idle_timeout_ms_; }

int TimeoutSettings::getBadgeAutoCloseMs() const { return badge_auto_close_ms_; }

int TimeoutSettings::getRobotBadgeAutoCloseMs() const { return robot_badge_auto_close_ms_; }

int TimeoutSettings::getErrorRectangleDelayedCloseMs() const { return error_rectangle_delayed_close_ms_; }

int TimeoutSettings::getTestBusyTimeoutMs() const { return test_busy_timeout_ms_; }

int TimeoutSettings::getAutoLocalizationSpinTimeoutMs() const { return auto_localization_spin_timeout_ms_; }

int TimeoutSettings::getAutoLocalizationSpinPublishPeriodMs() const { return auto_localization_spin_publish_period_ms_; }

int TimeoutSettings::getExperimentSavePromptDelayMs() const { return experiment_save_prompt_delay_ms_; }

void TimeoutSettings::initializeDefaults()
{
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qWarning() << "[TimeoutSettings::initializeDefaults] YAML configuration not loaded, using constructor defaults";
    return;
  }

  app_shutdown_delay_ms_ =
      sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.app_shutdown_delay_ms", DEFAULT_APP_SHUTDOWN_DELAY_MS), DEFAULT_APP_SHUTDOWN_DELAY_MS);

  auth_mode_dropdown_auto_close_ms_ =
      sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.auth_mode_dropdown_auto_close_ms", DEFAULT_AUTH_MODE_DROPDOWN_AUTO_CLOSE_MS),
                       DEFAULT_AUTH_MODE_DROPDOWN_AUTO_CLOSE_MS);

  register_default_idle_timeout_ms_ =
      sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.register_default_idle_timeout_ms", DEFAULT_REGISTER_DEFAULT_IDLE_TIMEOUT_MS),
                       DEFAULT_REGISTER_DEFAULT_IDLE_TIMEOUT_MS);

  register_sign_in_idle_timeout_ms_ =
      sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.register_sign_in_idle_timeout_ms", DEFAULT_REGISTER_SIGN_IN_IDLE_TIMEOUT_MS),
                       DEFAULT_REGISTER_SIGN_IN_IDLE_TIMEOUT_MS);

  badge_auto_close_ms_ =
      sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.badge_auto_close_ms", DEFAULT_BADGE_AUTO_CLOSE_MS), DEFAULT_BADGE_AUTO_CLOSE_MS);

  robot_badge_auto_close_ms_ = sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.robot_badge_auto_close_ms", DEFAULT_ROBOT_BADGE_AUTO_CLOSE_MS),
                                                DEFAULT_ROBOT_BADGE_AUTO_CLOSE_MS);

  error_rectangle_delayed_close_ms_ =
      sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.error_rectangle_delayed_close_ms", DEFAULT_ERROR_RECTANGLE_DELAYED_CLOSE_MS),
                       DEFAULT_ERROR_RECTANGLE_DELAYED_CLOSE_MS);

  test_busy_timeout_ms_ =
      sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.test_busy_timeout_ms", DEFAULT_TEST_BUSY_TIMEOUT_MS), DEFAULT_TEST_BUSY_TIMEOUT_MS);

  auto_localization_spin_timeout_ms_ =
      sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.auto_localization_spin_timeout_ms", DEFAULT_AUTO_LOCALIZATION_SPIN_TIMEOUT_MS),
                       DEFAULT_AUTO_LOCALIZATION_SPIN_TIMEOUT_MS);

  auto_localization_spin_publish_period_ms_ =
      sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.auto_localization_spin_publish_period_ms", DEFAULT_AUTO_LOCALIZATION_SPIN_PUBLISH_PERIOD_MS),
                       DEFAULT_AUTO_LOCALIZATION_SPIN_PUBLISH_PERIOD_MS);

  experiment_save_prompt_delay_ms_ =
      sanitizePositive(yaml_loader.getValue<int>("timeouts.qml.experiment_save_prompt_delay_ms", DEFAULT_EXPERIMENT_SAVE_PROMPT_DELAY_MS),
                       DEFAULT_EXPERIMENT_SAVE_PROMPT_DELAY_MS);

  emit timeoutsChanged();
}

int TimeoutSettings::sanitizePositive(int value, int fallback) const { return value > 0 ? value : fallback; }
