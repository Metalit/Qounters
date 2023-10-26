#include "sources.hpp"
#include "sourceui.hpp"
#include "game.hpp"
#include "utils.hpp"
#include "pp.hpp"
#include "environment.hpp"

// TODO: Spinometer

using namespace Qounters;

std::vector<std::pair<std::string, std::pair<SourceFn<std::string>, SourceUIFn>>> Qounters::textSources = {
    {"Static", {TextSource::GetStatic, TextSource::StaticUI}},
    {"Score", {TextSource::GetScore, TextSource::ScoreUI}},
    {"Rank", {TextSource::GetRank, TextSource::RankUI}},
    {"Personal Best", {TextSource::GetPersonalBest, TextSource::PersonalBestUI}},
    {"Combo", {TextSource::GetCombo, TextSource::ComboUI}},
    {"Multiplier", {TextSource::GetMultiplier, TextSource::MultiplierUI}},
    {"Health", {TextSource::GetHealth, TextSource::HealthUI}},
    {"Time", {TextSource::GetTime, TextSource::TimeUI}},
    {"Average Cut", {TextSource::GetAverageCut, TextSource::AverageCutUI}},
    {"Fails", {TextSource::GetFails, TextSource::FailsUI}},
    {"Mistakes", {TextSource::GetMistakes, TextSource::MistakesUI}},
    {"Notes", {TextSource::GetNotes, TextSource::NotesUI}},
    {"PP", {TextSource::GetPP, TextSource::PPUI}},
    {"Saber Speed", {TextSource::GetSaberSpeed, TextSource::SaberSpeedUI}},
    {"Spinometer", {TextSource::GetSpinometer, TextSource::SpinometerUI}},
};

std::vector<std::pair<std::string, std::pair<SourceFn<float>, SourceUIFn>>> Qounters::shapeSources = {
    {"Static", {ShapeSource::GetStatic, ShapeSource::StaticUI}},
    {"Score", {ShapeSource::GetScore, ShapeSource::ScoreUI}},
    {"Multiplier", {ShapeSource::GetMultiplier, ShapeSource::MultiplierUI}},
    {"Health", {ShapeSource::GetHealth, ShapeSource::HealthUI}},
    {"Average Cut", {ShapeSource::GetAverageCut, ShapeSource::AverageCutUI}},
    {"Time", {ShapeSource::GetTime, ShapeSource::TimeUI}},
    {"Notes", {ShapeSource::GetNotes, ShapeSource::NotesUI}},
};

std::vector<std::pair<std::string, std::pair<SourceFn<UnityEngine::Color>, SourceUIFn>>> Qounters::colorSources = {
    {"Static", {ColorSource::GetStatic, ColorSource::StaticUI}},
    {"Player", {ColorSource::GetPlayer, ColorSource::PlayerUI}},
    {"Rank", {ColorSource::GetRank, ColorSource::RankUI}},
    {"Personal Best", {ColorSource::GetPersonalBest, ColorSource::PersonalBestUI}},
    {"Combo", {ColorSource::GetCombo, ColorSource::ComboUI}},
    {"Multiplier", {ColorSource::GetMultiplier, ColorSource::MultiplierUI}},
    {"Health", {ColorSource::GetHealth, ColorSource::HealthUI}},
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

        float pos = Game::GetPositiveModifiers();
        float neg = Game::GetNegativeModifiers();
        score *= (1 + pos - neg);

        if (!opts.PositiveModifiers)
            pos = 0;
        if (!opts.NegativeModifiers)
            neg = 0;
        max *= (1 + pos - neg);

        if (max == 0)
            return "E";

        double ratio = score / (double) max;
        if (ratio == 1)
            return "SSS";
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
            double ratio = max > 0 ? best / (double) max : 1;
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
            return string_format("%i / %i", notes, Game::GetTotalNotes(opts.Saber));
        } else if (opts.Display == (int) Notes::Displays::Percent) {
            float ratio = notes / (float) Game::GetTotalNotes(opts.Saber);
            ratio *= 100;
            return Utils::FormatDecimals(ratio, opts.Decimals) + "%";
        } else
            return std::to_string(notes);
    }
    std::string GetPP(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<PP>();

        if (opts.HideUnranked && InSettingsEnvironment())
            return "Hidden PP";

        if (opts.Source == (int) PP::Sources::BeatLeader && opts.HideUnranked && !Qounters::PP::IsRankedBL())
            return "";
        else if (opts.Source == (int) PP::Sources::ScoreSaber && opts.HideUnranked && !Qounters::PP::IsRankedSS())
            return "";

        int score = Game::GetScore((int) Sabers::Both);
        int max = Game::GetMaxScore((int) Sabers::Both);
        float percent = max > 0 ? score / (double) max : 0.95;
        bool failed = Game::GetHealth() == 0;

        float pp = 0;
        if (opts.Source == (int) PP::Sources::BeatLeader)
            pp = Qounters::PP::CalculateBL(percent, failed);
        else if (opts.Source == (int) PP::Sources::ScoreSaber)
            pp = Qounters::PP::CalculateSS(percent, failed);

        return Utils::FormatDecimals(pp, opts.Decimals) + " PP";
    }
    std::string GetSaberSpeed(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<SaberSpeed>();

        float speed = 0;
        if (opts.Mode == (int) SaberSpeed::Modes::Average)
            speed = Game::GetAverageSpeed(opts.Saber);
        if (opts.Mode == (int) SaberSpeed::Modes::Best5Seconds)
            speed = Game::GetBestSpeed5Secs(opts.Saber);

        return Utils::FormatDecimals(speed, opts.Decimals);
    }
    std::string GetSpinometer(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Spinometer>();

        return "spin spin spin spin";
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

        float pos = Game::GetPositiveModifiers();
        float neg = Game::GetNegativeModifiers();
        score *= (1 + pos - neg);

        if (!opts.PositiveModifiers)
            pos = 0;
        if (!opts.NegativeModifiers)
            neg = 0;
        max *= (1 + pos - neg);

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
        int current = Game::GetScore((int) Sabers::Both);
        int max = Game::GetMaxScore((int) Sabers::Both);
        double bestRatio = songMax > 0 ? best / (double) songMax : 1;
        double ratio = max > 0 ? current / (double) max : 1;

        return ratio >= bestRatio ? opts.Better : opts.Worse;
    }
    UnityEngine::Color GetCombo(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Combo>();

        return Game::GetFullCombo(opts.Saber) ? opts.Full : opts.NonFull;
    }
    UnityEngine::Color GetMultiplier(UnparsedJSON unparsed) {
        auto opts = unparsed.Parse<Multiplier>();

        switch (Game::GetMultiplier()) {
            case 1: return opts.One;
            case 2: return opts.Two;
            case 4: return opts.Four;
            default: return opts.Eight;
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
