#include "sources.hpp"
#include "game.hpp"
#include "internals.hpp"

using namespace Qounters;

bool IsLeft(int saber) {
    return saber == (int) Sabers::Left || saber == (int) Sabers::Both;
}
bool IsRight(int saber) {
    return saber == (int) Sabers::Right || saber == (int) Sabers::Both;
}

namespace Qounters::Game {
    int GetScore(int saber) {
        int ret = 0;
        if (IsLeft(saber)) ret += leftScore;
        if (IsRight(saber)) ret += rightScore;
        return ret;
    }
    int GetMaxScore(int saber) {
        int ret = 0;
        if (IsLeft(saber)) ret += leftMaxScore;
        if (IsRight(saber)) ret += rightMaxScore;
        return ret;
    }
    int GetSongMaxScore() {
        return songMaxScore;
    }
    int GetCombo(int saber) {
        if (saber == (int) Sabers::Both) return combo;
        else if (IsLeft(saber)) return leftCombo;
        else return rightCombo;
    }
    bool GetFullCombo(int saber) {
        if (wallsHit > 0)
            return false;
        if (IsLeft(saber) && bombsLeftHit + notesLeftBadCut + notesLeftMissed > 0)
            return false;
        if (IsRight(saber) && bombsRightHit + notesRightBadCut + notesRightMissed > 0)
            return false;
        return true;
    }
    int GetMultiplier() {
        if (combo < 2) return 1;
        if (combo < 2 + 4) return 2;
        if (combo < 2 + 4 + 8) return 4;
        return 8;
    }
    float GetMultiplierProgress(bool absolute) {
        if (absolute) return std::min(combo / 14.0, 1.0);
        if (combo >= 2 + 4 + 8) return 1;
        if (combo >= 2 + 4) return (combo - 2 - 4) / 8.0;
        if (combo >= 2) return (combo - 2) / 4.0;
        return combo / 2.0;
    }
    float GetHealth() {
        return health;
    }
    float GetSongTime() {
        return songTime;
    }
    float GetSongLength() {
        return songLength;
    }
    int GetTotalNotes(int saber) {
        int ret = 0;
        if (IsLeft(saber)) ret += notesLeftCut + notesLeftBadCut + notesLeftMissed;
        if (IsRight(saber)) ret += notesRightCut + notesRightBadCut + notesRightMissed;
        return ret;
    }
    int GetNotesCut(int saber) {
        int ret = 0;
        if (IsLeft(saber)) ret += notesLeftCut;
        if (IsRight(saber)) ret += notesRightCut;
        return ret;
    }
    int GetNotesMissed(int saber) {
        int ret = 0;
        if (IsLeft(saber)) ret += notesLeftMissed;
        if (IsRight(saber)) ret += notesRightMissed;
        return ret;
    }
    int GetNotesBadCut(int saber) {
        int ret = 0;
        if (IsLeft(saber)) ret += notesLeftBadCut;
        if (IsRight(saber)) ret += notesRightBadCut;
        return ret;
    }
    int GetBombsHit(int saber) {
        int ret = 0;
        if (IsLeft(saber)) ret += bombsLeftHit;
        if (IsRight(saber)) ret += bombsRightHit;
        return ret;
    }
    int GetWallsHit() {
        return wallsHit;
    }
    int GetSongNotes() {
        return songNotes;
    }
    float GetPreSwing(int saber) {
        int notes = GetNotesCut(saber);
        if (notes == 0) return 0;
        int ret = 0;
        if (IsLeft(saber)) ret += leftPreSwing;
        if (IsRight(saber)) ret += rightPreSwing;
        return ret / (float) notes;
    }
    float GetPostSwing(int saber) {
        int notes = GetNotesCut(saber);
        if (notes == 0) return 0;
        int ret = 0;
        if (IsLeft(saber)) ret += leftPostSwing;
        if (IsRight(saber)) ret += rightPostSwing;
        return ret / (float) notes;
    }
    float GetAccuracy(int saber) {
        int notes = GetNotesCut(saber);
        if (notes == 0) return 0;
        int ret = 0;
        if (IsLeft(saber)) ret += leftAccuracy;
        if (IsRight(saber)) ret += rightAccuracy;
        return ret / (float) notes;
    }
    float GetTimeDependence(int saber) {
        int notes = GetNotesCut(saber);
        if (notes == 0) return 0;
        float ret = 0;
        if (IsLeft(saber)) ret += leftTimeDependence;
        if (IsRight(saber)) ret += rightTimeDependence;
        return ret / notes;
    }
    float GetAverageSpeed(int saber) {
        float ret = 0;
        int div = 0;
        if (IsLeft(saber)) {
            for (auto& val : leftSpeeds)
                ret += val;
            div += leftSpeeds.size();
        }
        if (IsRight(saber)) {
            for (auto& val : rightSpeeds)
                ret += val;
            div += rightSpeeds.size();
        }
        if (div == 0) return 0;
        return ret / div;
    }
    float GetBestSpeed5Secs(int saber) {
        float ret = 0;
        int back = SPEED_SAMPLES_PER_SEC * 5;
        if (IsLeft(saber)) {
            int size = leftSpeeds.size();
            int start = std::max(0, size - back);
            for (int i = start; i < size; i++)
                ret = std::max(ret, leftSpeeds[i]);
        }
        if (IsRight(saber)) {
            int size = rightSpeeds.size();
            int start = std::max(0, size - back);
            for (int i = start; i < size; i++)
                ret = std::max(ret, rightSpeeds[i]);
        }
        return ret;
    }
    float GetModifierMultiplier(bool positive, bool negative) {
        float ret = 1;
        if (positive)
            ret += positiveMods;
        if (negative)
            ret += negativeMods;
        return ret;
    }
    int GetBestScore() {
        return personalBest;
    }
    int GetFails() {
        return fails;
    }
    int GetRestarts() {
        return restarts;
    }
    UnityEngine::Color GetColor(int color) {
        switch ((ColorSource::Player::ColorSettings) color) {
            case ColorSource::Player::ColorSettings::LeftSaber:
                return colors->get_saberAColor();
            case ColorSource::Player::ColorSettings::RightSaber:
                return colors->get_saberBColor();
            case ColorSource::Player::ColorSettings::Lights1:
                return colors->get_environmentColor0();
            case ColorSource::Player::ColorSettings::Lights2:
                return colors->get_environmentColor1();
            case ColorSource::Player::ColorSettings::Walls:
                return colors->get_obstaclesColor();
            case ColorSource::Player::ColorSettings::Boost1:
                return colors->get_environmentColor0Boost();
            case ColorSource::Player::ColorSettings::Boost2:
                return colors->get_environmentColor1Boost();
        }
    }
}
