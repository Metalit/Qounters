#pragma once

#include "main.hpp"

#include "UnityEngine/Color.hpp"

namespace Qounters::Game {
    int GetScore(int saber);
    int GetMaxScore(int saber);
    int GetSongMaxScore();
    int GetCombo(int saber);
    bool GetFullCombo(int saber);
    int GetMultiplier();
    float GetMultiplierProgress(bool absolute);
    float GetHealth();
    float GetSongTime();
    float GetSongLength();
    int GetTotalNotes(int saber);
    int GetNotesCut(int saber);
    int GetNotesMissed(int saber);
    int GetNotesBadCut(int saber);
    int GetBombsHit(int saber);
    int GetWallsHit();
    int GetSongNotes();
    float GetPreSwing(int saber);
    float GetPostSwing(int saber);
    float GetAccuracy(int saber);
    float GetTimeDependence(int saber);
    float GetAverageSpeed(int saber);
    float GetBestSpeed5Secs(int saber);
    float GetModifierMultiplier(bool positive, bool negative);
    int GetBestScore();
    int GetFails();
    int GetRestarts();
    UnityEngine::Color GetColor(int color);
}
