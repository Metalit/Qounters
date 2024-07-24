#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "rapidjson-macros/shared/macros.hpp"

namespace Qounters::PP {
    DECLARE_JSON_STRUCT(BLSpeedModifiers) {
        VALUE(float, ssPassRating);
        VALUE(float, ssAccRating);
        VALUE(float, ssTechRating);
        VALUE(float, fsPassRating);
        VALUE(float, fsAccRating);
        VALUE(float, fsTechRating);
        VALUE(float, sfPassRating);
        VALUE(float, sfAccRating);
        VALUE(float, sfTechRating);
    };

    DECLARE_JSON_STRUCT(BLSongDiff) {
        NAMED_VALUE(std::string, Difficulty, "difficultyName");
        NAMED_VALUE(std::string, Characteristic, "modeName");
        NAMED_VALUE_DEFAULT(int, RankedStatus, 0, "status");
        NAMED_VALUE_DEFAULT(float, Stars, 0, "stars");
        NAMED_VALUE_DEFAULT(float, Predicted, 0, "predictedAcc");
        NAMED_VALUE_DEFAULT(float, Pass, 0, "passRating");
        NAMED_VALUE_DEFAULT(float, Acc, 0, "accRating");
        NAMED_VALUE_DEFAULT(float, Tech, 0, "techRating");
        NAMED_MAP_DEFAULT(float, ModifierValues, {}, "modifierValues");
        NAMED_VALUE_OPTIONAL(BLSpeedModifiers, ModifierRatings, "modifiersRating");
    };

    DECLARE_JSON_STRUCT(BLSong) {
        NAMED_VECTOR(BLSongDiff, Difficulties, "difficulties");
    };

    extern std::vector<std::pair<double, double>> const BeatLeaderCurve;
    extern std::vector<std::pair<double, double>> const ScoreSaberCurve;

    extern bool blSongValid;
    extern PP::BLSongDiff latestBeatleaderSong;
    extern bool ssSongValid;
    extern float latestScoresaberSong;

    std::vector<std::string> GetModStringsBL(GlobalNamespace::GameplayModifiers* modifiers, bool speeds, bool failed);
    float AccCurve(float acc, std::vector<std::pair<double, double>> const& curve);

    bool IsRankedBL();
    bool IsRankedSS();
    float CalculateBL(float percentage, GlobalNamespace::GameplayModifiers* modifiers, bool failed);
    float CalculateSS(float percentage, GlobalNamespace::GameplayModifiers* modifiers, bool failed);
    void GetMapInfo(GlobalNamespace::BeatmapKey map);
    void Reset();
}
