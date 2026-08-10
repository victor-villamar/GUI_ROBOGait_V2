#include <QDebug>

#include "Context/RobotContext.hpp"

using namespace ROBOGait::context;

RobotContext::RobotContext() : use_namespace_(true) {}

bool RobotContext::setSelectedRobot(const QString& robot_identifier, bool is_namespace)
{
  QString trimmed = robot_identifier.trimmed();
  if (trimmed.isEmpty())
  {
    qWarning() << "[RobotContext::setSelectedRobot] Empty robot identifier";
    return false;
  }

  robot_identifier_ = trimmed;
  use_namespace_ = is_namespace;

  if (is_namespace)
  {
    const QString normalized = normalizeNamespace(trimmed);
    if (normalized.isEmpty())
    {
      qWarning() << "[RobotContext::setSelectedRobot] Invalid namespace";
      return false;
    }

    topic_namespace_ = normalized;
  }
  else
  {
    topic_namespace_.clear();
  }

  return true;
}

void RobotContext::clear()
{
  robot_identifier_.clear();
  topic_namespace_.clear();
  use_namespace_ = true;
}

bool RobotContext::isConfigured() const { return !robot_identifier_.isEmpty(); }

bool RobotContext::usesNamespace() const { return use_namespace_; }

QString RobotContext::topicNamespace() const { return topic_namespace_; }

std::string RobotContext::resolveTopic(const std::string& topic) const
{
  std::string normalized = normalizeTopic(topic);

  if (!use_namespace_ || topic_namespace_.isEmpty())
  {
    return normalized;
  }

  std::string ns = topic_namespace_.toStdString();

  if (normalized == ns || normalized.rfind(ns + "/", 0) == 0)
  {
    return normalized;
  }

  return ns + normalized;
}

std::string RobotContext::resolveFrame(const std::string& frame) const
{
  std::string normalized = normalizeFrame(frame);

  if (!use_namespace_ || topic_namespace_.isEmpty())
  {
    return normalized;
  }

  std::string ns = topic_namespace_.toStdString();
  if (!ns.empty() && ns.front() == '/')
  {
    ns.erase(0, 1);
  }

  if (normalized == ns || normalized.rfind(ns + "/", 0) == 0)
  {
    return normalized;
  }

  return ns + "/" + normalized;
}

QString RobotContext::normalizeNamespace(const QString& robot_namespace) const
{
  QString normalized = robot_namespace.trimmed();
  if (normalized.isEmpty())
  {
    qWarning() << "[RobotContext::normalizeNamespace] Empty namespace provided";
    return QString();
  }

  if (!normalized.startsWith('/'))
  {
    normalized.prepend('/');
  }

  while (normalized.size() > 1 && normalized.endsWith('/'))
  {
    normalized.chop(1);
  }

  normalized.replace(" ", "_");

  return normalized;
}

std::string RobotContext::normalizeTopic(const std::string& topic) const
{
  if (topic.empty())
  {
    return topic;
  }

  if (topic.front() == '/')
  {
    return topic;
  }

  return "/" + topic;
}

std::string RobotContext::normalizeFrame(const std::string& frame) const
{
  if (frame.empty())
  {
    return frame;
  }

  if (frame.front() == '/')
  {
    return frame.substr(1);
  }

  return frame;
}
