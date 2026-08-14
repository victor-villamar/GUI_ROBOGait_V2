#include "Map/Source/SourceInterface.hpp"

#include <utility>

using namespace ROBOGait::map::source;

SourceInterface::SourceResult SourceInterface::SourceResult::success() { return SourceResult(true, {}); }

SourceInterface::SourceResult SourceInterface::SourceResult::failure(std::string error_in) { return SourceResult(false, std::move(error_in)); }

SourceInterface::SourceResult::SourceResult(bool success_in, std::string error_in) : error(std::move(error_in)), success_(success_in) {}

SourceInterface::SourceResult::operator bool() const { return success_; }