#pragma once

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "types.hpp"

namespace Qounters::Internals {
    extern int leftScore;
    extern int rightScore;
    extern int leftMaxScore;
    extern int rightMaxScore;
    extern int songMaxScore;
    extern int leftCombo;
    extern int rightCombo;
    extern int combo;
    extern float health;
    extern float songTime;
    extern float songLength;
    extern int notesLeftCut;
    extern int notesRightCut;
    extern int notesLeftBadCut;
    extern int notesRightBadCut;
    extern int notesLeftMissed;
    extern int notesRightMissed;
    extern int bombsLeftHit;
    extern int bombsRightHit;
    extern int wallsHit;
    extern int songNotesLeft;
    extern int songNotesRight;
    extern int leftPreSwing;
    extern int rightPreSwing;
    extern int leftPostSwing;
    extern int rightPostSwing;
    extern int leftAccuracy;
    extern int rightAccuracy;
    extern float leftTimeDependence;
    extern float rightTimeDependence;
    extern std::vector<float> leftSpeeds;
    extern std::vector<float> rightSpeeds;
    extern std::vector<float> leftAngles;
    extern std::vector<float> rightAngles;
    extern bool noFail;
    extern GlobalNamespace::GameplayModifiers* modifiers;
    extern float positiveMods;
    extern float negativeMods;
    extern double personalBest;
    extern int fails;
    extern int restarts;
    extern GlobalNamespace::ColorScheme* colors;
    extern GlobalNamespace::BeatmapLevel* beatmapLevel;
    extern GlobalNamespace::BeatmapKey beatmapKey;
    extern GlobalNamespace::BeatmapData* beatmapData;
    extern GlobalNamespace::EnvironmentInfoSO* environment;
    extern int leftMissedMaxScore;
    extern int rightMissedMaxScore;
    extern int leftMissedFixedScore;
    extern int rightMissedFixedScore;
    extern float timeSinceSlowUpdate;
    extern UnityEngine::Quaternion prevRotLeft;
    extern UnityEngine::Quaternion prevRotRight;
    extern UnityW<GlobalNamespace::SaberManager> saberManager;

    extern UnorderedEventCallback<> onInitialize;

    void Initialize();

    bool IsFakeNote(GlobalNamespace::NoteData* data);
    bool ShouldCountNote(GlobalNamespace::NoteData* data);
    void DoSlowUpdate();
}
