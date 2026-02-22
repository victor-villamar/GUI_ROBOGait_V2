#include <QDebug>
#include <QString>

#include "Map/Display/BaseDisplay.hpp"

using namespace ROBOGait::map::display;

BaseDisplay::BaseDisplay(const std::string& name, QObject* parent) : QObject(parent), name_(name), enabled_(true), parent_node_(nullptr)
{
  qInfo() << "[BaseDisplay::" << QString::fromStdString(name_) << "] Display created";
}

void BaseDisplay::setEnabled(bool enabled)
{
  if (enabled_ != enabled)
  {
    enabled_ = enabled;
    qInfo() << "[BaseDisplay::" << QString::fromStdString(name_) << "] Display" << (enabled_ ? "enabled" : "disabled");

    if (enabled_)
    {
      emit renderRequested();
    }
  }
}
