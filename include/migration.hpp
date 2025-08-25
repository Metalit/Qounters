#pragma once

#include "rapidjson-macros/shared/macros.hpp"

namespace Qounters::Migration {
    DECLARE_JSON_STRUCT(Score) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(std::string, Mode);
        VALUE(int, DecimalPrecision);
        VALUE(bool, CustomRankColors);
        VALUE(std::string, SSColor);
        VALUE(std::string, SColor);
        VALUE(std::string, AColor);
        VALUE(std::string, BColor);
        VALUE(std::string, CColor);
        VALUE(std::string, DColor);
        VALUE(std::string, EColor);
    };

    DECLARE_JSON_STRUCT(PersonalBest) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(std::string, Mode);
        VALUE(int, DecimalPrecision);
        VALUE(int, TextSize);
        VALUE(bool, UnderScore);
        VALUE(bool, HideFirstScore);
        VALUE(std::string, BetterColor);
        VALUE(std::string, DefaultColor);
    };

    DECLARE_JSON_STRUCT(AverageCut) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(bool, SeparateSaberCounts);
        VALUE(bool, SeparateCutValues);
        VALUE(int, AveragePrecision);
    };

    DECLARE_JSON_STRUCT(NotesLeft) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(bool, LabelAboveCount);
    };

    DECLARE_JSON_STRUCT(Notes) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(int, DecimalPrecision);
        VALUE(bool, ShowPercentage);
    };

    DECLARE_JSON_STRUCT(Misses) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(bool, CountBadCuts);
    };

    DECLARE_JSON_STRUCT(Fails) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(bool, ShowRestartsInstead);
    };

    DECLARE_JSON_STRUCT(SongProgress) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(std::string, Mode);
        VALUE(bool, ProgressTimeLeft);
        VALUE(bool, IncludeRing);
    };

    DECLARE_JSON_STRUCT(PP) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(bool, HideWhenUnranked);
    };

    DECLARE_JSON_STRUCT(SaberSpeed) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(std::string, Mode);
        VALUE(int, DecimalPrecision);
    };

    DECLARE_JSON_STRUCT(Spinometer) {
        VALUE(bool, Enabled);
        VALUE(std::string, Position);
        VALUE(float, Distance);
        VALUE(std::string, Mode);
    };

    DECLARE_JSON_STRUCT(OldConfig) {
        VALUE(Score, ScoreConfig);
        VALUE(PersonalBest, PBConfig);
        VALUE(AverageCut, CutConfig);
        VALUE(NotesLeft, NotesLeftConfig);
        VALUE(Notes, NoteConfig);
        VALUE(Misses, MissedConfig);
        VALUE(Fails, FailConfig);
        VALUE(SongProgress, ProgressConfig);
        VALUE(PP, PPConfig);
        VALUE(SaberSpeed, SpeedConfig);
        VALUE(Spinometer, Spinometer);

        VALUE(bool, Enabled);
        VALUE(bool, HideCombo);
        VALUE(bool, HideMultiplier);
        VALUE(bool, ItalicText);
        VALUE(bool, UprightInMultiplayer);
        VALUE(bool, DisableIn90Degree);
        VALUE(bool, FixedHUDPosition);
        VALUE(float, ComboOffset);
        VALUE(float, MultiplierOffset);
    };

    void Migrate();
}
