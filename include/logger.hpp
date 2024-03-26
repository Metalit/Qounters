#pragma once
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "paper/shared/logger.hpp"

class QountersLogger {
public:
  static constexpr auto Logger = Paper::ConstLoggerContext("Qounters++");
};
