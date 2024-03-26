#pragma once

#include "main.hpp"

#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"

namespace Qounters::PP {
    bool IsRankedBL();
    bool IsRankedSS();
    float CalculateBL(float percentage, bool failed);
    float CalculateSS(float percentage, bool failed);
    void GetMapInfo(GlobalNamespace::BeatmapKey key);
}
