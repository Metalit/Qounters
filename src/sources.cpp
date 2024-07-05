#include "sources.hpp"

#include "environment.hpp"
#include "game.hpp"
#include "internals.hpp"
#include "pp.hpp"
#include "sourceui.hpp"
#include "utils.hpp"

using namespace Qounters;

std::vector<std::pair<std::string, std::pair<SourceFn<std::string>, SourceUIFn>>> Qounters::textSources = {
    {TextSource::StaticName, {TextSource::GetStatic, TextSource::StaticUI}},
    {TextSource::ScoreName, {TextSource::GetScore, TextSource::ScoreUI}},
    {TextSource::RankName, {TextSource::GetRank, TextSource::RankUI}},
    {TextSource::PersonalBestName, {TextSource::GetPersonalBest, TextSource::PersonalBestUI}},
    {TextSource::ComboName, {TextSource::GetCombo, TextSource::ComboUI}},
    {TextSource::MultiplierName, {TextSource::GetMultiplier, TextSource::MultiplierUI}},
    {TextSource::HealthName, {TextSource::GetHealth, TextSource::HealthUI}},
    {TextSource::TimeName, {TextSource::GetTime, TextSource::TimeUI}},
    {TextSource::AverageCutName, {TextSource::GetAverageCut, TextSource::AverageCutUI}},
    {TextSource::TimeDependenceName, {TextSource::GetTimeDependence, TextSource::TimeDependenceUI}},
    {TextSource::FailsName, {TextSource::GetFails, TextSource::FailsUI}},
    {TextSource::MistakesName, {TextSource::GetMistakes, TextSource::MistakesUI}},
    {TextSource::NotesName, {TextSource::GetNotes, TextSource::NotesUI}},
    {TextSource::PPName, {TextSource::GetPP, TextSource::PPUI}},
    {TextSource::SaberSpeedName, {TextSource::GetSaberSpeed, TextSource::SaberSpeedUI}},
    {TextSource::SpinometerName, {TextSource::GetSpinometer, TextSource::SpinometerUI}},
    {TextSource::FCPercentName, {TextSource::GetFCPercent, TextSource::FCPercentUI}},
};

std::vector<std::pair<std::string, std::pair<SourceFn<float>, SourceUIFn>>> Qounters::shapeSources = {
    {ShapeSource::StaticName, {ShapeSource::GetStatic, ShapeSource::StaticUI}},
    {ShapeSource::ScoreName, {ShapeSource::GetScore, ShapeSource::ScoreUI}},
    {ShapeSource::MultiplierName, {ShapeSource::GetMultiplier, ShapeSource::MultiplierUI}},
    {ShapeSource::HealthName, {ShapeSource::GetHealth, ShapeSource::HealthUI}},
    {ShapeSource::AverageCutName, {ShapeSource::GetAverageCut, ShapeSource::AverageCutUI}},
    {ShapeSource::TimeName, {ShapeSource::GetTime, ShapeSource::TimeUI}},
    {ShapeSource::NotesName, {ShapeSource::GetNotes, ShapeSource::NotesUI}},
};

std::vector<std::pair<std::string, std::pair<SourceFn<UnityEngine::Color>, SourceUIFn>>> Qounters::colorSources = {
    {ColorSource::StaticName, {ColorSource::GetStatic, ColorSource::StaticUI}},
    {ColorSource::PlayerName, {ColorSource::GetPlayer, ColorSource::PlayerUI}},
    {ColorSource::RankName, {ColorSource::GetRank, ColorSource::RankUI}},
    {ColorSource::PersonalBestName, {ColorSource::GetPersonalBest, ColorSource::PersonalBestUI}},
    {ColorSource::ComboName, {ColorSource::GetCombo, ColorSource::ComboUI}},
    {ColorSource::MultiplierName, {ColorSource::GetMultiplier, ColorSource::MultiplierUI}},
    {ColorSource::HealthName, {ColorSource::GetHealth, ColorSource::HealthUI}},
};

std::vector<std::pair<std::string, std::pair<SourceFn<bool>, SourceUIFn>>> Qounters::enableSources = {
    {EnableSource::StaticName, {EnableSource::GetStatic, EnableSource::StaticUI}},
    {EnableSource::RankedName, {EnableSource::GetRanked, EnableSource::RankedUI}},
    {EnableSource::FullComboName, {EnableSource::GetFullCombo, EnableSource::FullComboUI}},
    {EnableSource::PercentageName, {EnableSource::GetPercentage, EnableSource::PercentageUI}},
    {EnableSource::FailedName, {EnableSource::GetFailed, EnableSource::FailedUI}},
};

std::vector<std::string_view> const Qounters::AverageCutPartStrings = {
    "Preswing",
    "Postswing",
    "Accuracy",
    "All",
};
std::vector<std::string_view> const Qounters::NotesDisplayStrings = {
    "Cut",
    "Remaining",
    "Cut Ratio",
    "Cut Percent",
};
std::vector<std::string_view> const Qounters::PPLeaderboardStrings = {
    "ScoreSaber",
    "BeatLeader",
};
std::vector<std::string_view> const Qounters::SaberSpeedModeStrings = {
    "Average",
    "5 Second Highest",
};
std::vector<std::string_view> const Qounters::SpinometerModeStrings = {
    "Average",
    "Highest",
};
std::vector<std::string_view> const Qounters::RankedStatusLeaderboardStrings = {
    "ScoreSaber",
    "BeatLeader",
    "Either",
    "Both",
};

namespace Qounters::TextSource {
    std::string GetStatic(UnparsedJSON unparsed) {
        return unparsed.Parse<Static>().Input;
    }
    std::string GetScore(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Score>();

        int score = Game::GetScore(opts.Saber);
        if (opts.Percentage) {
            int max = Game::GetMaxScore(opts.Saber);
            double ratio = max > 0 ? score / (double) max : 1;
            ratio *= 100;
            return Utils::FormatDecimals(ratio, opts.Decimals) + "%";
        } else
            return std::to_string(score);
    }
    std::string GetRank(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Rank>();

        int score = Game::GetScore(opts.Saber);
        int max = Game::GetMaxScore(opts.Saber);
        if (max == 0)
            return "SS";
        if (score == max)
            return "SSS";

        score *= Game::GetModifierMultiplier(true, true);
        // if "Positive Modifiers" is enabled, we want an 82% with a +10% modifier to be an SS
        // otherwise, we want that to be an S rank (same with negative)
        max *= Game::GetModifierMultiplier(!opts.PositiveModifiers, !opts.NegativeModifiers);

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
            return "C";
        return "E";
    }
    std::string GetPersonalBest(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<PersonalBest>();

        int best = Game::GetBestScore();
        if (best == -1) {
            if (opts.HideFirstScore)
                return opts.Label ? "PB: --" : "--";
            else
                best = 0;
        }
        if (opts.Percentage) {
            int max = Game::GetSongMaxScore();
            double ratio = max > 0 ? best / ((double) Game::GetModifierMultiplier(true, true) * max) : 1;
            ratio *= 100;
            std::string ret = Utils::FormatDecimals(ratio, opts.Decimals) + "%";
            return opts.Label ? "PB: " + ret : ret;
        } else
            return opts.Label ? "PB: " + std::to_string(best) : std::to_string(best);
    }
    std::string GetCombo(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Combo>();

        return std::to_string(Game::GetCombo(opts.Saber));
    }
    std::string GetMultiplier(UnparsedJSON unparsed) {
        return std::to_string(Game::GetMultiplier());
    }
    std::string GetHealth(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Health>();

        float health = Game::GetHealth();
        if (opts.Percentage) {
            health *= 100;
            return Utils::FormatDecimals(health, opts.Decimals) + "%";
        } else
            return Utils::FormatDecimals(health, opts.Decimals);
    }
    std::string GetTime(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Time>();

        float seconds = (int) Game::GetSongTime();
        if (opts.Remaining)
            seconds = Game::GetSongLength() - seconds;

        if (opts.Percentage) {
            float ratio = seconds / Game::GetSongLength();
            ratio *= 100;
            return std::to_string((int) ratio) + "%";
        } else
            return Utils::SecondsToString((int) seconds);
    }
    std::string GetAverageCut(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<AverageCut>();

        float total = 0;
        if (opts.Part == (int) AverageCut::Parts::Pre || opts.Part == (int) AverageCut::Parts::All)
            total += Game::GetPreSwing(opts.Saber);
        if (opts.Part == (int) AverageCut::Parts::Post || opts.Part == (int) AverageCut::Parts::All)
            total += Game::GetPostSwing(opts.Saber);
        if (opts.Part == (int) AverageCut::Parts::Acc || opts.Part == (int) AverageCut::Parts::All)
            total += Game::GetAccuracy(opts.Saber);

        return Utils::FormatDecimals(total, opts.Decimals);
    }
    std::string GetTimeDependence(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<TimeDependence>();

        double ret = Game::GetTimeDependence(opts.Saber);
        ret *= std::pow(10, opts.DecimalOffset);
        return Utils::FormatDecimals(ret, opts.Decimals);
    }
    std::string GetFails(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Fails>();

        return std::to_string(opts.Restarts ? Game::GetRestarts() : Game::GetFails());
    }
    std::string GetMistakes(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Mistakes>();

        int total = 0;
        if (opts.Misses)
            total += Game::GetNotesMissed(opts.Saber);
        if (opts.BadCuts)
            total += Game::GetNotesBadCut(opts.Saber);
        if (opts.Bombs)
            total += Game::GetBombsHit(opts.Saber);
        if (opts.Walls && opts.Saber == (int) Sabers::Both)
            total += Game::GetWallsHit();

        return std::to_string(total);
    }
    std::string GetNotes(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Notes>();

        int notes = Game::GetNotesCut(opts.Saber);
        if (opts.Display == (int) Notes::Displays::Remaining)
            notes = Game::GetTotalNotes(opts.Saber) - notes;

        if (opts.Display == (int) Notes::Displays::Ratio) {
            return fmt::format("{} / {}", notes, Game::GetTotalNotes(opts.Saber));
        } else if (opts.Display == (int) Notes::Displays::Percent) {
            float ratio = notes / (float) Game::GetTotalNotes(opts.Saber);
            ratio *= 100;
            return Utils::FormatDecimals(ratio, opts.Decimals) + "%";
        } else
            return std::to_string(notes);
    }
    std::string GetPP(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<PP>();

        int score = Game::GetScore((int) Sabers::Both);
        int max = Game::GetMaxScore((int) Sabers::Both);
        float percent = max > 0 ? score / (double) max : 0.95;
        bool failed = Game::GetHealth() == 0;

        float pp = 0;
        if (opts.Leaderboard == (int) PP::Leaderboards::BeatLeader)
            pp = Qounters::PP::CalculateBL(percent, modifiers, failed);
        else if (opts.Leaderboard == (int) PP::Leaderboards::ScoreSaber)
            pp = Qounters::PP::CalculateSS(percent, modifiers, failed);

        return Utils::FormatDecimals(pp, opts.Decimals) + " PP";
    }
    std::string GetSaberSpeed(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<SaberSpeed>();

        float speed = 0;
        if (opts.Mode == (int) SaberSpeed::Modes::Average)
            speed = Game::GetAverageSpeed(opts.Saber);
        else if (opts.Mode == (int) SaberSpeed::Modes::Best5Seconds)
            speed = Game::GetBestSpeed5Secs(opts.Saber);

        return Utils::FormatDecimals(speed, opts.Decimals);
    }
    std::string GetSpinometer(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Spinometer>();

        float anglePerSec = 0;
        if (opts.Mode == (int) Spinometer::Modes::Average)
            anglePerSec = Game::GetLastSecAngle(opts.Saber);
        else if (opts.Mode == (int) Spinometer::Modes::Highest)
            anglePerSec = Game::GetHighestSecAngle(opts.Saber);

        return std::to_string((int) anglePerSec);
    }
    std::string GetFCPercent(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<FCPercent>();

        int score = Game::GetFCScore(opts.Saber);
        int max = Game::GetMaxScore(opts.Saber);
        float percent = max > 0 ? score / (double) max : 1;
        percent *= 100;
        return Utils::FormatDecimals(percent, opts.Decimals) + "%";
    }
}

namespace Qounters::ShapeSource {
    float GetStatic(UnparsedJSON unparsed) {
        return unparsed.Parse<Static>().Input;
    }
    float GetScore(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Score>();

        int score = Game::GetScore(opts.Saber);
        int max = Game::GetMaxScore(opts.Saber);
        return max > 0 ? score / (double) max : 1;
    }
    float GetMultiplier(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Multiplier>();

        return Game::GetMultiplierProgress(opts.Absolute);
    }
    float GetHealth(UnparsedJSON unparsed) {
        return Game::GetHealth();
    }
    float GetTime(UnparsedJSON unparsed) {
        return Game::GetSongTime() / Game::GetSongLength();
    }
    float GetAverageCut(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<AverageCut>();

        float total = 0;
        int max = 0;
        if (opts.Part == (int) AverageCut::Parts::Pre || opts.Part == (int) AverageCut::Parts::All) {
            total += Game::GetPreSwing(opts.Saber);
            max += 70;
        }
        if (opts.Part == (int) AverageCut::Parts::Post || opts.Part == (int) AverageCut::Parts::All) {
            total += Game::GetPostSwing(opts.Saber);
            max += 30;
        }
        if (opts.Part == (int) AverageCut::Parts::Acc || opts.Part == (int) AverageCut::Parts::All) {
            total += Game::GetAccuracy(opts.Saber);
            max += 15;
        }

        return total / max;
    }
    float GetNotes(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Notes>();

        return Game::GetNotesCut(opts.Saber) / (double) Game::GetTotalNotes(opts.Saber);
    }
}

namespace Qounters::ColorSource {
    UnityEngine::Color GetStatic(UnparsedJSON unparsed) {
        return unparsed.Parse<Static>().Input;
    }
    UnityEngine::Color GetPlayer(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Player>();

        return Game::GetColor(opts.Setting);
    }
    UnityEngine::Color GetRank(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Rank>();

        int score = Game::GetScore(opts.Saber);
        int max = Game::GetMaxScore(opts.Saber);
        if (max == 0)
            return opts.SS;

        score *= Game::GetModifierMultiplier(true, true);
        // if "Positive Modifiers" is enabled, we want an 82% with a +10% modifier to be an SS
        // otherwise, we want that to be an S rank (same with negative)
        max *= Game::GetModifierMultiplier(!opts.PositiveModifiers, !opts.NegativeModifiers);

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
            return opts.C;
        return opts.E;
    }
    UnityEngine::Color GetPersonalBest(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<PersonalBest>();

        int best = Game::GetBestScore();
        int songMax = Game::GetSongMaxScore();
        double current = Game::GetScore((int) Sabers::Both);
        // PB modifiers would be applied to best score
        current *= Game::GetModifierMultiplier(true, true);
        int max = Game::GetMaxScore((int) Sabers::Both);
        double bestRatio = songMax > 0 ? best / (double) songMax : 1;
        double ratio = max > 0 ? current / max : 1;

        return ratio >= bestRatio ? opts.Better : opts.Worse;
    }
    UnityEngine::Color GetCombo(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Combo>();

        return Game::GetFullCombo(opts.Saber) ? opts.Full : opts.NonFull;
    }
    UnityEngine::Color GetMultiplier(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Multiplier>();

        switch (Game::GetMultiplier()) {
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
    UnityEngine::Color GetHealth(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Health>();

        float health = Game::GetHealth();

        if (Game::GetFullCombo((int) Sabers::Both) || health == 1)
            return opts.Full;
        else if (health >= 0.5)
            return opts.AboveHalf;
        else
            return opts.BelowHalf;
    }
}

namespace Qounters::EnableSource {
    bool GetStatic(UnparsedJSON unparsed) {
        return true;
    }
    bool GetRanked(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Ranked>();

        switch ((Ranked::Leaderboards) opts.Leaderboard) {
            case Ranked::Leaderboards::ScoreSaber:
                return PP::IsRankedSS();
            case Ranked::Leaderboards::BeatLeader:
                return PP::IsRankedBL();
            case Ranked::Leaderboards::Either:
                return PP::IsRankedSS() || PP::IsRankedBL();
            case Ranked::Leaderboards::Both:
                return PP::IsRankedSS() && PP::IsRankedBL();
        }
    }
    bool GetFullCombo(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<FullCombo>();

        return Game::GetFullCombo(opts.Saber);
    }
    bool GetPercentage(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Percentage>();

        int score = Game::GetScore(opts.Saber);
        int max = Game::GetMaxScore(opts.Saber);
        float percent = max > 0 ? score / (double) max : 1;
        return percent * 100 >= opts.Percent;
    }
    bool GetFailed(UnparsedJSON unparsed) {
        return Game::GetHealth() == 0;
    }
}
