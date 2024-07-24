#pragma once

#include "sources.hpp"

namespace Qounters {
    enum class Events {
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

    int RegisterCustomEvent(std::string mod, int event);

    void RegisterToEvent(Sources sourceType, std::string source, int event);

    void BroadcastEvent(int event);
    void BroadcastEvent(std::string mod, int event);
}
