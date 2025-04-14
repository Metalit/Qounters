#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "metacore/shared/pp.hpp"
#include "rapidjson-macros/shared/macros.hpp"

namespace Qounters::PP {
    extern bool blSongValid;
    extern MetaCore::PP::BLSongDiff latestBeatleaderSong;
    extern bool ssSongValid;
    extern MetaCore::PP::SSSongDiff latestScoresaberSong;

    bool IsRankedBL();
    bool IsRankedSS();
    float CalculateBL(float percentage, GlobalNamespace::GameplayModifiers* modifiers, bool failed);
    float CalculateSS(float percentage, GlobalNamespace::GameplayModifiers* modifiers, bool failed);
    void GetMapInfo(GlobalNamespace::BeatmapKey map);
    void Reset();
}
