#pragma once

#include "UnityEngine/GameObject.hpp"
#include "config-utils/shared/config-utils.hpp"
#include "types.hpp"

namespace Qounters::Sources {
    extern std::vector<std::pair<std::string, std::pair<Types::SourceFn<std::string>, Types::SourceUIFn>>> texts;
    extern std::vector<std::pair<std::string, std::pair<Types::SourceFn<float>, Types::SourceUIFn>>> shapes;
    extern std::vector<std::pair<std::string, std::pair<Types::SourceFn<UnityEngine::Color>, Types::SourceUIFn>>> colors;
    extern std::vector<std::pair<std::string, std::pair<Types::SourceFn<bool>, Types::SourceUIFn>>> enables;

    struct PremadeInfo {
        std::string name;
        Types::PremadeFn creation;
        Types::PremadeUIFn uiFunction;
        Types::PremadeUpdateFn update;
        PremadeInfo(
            std::string_view name, Types::PremadeFn creation, Types::PremadeUIFn uiFunction = nullptr, Types::PremadeUpdateFn update = nullptr
        ) :
            name(name),
            creation(creation),
            uiFunction(uiFunction),
            update(update) {}
    };
    // map with mod name for ordering purposes
    extern std::map<std::string, std::vector<PremadeInfo>> premades;
    PremadeInfo* GetPremadeInfo(std::string const& mod, std::string const& name);

    template <class T>
    T GetSource(std::vector<std::pair<std::string, T>> sourceVec, std::string source) {
        for (auto& [str, ret] : sourceVec) {
            if (str == source)
                return ret;
        }
        return {};
    }

    template <class T>
    inline void Register(
        std::vector<std::pair<std::string, std::pair<T, Types::SourceUIFn>>> sourceVec, std::string source, T sourceFn, Types::SourceUIFn sourceUIFn
    ) {
        sourceVec.emplace_back(source, std::make_pair(sourceFn, sourceUIFn));
    }

    inline void RegisterText(std::string sourceName, Types::SourceFn<std::string> sourceFn, Types::SourceUIFn sourceUIFn) {
        Register(texts, sourceName, sourceFn, sourceUIFn);
    }
    inline void RegisterShape(std::string sourceName, Types::SourceFn<float> sourceFn, Types::SourceUIFn sourceUIFn) {
        Register(shapes, sourceName, sourceFn, sourceUIFn);
    }
    inline void RegisterColor(std::string sourceName, Types::SourceFn<UnityEngine::Color> sourceFn, Types::SourceUIFn sourceUIFn) {
        Register(colors, sourceName, sourceFn, sourceUIFn);
    }
    inline void RegisterEnable(std::string sourceName, Types::SourceFn<bool> sourceFn, Types::SourceUIFn sourceUIFn) {
        Register(enables, sourceName, sourceFn, sourceUIFn);
    }

    extern std::vector<std::string_view> const AverageCutPartStrings;
    extern std::vector<std::string_view> const NotesDisplayStrings;
    extern std::vector<std::string_view> const PPLeaderboardStrings;
    extern std::vector<std::string_view> const SaberSpeedModeStrings;
    extern std::vector<std::string_view> const SpinometerModeStrings;
    extern std::vector<std::string_view> const RankedStatusLeaderboardStrings;

    namespace Text {
        inline std::string const StaticName = "Static";
        inline std::string const ScoreName = "Score";
        inline std::string const RankName = "Rank";
        inline std::string const PersonalBestName = "Personal Best";
        inline std::string const ComboName = "Combo";
        inline std::string const MultiplierName = "Multiplier";
        inline std::string const HealthName = "Health";
        inline std::string const TimeName = "Time";
        inline std::string const AverageCutName = "Average Cut";
        inline std::string const TimeDependenceName = "Time Dependence";
        inline std::string const FailsName = "Fails";
        inline std::string const MistakesName = "Mistakes";
        inline std::string const NotesName = "Notes";
        inline std::string const PPName = "PP";
        inline std::string const SaberSpeedName = "Saber Speed";
        inline std::string const SpinometerName = "Spinometer";
        inline std::string const FCPercentName = "FC Percentage";

        DECLARE_JSON_STRUCT(Static) {
            VALUE_DEFAULT(std::string, Input, "");
        };
        DECLARE_JSON_STRUCT(Score) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(int, Decimals, 1);
            VALUE_DEFAULT(bool, Percentage, true);
        };
        DECLARE_JSON_STRUCT(Rank) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(bool, PositiveModifiers, false);
            VALUE_DEFAULT(bool, NegativeModifiers, true);
        };
        DECLARE_JSON_STRUCT(PersonalBest) {
            VALUE_DEFAULT(int, Decimals, 1);
            VALUE_DEFAULT(bool, Percentage, true);
            VALUE_DEFAULT(bool, HideFirstScore, true);
            VALUE_DEFAULT(bool, Label, true);
        };
        DECLARE_JSON_STRUCT(Combo) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
        };
        DECLARE_JSON_STRUCT(Multiplier){};
        DECLARE_JSON_STRUCT(Health) {
            VALUE_DEFAULT(int, Decimals, 1);
            VALUE_DEFAULT(bool, Percentage, true);
        };
        DECLARE_JSON_STRUCT(Time) {
            VALUE_DEFAULT(bool, Remaining, false);
            VALUE_DEFAULT(bool, Percentage, false);
        };
        DECLARE_JSON_STRUCT(AverageCut) {
            enum class Parts {
                Pre,
                Post,
                Acc,
                All,
            };
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(int, Decimals, 1);
            VALUE_DEFAULT(int, Part, (int) Parts::All);
        };
        DECLARE_JSON_STRUCT(TimeDependence) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(int, Decimals, 1);
            VALUE_DEFAULT(int, DecimalOffset, 0);
        };
        DECLARE_JSON_STRUCT(Fails) {
            VALUE_DEFAULT(bool, Restarts, false);
        };
        DECLARE_JSON_STRUCT(Mistakes) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(bool, Misses, true);
            VALUE_DEFAULT(bool, BadCuts, true);
            VALUE_DEFAULT(bool, Bombs, false);
            VALUE_DEFAULT(bool, Walls, false);
        };
        DECLARE_JSON_STRUCT(Notes) {
            enum class Displays {
                Cut,
                Remaining,
                Ratio,
                Percent,
            };
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(int, Display, (int) Displays::Remaining);
            VALUE_DEFAULT(int, Decimals, 1);
        };
        DECLARE_JSON_STRUCT(PP) {
            enum class Leaderboards {
                ScoreSaber,
                BeatLeader,
            };
            VALUE_DEFAULT(int, Leaderboard, (int) Leaderboards::ScoreSaber);
            VALUE_DEFAULT(int, Decimals, 1);
        };
        DECLARE_JSON_STRUCT(SaberSpeed) {
            enum class Modes {
                Average,
                Best5Seconds,
            };
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(int, Decimals, 1);
            VALUE_DEFAULT(int, Mode, (int) Modes::Average);
        };
        DECLARE_JSON_STRUCT(Spinometer) {
            enum class Modes {
                Average,
                Highest,
            };
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(int, Mode, (int) Modes::Average);
        };
        DECLARE_JSON_STRUCT(FCPercent) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(int, Decimals, 1);
        };

        std::string GetStatic(UnparsedJSON options);
        std::string GetScore(UnparsedJSON options);
        std::string GetRank(UnparsedJSON options);
        std::string GetPersonalBest(UnparsedJSON options);
        std::string GetCombo(UnparsedJSON options);
        std::string GetMultiplier(UnparsedJSON options);
        std::string GetHealth(UnparsedJSON options);
        std::string GetTime(UnparsedJSON options);
        std::string GetAverageCut(UnparsedJSON options);
        std::string GetTimeDependence(UnparsedJSON options);
        std::string GetFails(UnparsedJSON options);
        std::string GetMistakes(UnparsedJSON options);
        std::string GetNotes(UnparsedJSON options);
        std::string GetPP(UnparsedJSON options);
        std::string GetSaberSpeed(UnparsedJSON options);
        std::string GetSpinometer(UnparsedJSON options);
        std::string GetFCPercent(UnparsedJSON options);
    }

    namespace Shape {
        inline std::string const StaticName = "Static";
        inline std::string const ScoreName = "Score";
        inline std::string const MultiplierName = "Multiplier";
        inline std::string const HealthName = "Health";
        inline std::string const TimeName = "Time";
        inline std::string const AverageCutName = "Average Cut";
        inline std::string const NotesName = "Notes";

        DECLARE_JSON_STRUCT(Static) {
            VALUE_DEFAULT(float, Input, 1);
        };
        DECLARE_JSON_STRUCT(Score) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
        };
        DECLARE_JSON_STRUCT(Multiplier) {
            VALUE_DEFAULT(bool, Absolute, false);
        };
        DECLARE_JSON_STRUCT(Health){};
        DECLARE_JSON_STRUCT(Time){};
        DECLARE_JSON_STRUCT(AverageCut) {
            enum class Parts {
                Pre,
                Post,
                Acc,
                All,
            };
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(int, Part, (int) Parts::All);
        };
        DECLARE_JSON_STRUCT(Notes) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
        };

        float GetStatic(UnparsedJSON options);
        float GetScore(UnparsedJSON options);
        float GetMultiplier(UnparsedJSON options);
        float GetHealth(UnparsedJSON options);
        float GetTime(UnparsedJSON options);
        float GetAverageCut(UnparsedJSON options);
        float GetNotes(UnparsedJSON options);
    }

    namespace Color {
        inline std::string const StaticName = "Static";
        inline std::string const PlayerName = "Color Scheme";
        inline std::string const RankName = "Rank";
        inline std::string const PersonalBestName = "Personal Best";
        inline std::string const ComboName = "Combo";
        inline std::string const MultiplierName = "Multiplier";
        inline std::string const HealthName = "Health";

        DECLARE_JSON_STRUCT(Static) {
            VALUE_DEFAULT(ConfigUtils::Color, Input, ConfigUtils::Color(1, 1, 1, 1));
        };
        DECLARE_JSON_STRUCT(Player) {
            enum class ColorSettings {
                LeftSaber,
                RightSaber,
                Lights1,
                Lights2,
                Walls,
                Boost1,
                Boost2,
            };
            VALUE_DEFAULT(int, Setting, (int) ColorSettings::LeftSaber);
        };
        DECLARE_JSON_STRUCT(Rank) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(bool, PositiveModifiers, false);
            VALUE_DEFAULT(bool, NegativeModifiers, true);
            VALUE_DEFAULT(ConfigUtils::Color, SS, ConfigUtils::Color(0, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, S, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, A, ConfigUtils::Color(0, 1, 0, 1));
            VALUE_DEFAULT(ConfigUtils::Color, B, ConfigUtils::Color(1, 0.92, 0.016, 1));
            VALUE_DEFAULT(ConfigUtils::Color, C, ConfigUtils::Color(1, 0.5, 0, 1));
            VALUE_DEFAULT(ConfigUtils::Color, D, ConfigUtils::Color(1, 0, 0, 1));
            VALUE_DEFAULT(ConfigUtils::Color, E, ConfigUtils::Color(1, 0, 0, 1));
            // parse after SS
            VALUE_DEFAULT(ConfigUtils::Color, SSS, self->SS) = ConfigUtils::Color(0, 1, 1, 1);
        };
        DECLARE_JSON_STRUCT(PersonalBest) {
            VALUE_DEFAULT(ConfigUtils::Color, Better, ConfigUtils::Color(0, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, Worse, ConfigUtils::Color(1, 0.647, 0, 1));
        };
        DECLARE_JSON_STRUCT(Combo) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(ConfigUtils::Color, Full, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, NonFull, ConfigUtils::Color(1, 1, 1, 1));
        };
        DECLARE_JSON_STRUCT(Multiplier) {
            VALUE_DEFAULT(ConfigUtils::Color, One, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, Two, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, Four, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, Eight, ConfigUtils::Color(1, 1, 1, 1));
        };
        DECLARE_JSON_STRUCT(Health) {
            VALUE_DEFAULT(ConfigUtils::Color, Full, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, AboveHalf, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, BelowHalf, ConfigUtils::Color(1, 1, 1, 1));
        };

        UnityEngine::Color GetStatic(UnparsedJSON options);
        UnityEngine::Color GetPlayer(UnparsedJSON options);
        UnityEngine::Color GetRank(UnparsedJSON options);
        UnityEngine::Color GetPersonalBest(UnparsedJSON options);
        UnityEngine::Color GetCombo(UnparsedJSON options);
        UnityEngine::Color GetMultiplier(UnparsedJSON options);
        UnityEngine::Color GetHealth(UnparsedJSON options);
    }

    namespace Enable {
        inline std::string const StaticName = "Always";
        inline std::string const RankedName = "Ranked";
        inline std::string const FullComboName = "Full Combo";
        inline std::string const PercentageName = "Percentage Above";
        inline std::string const FailedName = "Failed";

        DECLARE_JSON_STRUCT(Static){};
        DECLARE_JSON_STRUCT(Ranked) {
            enum class Leaderboards {
                ScoreSaber,
                BeatLeader,
                Either,
                Both,
            };
            VALUE_DEFAULT(int, Leaderboard, (int) Leaderboards::Either);
        };
        DECLARE_JSON_STRUCT(FullCombo) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
        };
        DECLARE_JSON_STRUCT(Percentage) {
            VALUE_DEFAULT(int, Saber, (int) Types::Sabers::Both);
            VALUE_DEFAULT(float, Percent, 90);
        };
        DECLARE_JSON_STRUCT(Failed){};

        bool GetStatic(UnparsedJSON options);
        bool GetRanked(UnparsedJSON options);
        bool GetFullCombo(UnparsedJSON options);
        bool GetPercentage(UnparsedJSON options);
        bool GetFailed(UnparsedJSON options);
    }
}
