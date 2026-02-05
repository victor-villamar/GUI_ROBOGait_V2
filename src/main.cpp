#include <memory>

#include "Core/RoboGaitApplication.hpp"

int main(int argc, char* argv[])
{
  qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

  auto app = std::make_unique<ROBOGait::core::RoboGaitApplication>(argc, argv);

  app->initCommon();
  app->initialize();

  if (!app->initForNormalAppBoot())
  {
    return -1;
  }

  return app->exec();
}
