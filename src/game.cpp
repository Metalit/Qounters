#include "game.hpp"

#include "internals.hpp"
#include "main.hpp"
#include "pp.hpp"
#include "sources.hpp"

using namespace Qounters;
using namespace Qounters::Internals;

static bool IsLeft(int saber) {
    return saber == (int) Types::Sabers::Left || saber == (int) Types::Sabers::Both;
}
static bool IsRight(int saber) {
    return saber == (int) Types::Sabers::Right || saber == (int) Types::Sabers::Both;
}

int Game::GetScore(int saber) {
    int ret = 0;
    if (IsLeft(saber))
        ret += leftScore;
    if (IsRight(saber))
        ret += rightScore;
    return ret;
}
int Game::GetMaxScore(int saber) {
    int ret = 0;
    if (IsLeft(saber))
        ret += leftMaxScore;
    if (IsRight(saber))
        ret += rightMaxScore;
    return ret;
}
int Game::GetSongMaxScore() {
    return songMaxScore;
}
int Game::GetCombo(int saber) {
    if (saber == (int) Types::Sabers::Both)
        return combo;
    else if (IsLeft(saber))
        return leftCombo;
    else
        return rightCombo;
}
bool Game::GetFullCombo(int saber) {
    if (wallsHit > 0)
        return false;
    if (IsLeft(saber) && bombsLeftHit + notesLeftBadCut + notesLeftMissed > 0)
        return false;
    if (IsRight(saber) && bombsRightHit + notesRightBadCut + notesRightMissed > 0)
        return false;
    return true;
}
int Game::GetMultiplier() {
    if (combo < 2)
        return 1;
    if (combo < 2 + 4)
        return 2;
    if (combo < 2 + 4 + 8)
        return 4;
    return 8;
}
float Game::GetMultiplierProgress(bool absolute) {
    if (absolute)
        return std::min(combo / 14.0, 1.0);
    if (combo >= 2 + 4 + 8)
        return 1;
    if (combo >= 2 + 4)
        return (combo - 2 - 4) / 8.0;
    if (combo >= 2)
        return (combo - 2) / 4.0;
    return combo / 2.0;
}
float Game::GetHealth() {
    return health;
}
float Game::GetSongTime() {
    return songTime;
}
float Game::GetSongLength() {
    return songLength;
}
int Game::GetTotalNotes(int saber) {
    int ret = 0;
    if (IsLeft(saber))
        ret += notesLeftCut + notesLeftBadCut + notesLeftMissed;
    if (IsRight(saber))
        ret += notesRightCut + notesRightBadCut + notesRightMissed;
    return ret;
}
int Game::GetNotesCut(int saber) {
    int ret = 0;
    if (IsLeft(saber))
        ret += notesLeftCut;
    if (IsRight(saber))
        ret += notesRightCut;
    return ret;
}
int Game::GetNotesMissed(int saber) {
    int ret = 0;
    if (IsLeft(saber))
        ret += notesLeftMissed;
    if (IsRight(saber))
        ret += notesRightMissed;
    return ret;
}
int Game::GetNotesBadCut(int saber) {
    int ret = 0;
    if (IsLeft(saber))
        ret += notesLeftBadCut;
    if (IsRight(saber))
        ret += notesRightBadCut;
    return ret;
}
int Game::GetBombsHit(int saber) {
    int ret = 0;
    if (IsLeft(saber))
        ret += bombsLeftHit;
    if (IsRight(saber))
        ret += bombsRightHit;
    return ret;
}
int Game::GetWallsHit() {
    return wallsHit;
}
int Game::GetSongNotes(int saber) {
    int ret = 0;
    if (IsLeft(saber))
        ret += songNotesLeft;
    if (IsRight(saber))
        ret += songNotesRight;
    return ret;
}
float Game::GetPreSwing(int saber) {
    int notes = GetNotesCut(saber);
    if (notes == 0)
        return 0;
    int ret = 0;
    if (IsLeft(saber))
        ret += leftPreSwing;
    if (IsRight(saber))
        ret += rightPreSwing;
    return ret / (float) notes;
}
float Game::GetPostSwing(int saber) {
    int notes = GetNotesCut(saber);
    if (notes == 0)
        return 0;
    int ret = 0;
    if (IsLeft(saber))
        ret += leftPostSwing;
    if (IsRight(saber))
        ret += rightPostSwing;
    return ret / (float) notes;
}
float Game::GetAccuracy(int saber) {
    int notes = GetNotesCut(saber);
    if (notes == 0)
        return 0;
    int ret = 0;
    if (IsLeft(saber))
        ret += leftAccuracy;
    if (IsRight(saber))
        ret += rightAccuracy;
    return ret / (float) notes;
}
float Game::GetTimeDependence(int saber) {
    int notes = GetNotesCut(saber);
    if (notes == 0)
        return 0;
    float ret = 0;
    if (IsLeft(saber))
        ret += leftTimeDependence;
    if (IsRight(saber))
        ret += rightTimeDependence;
    return ret / notes;
}
float Game::GetAverageSpeed(int saber) {
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
    if (div == 0)
        return 0;
    return ret / div;
}
float Game::GetBestSpeed5Secs(int saber) {
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
float Game::GetLastSecAngle(int saber) {
    float ret = 0;
    int back = SPEED_SAMPLES_PER_SEC;
    if (IsLeft(saber)) {
        int size = leftAngles.size();
        int start = std::max(0, size - back);
        for (int i = start; i < size; i++)
            ret += leftAngles[i];
    }
    if (IsRight(saber)) {
        int size = rightAngles.size();
        int start = std::max(0, size - back);
        for (int i = start; i < size; i++)
            ret += rightAngles[i];
    }
    if (saber == (int) Types::Sabers::Both)
        ret /= 2;
    return ret;
}
float Game::GetHighestSecAngle(int saber) {
    float ret = 0;
    if (IsLeft(saber)) {
        for (auto& val : leftAngles)
            ret = std::max(ret, val);
    }
    if (IsRight(saber)) {
        for (auto& val : rightAngles)
            ret = std::max(ret, val);
    }
    return ret * SPEED_SAMPLES_PER_SEC;
}
float Game::GetModifierMultiplier(bool positive, bool negative) {
    float ret = 1;
    if (positive)
        ret += positiveMods;
    if (negative)
        ret += negativeMods;
    return ret;
}
double Game::GetBestScore() {
    return personalBest;
}
int Game::GetFails() {
    return fails;
}
int Game::GetRestarts() {
    return restarts;
}
UnityEngine::Color Game::GetColor(int color) {
    if (!colors)
        return UnityEngine::Color::get_white();
    switch ((Sources::Color::Player::ColorSettings) color) {
        case Sources::Color::Player::ColorSettings::LeftSaber:
            return colors->saberAColor;
        case Sources::Color::Player::ColorSettings::RightSaber:
            return colors->saberBColor;
        case Sources::Color::Player::ColorSettings::Lights1:
            return colors->environmentColor0;
        case Sources::Color::Player::ColorSettings::Lights2:
            return colors->environmentColor1;
        case Sources::Color::Player::ColorSettings::Walls:
            return colors->obstaclesColor;
        case Sources::Color::Player::ColorSettings::Boost1:
            return colors->environmentColor0Boost;
        case Sources::Color::Player::ColorSettings::Boost2:
            return colors->environmentColor1Boost;
    }
}
int Game::GetFCScore(int saber) {
    float swingRatio = (GetPreSwing(saber) + GetPostSwing(saber) + GetAccuracy(saber)) / 115;
    int missed = 0;
    int fixed = 0;
    if (IsLeft(saber)) {
        missed += leftMissedMaxScore;
        fixed += leftMissedFixedScore;
    }
    if (IsRight(saber)) {
        missed += rightMissedMaxScore;
        fixed += rightMissedFixedScore;
    }
    return (missed * swingRatio) + fixed + GetScore(saber);
}
bool Game::GetIsRanked(int leaderboards) {
    switch ((Sources::Enable::Ranked::Leaderboards) leaderboards) {
        case Sources::Enable::Ranked::Leaderboards::ScoreSaber:
            return PP::IsRankedSS();
        case Sources::Enable::Ranked::Leaderboards::BeatLeader:
            return PP::IsRankedBL();
        case Sources::Enable::Ranked::Leaderboards::Either:
            return PP::IsRankedSS() || PP::IsRankedBL();
        case Sources::Enable::Ranked::Leaderboards::Both:
            return PP::IsRankedSS() && PP::IsRankedBL();
    }
}
float Game::GetRankedPP(int leaderboard) {
    int score = leftScore + rightScore;
    int max = leftMaxScore + rightMaxScore;
    float percent = max > 0 ? score / (double) max : 0.95;
    bool failed = health <= 0;

    if (leaderboard == (int) Sources::Text::PP::Leaderboards::BeatLeader)
        return Qounters::PP::CalculateBL(percent, modifiers, failed);
    else if (leaderboard == (int) Sources::Text::PP::Leaderboards::ScoreSaber)
        return Qounters::PP::CalculateSS(percent, modifiers, failed);
    return 0;
}
