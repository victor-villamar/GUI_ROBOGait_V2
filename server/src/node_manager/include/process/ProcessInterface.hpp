#pragma once

#include <string>

#include "../common/enum.hpp"

namespace ROBOGait
{
namespace process
{
/**
 * @brief Interface for process management.
 */
class ProcessInterface
{
protected:
  /**
   * @brief Protected constructor to prevent direct instantiation.
   */
  ProcessInterface() : state_(common::ProcessState::UNKNOWN), name_("") {}

  /**
   * @brief Sets the state of the process.
   * @param state The new state to set.
   */
  void setState(common::ProcessState state) { state_ = state; }

  /**
   * @brief Sets the name of the process.
   * @param name The new name to set.
   */
  void setName(const std::string& name) { name_ = name; }

public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~ProcessInterface() = default;

  /**
   * @brief Pure virtual method to run the process.
   * Concrete implementations must define this method.
   */
  virtual void run() = 0;
  /**
   * @brief Pure virtual method to stop the process.
   * Concrete implementations must define this method.
   */
  virtual void stop() = 0;

  /**
   * @brief Gets the current state of the process.
   * @return The current process state.
   */
  common::ProcessState getState() const { return state_; }

  /**
   * @brief Gets the name of the process.
   * @return The name of the process.
   */
  const std::string& getName() const { return name_; }

protected:
  common::ProcessState state_; /**< Current state of the process */
  std::string name_;           /**< Name of the process */
};
} // namespace process
} // namespace ROBOGait