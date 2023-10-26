#pragma once

#include "main.hpp"

#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/NoteData.hpp"

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
    extern float leftPreSwing;
    extern float rightPreSwing;
    extern float leftPostSwing;
    extern float rightPostSwing;
    extern float leftAccuracy;
    extern float rightAccuracy;
    extern std::vector<float> leftSpeeds;
    extern std::vector<float> rightSpeeds;
    extern bool noFail;
    extern float positiveMods;
    extern float negativeMods;
    extern int personalBest;
    extern int fails;
    extern int restarts;
    extern GlobalNamespace::ColorScheme* colors;

    void Initialize();

    bool ShouldProcessNote(GlobalNamespace::NoteData* data);
}
