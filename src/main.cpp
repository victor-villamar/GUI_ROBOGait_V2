#include <memory>

#include "Core/RoboGaitApplication.hpp"

int main(int argc, char* argv[])
{
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
