#include <memory>
#include <signal.h>

#include "Core/RoboGaitApplication.hpp"

/**
 * @brief Signal handler for graceful shutdown
 *
 * @param sig Signal number received
 */
void signalHandler(int sig)
{
  qDebug() << "[main::signalHandler] Received signal" << sig << "- shutting down gracefully";
  QCoreApplication::quit();
}

int main(int argc, char* argv[])
{
  // Install signal handlers for graceful shutdown
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

  auto app = std::make_unique<ROBOGait::core::RoboGaitApplication>(argc, argv);

  app->initCommon();

  if (!app->initialize())
  {
    qCritical() << "Failed to initialize application. Exiting...";
    return -1;
  }

  if (!app->initForNormalAppBoot())
  {
    qCritical() << "Failed to initialize QML engine. Exiting...";
    return -1;
  }

  return app->exec();
}
