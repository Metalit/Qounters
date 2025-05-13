#include "pp.hpp"

#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapDifficultySerializedMethods.hpp"
#include "System/Action_1.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "custom-types/shared/delegate.hpp"
#include "environment.hpp"
#include "events.hpp"
#include "main.hpp"
#include "metacore/shared/events.hpp"
#include "metacore/shared/songs.hpp"

using namespace Qounters;
using namespace GlobalNamespace;

static BeatmapKey latestRequest = {nullptr, 0, nullptr};

bool PP::blSongValid = false;
MetaCore::PP::BLSongDiff PP::latestBeatleaderSong;
bool PP::ssSongValid = false;
MetaCore::PP::SSSongDiff PP::latestScoresaberSong;

bool PP::IsRankedBL() {
    if (Environment::InSettings())
        return blSongValid;
    // https://github.com/BeatLeader/beatleader-qmod/blob/b5b7dc811f6b39f52451d2dad9ebb70f3ad4ad57/src/UI/LevelInfoUI.cpp#L78
    return blSongValid && latestBeatleaderSong.Stars > 0 && latestBeatleaderSong.RankedStatus == 3;
}

bool PP::IsRankedSS() {
    if (Environment::InSettings())
        return ssSongValid;
    return ssSongValid && latestScoresaberSong > 0;
}

float PP::CalculateBL(float percentage, GameplayModifiers* modifiers, bool failed) {
    if (!blSongValid)
        return 0;

    if (Environment::InSettings()) {
        MetaCore::PP::BLSongDiff customDiff = {};
        customDiff.Pass = 0.8 * settingsStarsBL;
        customDiff.Acc = settingsStarsBL;
        customDiff.Tech = 0.5 * settingsStarsBL;
        return MetaCore::PP::Calculate(customDiff, percentage, modifiers, failed);
    }

    return MetaCore::PP::Calculate(latestBeatleaderSong, percentage, modifiers, failed);
}

float PP::CalculateSS(float percentage, GameplayModifiers* modifiers, bool failed) {
    if (!ssSongValid)
        return 0;
    return MetaCore::PP::Calculate(Environment::InSettings() ? settingsStarsSS : latestScoresaberSong, percentage, modifiers, failed);
}

static void OnMapInfo(std::optional<MetaCore::PP::BLSongDiff> bl, std::optional<MetaCore::PP::SSSongDiff> ss) {
    if (bl.has_value()) {
        PP::latestBeatleaderSong = *bl;
        PP::blSongValid = true;
    }
    if (ss.has_value()) {
        PP::latestScoresaberSong = *ss;
        PP::ssSongValid = true;
    }
    Events::BroadcastQountersEvent(Events::MapInfo);
}

void PP::GetMapInfo(BeatmapKey map) {
    if (!map.IsValid())
        return;

    blSongValid = false;
    ssSongValid = false;
    latestRequest = map;
    Events::BroadcastQountersEvent(Events::MapInfo);

    MetaCore::PP::GetMapInfo(map, [map](std::optional<MetaCore::PP::BLSongDiff> bl, std::optional<MetaCore::PP::SSSongDiff> ss) {
        if (latestRequest.Equals(map))
            OnMapInfo(bl, ss);
    });
}

void PP::Reset() {
    blSongValid = false;
    ssSongValid = false;
    latestRequest = {nullptr, 0, nullptr};
    Events::BroadcastQountersEvent(Events::MapInfo);
}

ON_EVENT(MetaCore::Events::MapSelected) {
    PP::GetMapInfo(MetaCore::Songs::GetSelectedKey());
}
