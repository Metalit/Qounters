#include "sources.hpp"

#include "customtypes/components.hpp"
#include "environment.hpp"
#include "metacore/shared/internals.hpp"
#include "metacore/shared/stats.hpp"
#include "metacore/shared/strings.hpp"
#include "playtest.hpp"
#include "pp.hpp"
#include "sourceui.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace MetaCore;

std::vector<std::pair<std::string, std::pair<Types::SourceFn<std::string>, Types::SourceUIFn>>> Sources::texts = {
    {Sources::Text::StaticName, {Sources::Text::GetStatic, Sources::Text::StaticUI}},
    {Sources::Text::ScoreName, {Sources::Text::GetScore, Sources::Text::ScoreUI}},
    {Sources::Text::RankName, {Sources::Text::GetRank, Sources::Text::RankUI}},
    {Sources::Text::PersonalBestName, {Sources::Text::GetPersonalBest, Sources::Text::PersonalBestUI}},
    {Sources::Text::ComboName, {Sources::Text::GetCombo, Sources::Text::ComboUI}},
    {Sources::Text::MultiplierName, {Sources::Text::GetMultiplier, Sources::Text::MultiplierUI}},
    {Sources::Text::HealthName, {Sources::Text::GetHealth, Sources::Text::HealthUI}},
    {Sources::Text::TimeName, {Sources::Text::GetTime, Sources::Text::TimeUI}},
    {Sources::Text::AverageCutName, {Sources::Text::GetAverageCut, Sources::Text::AverageCutUI}},
    {Sources::Text::TimeDependenceName, {Sources::Text::GetTimeDependence, Sources::Text::TimeDependenceUI}},
    {Sources::Text::FailsName, {Sources::Text::GetFails, Sources::Text::FailsUI}},
    {Sources::Text::MistakesName, {Sources::Text::GetMistakes, Sources::Text::MistakesUI}},
    {Sources::Text::NotesName, {Sources::Text::GetNotes, Sources::Text::NotesUI}},
    {Sources::Text::PPName, {Sources::Text::GetPP, Sources::Text::PPUI}},
    {Sources::Text::SaberSpeedName, {Sources::Text::GetSaberSpeed, Sources::Text::SaberSpeedUI}},
    {Sources::Text::SpinometerName, {Sources::Text::GetSpinometer, Sources::Text::SpinometerUI}},
    {Sources::Text::FCPercentName, {Sources::Text::GetFCPercent, Sources::Text::FCPercentUI}},
};

std::vector<std::pair<std::string, std::pair<Types::SourceFn<float>, Types::SourceUIFn>>> Sources::shapes = {
    {Sources::Shape::StaticName, {Sources::Shape::GetStatic, Sources::Shape::StaticUI}},
    {Sources::Shape::ScoreName, {Sources::Shape::GetScore, Sources::Shape::ScoreUI}},
    {Sources::Shape::MultiplierName, {Sources::Shape::GetMultiplier, Sources::Shape::MultiplierUI}},
    {Sources::Shape::HealthName, {Sources::Shape::GetHealth, Sources::Shape::HealthUI}},
    {Sources::Shape::AverageCutName, {Sources::Shape::GetAverageCut, Sources::Shape::AverageCutUI}},
    {Sources::Shape::TimeName, {Sources::Shape::GetTime, Sources::Shape::TimeUI}},
    {Sources::Shape::NotesName, {Sources::Shape::GetNotes, Sources::Shape::NotesUI}},
};

std::vector<std::pair<std::string, std::pair<Types::SourceFn<UnityEngine::Color>, Types::SourceUIFn>>> Sources::colors = {
    {Sources::Color::StaticName, {Sources::Color::GetStatic, Sources::Color::StaticUI}},
    {Sources::Color::PlayerName, {Sources::Color::GetPlayer, Sources::Color::PlayerUI}},
    {Sources::Color::RankName, {Sources::Color::GetRank, Sources::Color::RankUI}},
    {Sources::Color::PersonalBestName, {Sources::Color::GetPersonalBest, Sources::Color::PersonalBestUI}},
    {Sources::Color::ComboName, {Sources::Color::GetCombo, Sources::Color::ComboUI}},
    {Sources::Color::MultiplierName, {Sources::Color::GetMultiplier, Sources::Color::MultiplierUI}},
    {Sources::Color::HealthName, {Sources::Color::GetHealth, Sources::Color::HealthUI}},
};

std::vector<std::pair<std::string, std::pair<Types::SourceFn<bool>, Types::SourceUIFn>>> Sources::enables = {
    {Sources::Enable::StaticName, {Sources::Enable::GetStatic, Sources::Enable::StaticUI}},
    {Sources::Enable::RankedName, {Sources::Enable::GetRanked, Sources::Enable::RankedUI}},
    {Sources::Enable::FullComboName, {Sources::Enable::GetFullCombo, Sources::Enable::FullComboUI}},
    {Sources::Enable::PercentageName, {Sources::Enable::GetPercentage, Sources::Enable::PercentageUI}},
    {Sources::Enable::FailedName, {Sources::Enable::GetFailed, Sources::Enable::FailedUI}},
};

static UnityEngine::UI::Graphic* CreateMultiplier(UnityEngine::GameObject*, UnparsedJSON);
static UnityEngine::UI::Graphic* CreateProgressBar(UnityEngine::GameObject*, UnparsedJSON);

std::map<std::string, std::vector<Sources::PremadeInfo>> Sources::premades = {
    {"",
     {{Options::BaseGameObjectStrings[(int) BaseGameGraphic::Objects::Multiplier], CreateMultiplier},
      {Options::BaseGameObjectStrings[(int) BaseGameGraphic::Objects::ProgressBar], CreateProgressBar}}},
};

static UnityEngine::UI::Graphic* CreateMultiplier(UnityEngine::GameObject* parent, UnparsedJSON) {
    auto ret = BaseGameGraphic::Create(parent->transform);
    ret->SetComponent((int) BaseGameGraphic::Objects::Multiplier);
    return ret;
}
static UnityEngine::UI::Graphic* CreateProgressBar(UnityEngine::GameObject* parent, UnparsedJSON) {
    auto ret = BaseGameGraphic::Create(parent->transform);
    ret->SetComponent((int) BaseGameGraphic::Objects::ProgressBar);
    return ret;
}

Sources::PremadeInfo* Sources::GetPremadeInfo(std::string const& mod, std::string const& name) {
    auto itr = premades.find(mod);
    if (itr != premades.end()) {
        for (auto& info : itr->second) {
            if (info.name == name)
                return &info;
        }
    }
    return nullptr;
}

std::vector<std::string_view> const Sources::AverageCutPartStrings = {
    "Preswing",
    "Postswing",
    "Accuracy",
    "All",
};
std::vector<std::string_view> const Sources::NotesDisplayStrings = {
    "Cut",
    "Remaining",
    "Cut Ratio",
    "Cut Percent",
};
std::vector<std::string_view> const Sources::PPLeaderboardStrings = {
    "ScoreSaber",
    "BeatLeader",
};
std::vector<std::string_view> const Sources::SaberSpeedModeStrings = {
    "Overall Average",
    "5 Second Highest",
};
std::vector<std::string_view> const Sources::SpinometerModeStrings = {
    "Average",
    "Highest",
};
std::vector<std::string_view> const Sources::RankedStatusLeaderboardStrings = {
    "ScoreSaber",
    "BeatLeader",
    "Either",
    "Both",
};

float Sources::Text::PP::GetRankedPP(int leaderboard) {
    int score = Internals::leftScore() + Internals::rightScore();
    int max = Internals::leftMaxScore() + Internals::rightMaxScore();
    float percent = max > 0 ? score / (double) max : 0.95;
    bool failed = Internals::health() <= 0;

    switch ((Leaderboards) leaderboard) {
        case Leaderboards::BeatLeader:
            return Qounters::PP::CalculateBL(percent, Internals::modifiers(), failed);
        case Leaderboards::ScoreSaber:
            return Qounters::PP::CalculateSS(percent, Internals::modifiers(), failed);
    }
    return 0;
}

bool Sources::Enable::Ranked::GetIsRanked(int leaderboards) {
    switch ((Leaderboards) leaderboards) {
        case Leaderboards::ScoreSaber:
            return PP::IsRankedSS();
        case Leaderboards::BeatLeader:
            return PP::IsRankedBL();
        case Leaderboards::Either:
            return PP::IsRankedSS() || PP::IsRankedBL();
        case Leaderboards::Both:
            return PP::IsRankedSS() && PP::IsRankedBL();
    }
    return false;
}

UnityEngine::Color Sources::Color::Player::GetColor(int setting) {
    if (!Internals::colors())
        return UnityEngine::Color::get_white();

    switch ((ColorSettings) setting) {
        case ColorSettings::LeftSaber:
            return Internals::colors()->saberAColor;
        case ColorSettings::RightSaber:
            return Internals::colors()->saberBColor;
        case ColorSettings::Lights1:
            return Internals::colors()->environmentColor0;
        case ColorSettings::Lights2:
            return Internals::colors()->environmentColor1;
        case ColorSettings::Walls:
            return Internals::colors()->obstaclesColor;
        case ColorSettings::Boost1:
            return Internals::colors()->environmentColor0Boost;
        case ColorSettings::Boost2:
            return Internals::colors()->environmentColor1Boost;
    }
    return UnityEngine::Color::get_white();
}

std::string Sources::Text::GetStatic(UnparsedJSON unparsed) {
    return unparsed.Parse<Static>().Input;
}
std::string Sources::Text::GetScore(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Score>();

    int score = Stats::GetScore(opts.Saber);
    if (opts.Percentage) {
        int max = Stats::GetMaxScore(opts.Saber);
        double ratio = max > 0 ? score / (double) max : 1;
        ratio *= 100;
        return Strings::FormatDecimals(ratio, opts.Decimals) + "%";
    } else {
        // spaces between every three digits, and pad zeroes if below 100
        auto number = fmt::format("{:03}", score);
        if (score < 1000)
            return number;
        size_t len = number.size();
        for (int i = 1; i <= (len - 1) / 3; i++) {
            size_t split = len - 3 * i;
            number = number.substr(0, split) + " " + number.substr(split);
        }
        return number;
    }
}
std::string Sources::Text::GetRank(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Rank>();

    int score = Stats::GetScore(opts.Saber);
    int max = Stats::GetMaxScore(opts.Saber);
    if (max == 0)
        return "SS";
    if (score == max)
        return "SSS";

    score *= Stats::GetModifierMultiplier(true, true);
    // if "Positive Modifiers" is enabled, we want an 82% with a +10% modifier to be an SS
    // otherwise, we want that to be an S rank (same with negative)
    max *= Stats::GetModifierMultiplier(!opts.PositiveModifiers, !opts.NegativeModifiers);

    if (max == 0)
        return "E";

    double ratio = score / (double) max;
    if (ratio >= 0.9)
        return "SS";
    if (ratio >= 0.8)
        return "S";
    if (ratio >= 0.65)
        return "A";
    if (ratio >= 0.5)
        return "B";
    if (ratio >= 0.35)
        return "C";
    if (ratio >= 0.2)
        return "D";
    return "E";
}
std::string Sources::Text::GetPersonalBest(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<PersonalBest>();

    int best = Stats::GetBestScore();
    if (best == -1) {
        if (opts.HideFirstScore)
            return opts.Label ? "PB: --" : "--";
        else
            best = 0;
    }
    int max = Stats::GetSongMaxScore();
    std::string text;
    if (opts.Percentage) {
        double ratio;
        if (Environment::InSettings())
            ratio = Playtest::GetOverridePBRatio();
        else
            ratio = max > 0 ? best / (Stats::GetModifierMultiplier(true, true) * max) : 1;
        text = Strings::FormatDecimals(ratio * 100, opts.Decimals) + "%";
    } else
        text = Environment::InSettings() && max == 1 ? "0" : std::to_string(best);
    return opts.Label ? "PB: " + text : text;
}
std::string Sources::Text::GetCombo(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Combo>();

    return std::to_string(Stats::GetCombo(opts.Saber));
}
std::string Sources::Text::GetMultiplier(UnparsedJSON unparsed) {
    return std::to_string(Stats::GetMultiplier());
}
std::string Sources::Text::GetHealth(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Health>();

    float health = Stats::GetHealth();
    if (opts.Percentage) {
        health *= 100;
        return Strings::FormatDecimals(health, opts.Decimals) + "%";
    } else
        return Strings::FormatDecimals(health, opts.Decimals);
}
std::string Sources::Text::GetTime(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Time>();

    float seconds = (int) Stats::GetSongTime();
    if (opts.Remaining)
        seconds = Stats::GetSongLength() - seconds;

    if (opts.Percentage) {
        float ratio = seconds / Stats::GetSongLength();
        ratio *= 100;
        return std::to_string((int) ratio) + "%";
    } else
        return Strings::SecondsToString((int) seconds);
}
std::string Sources::Text::GetAverageCut(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<AverageCut>();

    float total = 0;
    if (opts.Part == (int) AverageCut::Parts::Pre || opts.Part == (int) AverageCut::Parts::All)
        total += Stats::GetPreSwing(opts.Saber);
    if (opts.Part == (int) AverageCut::Parts::Post || opts.Part == (int) AverageCut::Parts::All)
        total += Stats::GetPostSwing(opts.Saber);
    if (opts.Part == (int) AverageCut::Parts::Acc || opts.Part == (int) AverageCut::Parts::All)
        total += Stats::GetAccuracy(opts.Saber);

    return Strings::FormatDecimals(total, opts.Decimals);
}
std::string Sources::Text::GetTimeDependence(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<TimeDependence>();

    double ret = Stats::GetTimeDependence(opts.Saber);
    ret *= std::pow(10, opts.DecimalOffset);
    return Strings::FormatDecimals(ret, opts.Decimals);
}
std::string Sources::Text::GetFails(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Fails>();

    return std::to_string(opts.Restarts ? Stats::GetRestarts() : Stats::GetFails());
}
std::string Sources::Text::GetMistakes(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Mistakes>();

    int total = 0;
    if (opts.Misses)
        total += Stats::GetNotesMissed(opts.Saber);
    if (opts.BadCuts)
        total += Stats::GetNotesBadCut(opts.Saber);
    if (opts.Bombs)
        total += Stats::GetBombsHit(opts.Saber);
    if (opts.Walls && opts.Saber == (int) Types::Sabers::Both)
        total += Stats::GetWallsHit();

    return std::to_string(total);
}
std::string Sources::Text::GetNotes(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Notes>();

    if (opts.Display == (int) Notes::Displays::Remaining)
        return std::to_string(Stats::GetSongNotes(opts.Saber) - Stats::GetTotalNotes(opts.Saber));

    int notes = Stats::GetNotesCut(opts.Saber);
    if (opts.Display == (int) Notes::Displays::Ratio) {
        return fmt::format("{} / {}", notes, Stats::GetTotalNotes(opts.Saber));
    } else if (opts.Display == (int) Notes::Displays::Percent) {
        float ratio = notes / (float) Stats::GetTotalNotes(opts.Saber);
        ratio *= 100;
        return Strings::FormatDecimals(ratio, opts.Decimals) + "%";
    } else
        return std::to_string(notes);
}
std::string Sources::Text::GetPP(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<PP>();

    return Strings::FormatDecimals(PP::GetRankedPP(opts.Leaderboard), opts.Decimals) + " PP";
}
std::string Sources::Text::GetSaberSpeed(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<SaberSpeed>();

    float speed = 0;
    if (opts.Mode == (int) SaberSpeed::Modes::Average)
        speed = Stats::GetAverageSpeed(opts.Saber);
    else if (opts.Mode == (int) SaberSpeed::Modes::Best5Seconds)
        speed = Stats::GetBestSpeed5Secs(opts.Saber);

    return Strings::FormatDecimals(speed, opts.Decimals);
}
std::string Sources::Text::GetSpinometer(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Spinometer>();

    float anglePerSec = 0;
    if (opts.Mode == (int) Spinometer::Modes::Average)
        anglePerSec = Stats::GetLastSecAngle(opts.Saber);
    else if (opts.Mode == (int) Spinometer::Modes::Highest)
        anglePerSec = Stats::GetHighestSecAngle(opts.Saber);

    return std::to_string((int) anglePerSec);
}
std::string Sources::Text::GetFCPercent(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<FCPercent>();

    int score = Stats::GetFCScore(opts.Saber);
    int max = Stats::GetMaxScore(opts.Saber);
    float percent = max > 0 ? score / (double) max : 1;
    percent *= 100;
    return Strings::FormatDecimals(percent, opts.Decimals) + "%";
}

float Sources::Shape::GetStatic(UnparsedJSON unparsed) {
    return unparsed.Parse<Static>().Input;
}
float Sources::Shape::GetScore(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Score>();

    int score = Stats::GetScore(opts.Saber);
    int max = Stats::GetMaxScore(opts.Saber);
    return max > 0 ? score / (double) max : 1;
}
float Sources::Shape::GetMultiplier(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Multiplier>();

    return Stats::GetMultiplierProgress(opts.Absolute);
}
float Sources::Shape::GetHealth(UnparsedJSON unparsed) {
    return Stats::GetHealth();
}
float Sources::Shape::GetTime(UnparsedJSON unparsed) {
    return Stats::GetSongTime() / Stats::GetSongLength();
}
float Sources::Shape::GetAverageCut(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<AverageCut>();

    float total = 0;
    int max = 0;
    if (opts.Part == (int) AverageCut::Parts::Pre || opts.Part == (int) AverageCut::Parts::All) {
        total += Stats::GetPreSwing(opts.Saber);
        max += 70;
    }
    if (opts.Part == (int) AverageCut::Parts::Post || opts.Part == (int) AverageCut::Parts::All) {
        total += Stats::GetPostSwing(opts.Saber);
        max += 30;
    }
    if (opts.Part == (int) AverageCut::Parts::Acc || opts.Part == (int) AverageCut::Parts::All) {
        total += Stats::GetAccuracy(opts.Saber);
        max += 15;
    }

    return total / max;
}
float Sources::Shape::GetNotes(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Notes>();

    return Stats::GetNotesCut(opts.Saber) / (double) Stats::GetTotalNotes(opts.Saber);
}

UnityEngine::Color Sources::Color::GetStatic(UnparsedJSON unparsed) {
    return unparsed.Parse<Static>().Input;
}
UnityEngine::Color Sources::Color::GetPlayer(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Player>();

    return Player::GetColor(opts.Setting);
}
UnityEngine::Color Sources::Color::GetRank(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Rank>();

    int score = Stats::GetScore(opts.Saber);
    int max = Stats::GetMaxScore(opts.Saber);
    if (max == 0)
        return opts.SS;
    if (score == max)
        return opts.SSS;

    score *= Stats::GetModifierMultiplier(true, true);
    // if "Positive Modifiers" is enabled, we want an 82% with a +10% modifier to be an SS
    // otherwise, we want that to be an S rank (same with negative)
    max *= Stats::GetModifierMultiplier(!opts.PositiveModifiers, !opts.NegativeModifiers);

    if (max == 0)
        return opts.E;

    double ratio = score / (double) max;
    if (ratio >= 0.9)
        return opts.SS;
    if (ratio >= 0.8)
        return opts.S;
    if (ratio >= 0.65)
        return opts.A;
    if (ratio >= 0.5)
        return opts.B;
    if (ratio >= 0.35)
        return opts.C;
    if (ratio >= 0.2)
        return opts.D;
    return opts.E;
}
UnityEngine::Color Sources::Color::GetPersonalBest(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<PersonalBest>();

    double best = Stats::GetBestScore();
    int songMax = Stats::GetSongMaxScore();
    double current = Stats::GetScore((int) Types::Sabers::Both);
    // PB modifiers would be applied to best score
    current *= Stats::GetModifierMultiplier(true, true);
    int max = Stats::GetMaxScore((int) Types::Sabers::Both);
    double bestRatio = songMax > 0 ? best / songMax : 1;
    double ratio = max > 0 ? current / max : 1;

    return ratio >= bestRatio ? opts.Better : opts.Worse;
}
UnityEngine::Color Sources::Color::GetCombo(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Combo>();

    return Stats::GetFullCombo(opts.Saber) ? opts.Full : opts.NonFull;
}
UnityEngine::Color Sources::Color::GetMultiplier(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Multiplier>();

    switch (Stats::GetMultiplier()) {
        case 1:
            return opts.One;
        case 2:
            return opts.Two;
        case 4:
            return opts.Four;
        default:
            return opts.Eight;
    }
}
UnityEngine::Color Sources::Color::GetHealth(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Health>();

    float health = Stats::GetHealth();

    if (Stats::GetFullCombo((int) Types::Sabers::Both) || health == 1)
        return opts.Full;
    else if (health >= 0.5)
        return opts.AboveHalf;
    else
        return opts.BelowHalf;
}

bool Sources::Enable::GetStatic(UnparsedJSON unparsed) {
    return true;
}
bool Sources::Enable::GetRanked(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Ranked>();

    return Ranked::GetIsRanked(opts.Leaderboard);
}
bool Sources::Enable::GetFullCombo(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<FullCombo>();

    return Stats::GetFullCombo(opts.Saber);
}
bool Sources::Enable::GetPercentage(UnparsedJSON unparsed) {
    auto opts = unparsed.Parse<Percentage>();

    int score = Stats::GetScore(opts.Saber);
    int max = Stats::GetMaxScore(opts.Saber);
    float percent = max > 0 ? score / (double) max : 1;
    return percent * 100 >= opts.Percent;
}
bool Sources::Enable::GetFailed(UnparsedJSON unparsed) {
    return Stats::GetHealth() == 0;
}
