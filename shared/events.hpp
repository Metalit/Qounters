#pragma once

#include "sources.hpp"

namespace Qounters::Events {
    enum Events {
        ScoreChanged,
        NoteCut,
        NoteMissed,
        BombCut,
        WallHit,
        ComboChanged,
        HealthChanged,
        Update,
        SlowUpdate,
        MapInfo,
        EventMax = MapInfo,
    };

    int RegisterCustom(std::string mod, int event);

    void RegisterTo(Types::Sources sourceType, std::string source, int event);

    void Broadcast(int event);
    void Broadcast(std::string mod, int event);
}
