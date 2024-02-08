#pragma once

#include "main.hpp"

#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"

namespace Qounters::PP {
    bool IsRankedBL();
    bool IsRankedSS();
    float CalculateBL(float percentage, bool failed);
    float CalculateSS(float percentage, bool failed);
    void GetMapInfo(GlobalNamespace::IDifficultyBeatmap* map);
}
