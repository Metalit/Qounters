#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "main.hpp"

namespace Qounters::PP {
    bool IsRankedBL();
    bool IsRankedSS();
    float CalculateBL(float percentage, bool failed);
    float CalculateSS(float percentage, bool failed);
    void GetMapInfo(GlobalNamespace::BeatmapKey map);
}
