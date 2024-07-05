#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "main.hpp"

namespace Qounters::PP {
    bool IsRankedBL();
    bool IsRankedSS();
    float CalculateBL(float percentage, GlobalNamespace::GameplayModifiers* modifiers, bool failed);
    float CalculateSS(float percentage, GlobalNamespace::GameplayModifiers* modifiers, bool failed);
    void GetMapInfo(GlobalNamespace::BeatmapKey map);
}
