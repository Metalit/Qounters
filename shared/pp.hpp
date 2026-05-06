#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "export.h"
#include "metacore/shared/pp.hpp"
#include "rapidjson-macros/shared/macros.hpp"

namespace Qounters::PP {
    QOUNTERS_EXPORT extern bool blSongValid;
    QOUNTERS_EXPORT extern MetaCore::PP::BLSongDiff latestBeatleaderSong;
    QOUNTERS_EXPORT extern bool ssSongValid;
    QOUNTERS_EXPORT extern MetaCore::PP::SSSongDiff latestScoresaberSong;

    QOUNTERS_EXPORT bool IsRankedBL();
    QOUNTERS_EXPORT bool IsRankedSS();
    QOUNTERS_EXPORT float CalculateBL(float percentage, GlobalNamespace::GameplayModifiers* modifiers, bool failed);
    QOUNTERS_EXPORT float CalculateSS(float percentage, GlobalNamespace::GameplayModifiers* modifiers, bool failed);
    QOUNTERS_EXPORT void GetMapInfo(GlobalNamespace::BeatmapKey map);
    QOUNTERS_EXPORT void Reset();
}
