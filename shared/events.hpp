#pragma once

#include "export.h"
#include "sources.hpp"

namespace Qounters::Events {
    enum CustomEvents {
        MapInfo,
    };

    QOUNTERS_EXPORT void RegisterToEvent(Types::Sources sourceType, std::string source, int event);
    QOUNTERS_EXPORT void RegisterToEvent(Types::Sources sourceType, std::string source, std::string mod, int event);
    QOUNTERS_EXPORT void RegisterToQountersEvent(Types::Sources sourceType, std::string source, int event);

    QOUNTERS_EXPORT void BroadcastQountersEvent(int event);
}
