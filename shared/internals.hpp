#pragma once

#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "types.hpp"

namespace Qounters {
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
    extern int songNotes;
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
    extern float positiveMods;
    extern float negativeMods;
    extern int personalBest;
    extern int fails;
    extern int restarts;
    extern GlobalNamespace::ColorScheme* colors;
    extern GlobalNamespace::BeatmapData* beatmapData;
    extern int leftMissedMaxScore;
    extern int rightMissedMaxScore;
    extern int leftMissedFixedScore;
    extern int rightMissedFixedScore;

    void Initialize();

    bool ShouldProcessNote(GlobalNamespace::NoteData* data);
}
