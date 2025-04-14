#pragma once

#include "sources.hpp"

namespace Qounters::Events {
    enum CustomEvents {
        PPInfo,
    };

    void RegisterToEvent(Types::Sources sourceType, std::string source, int event);
    void RegisterToEvent(Types::Sources sourceType, std::string source, std::string mod, int event);
    void RegisterToQountersEvent(Types::Sources sourceType, std::string source, std::string mod, int event);
}
