#pragma once

#include "main.hpp"

#include "UnityEngine/GameObject.hpp"

#include "config-utils/shared/config-utils.hpp"
#include "types.hpp"

namespace Qounters {
    extern std::vector<std::pair<std::string, std::pair<SourceFn<std::string>, SourceUIFn>>> textSources;
    extern std::vector<std::pair<std::string, std::pair<SourceFn<float>, SourceUIFn>>> shapeSources;
    extern std::vector<std::pair<std::string, std::pair<SourceFn<UnityEngine::Color>, SourceUIFn>>> colorSources;

    template<class T>
    T GetSource(std::vector<std::pair<std::string, T>> sourceVec, std::string source) {
        for (auto& [str, ret] : sourceVec) {
            if (str == source)
                return ret;
        }
        return {};
    }

    template<class T>
    void RegisterSource(std::vector<std::pair<std::string, std::pair<T, SourceUIFn>>> sourceVec, std::string source, T sourceFn, SourceUIFn sourceUIFn) {
        sourceVec.emplace_back(source, std::make_pair(sourceFn, sourceUIFn));
    }

    inline void RegisterTextSource(std::string sourceName, SourceFn<std::string> sourceFn, SourceUIFn sourceUIFn) {
        RegisterSource(textSources, sourceName, sourceFn, sourceUIFn);
    }
    inline void RegisterShapeSource(std::string sourceName, SourceFn<float> sourceFn, SourceUIFn sourceUIFn) {
        RegisterSource(shapeSources, sourceName, sourceFn, sourceUIFn);
    }
    inline void RegisterColorSource(std::string sourceName, SourceFn<UnityEngine::Color> sourceFn, SourceUIFn sourceUIFn) {
        RegisterSource(colorSources, sourceName, sourceFn, sourceUIFn);
    }

    extern const std::vector<std::string> AverageCutPartStrings;
    extern const std::vector<std::string> NotesDisplayStrings;
    extern const std::vector<std::string> PPSourceStrings;
    extern const std::vector<std::string> SaberSpeedModeStrings;
    extern const std::vector<std::string> SpinometerModeStrings;

    namespace TextSource {
        inline const std::string StaticName = "Static";
        inline const std::string ScoreName = "Score";
        inline const std::string RankName = "Rank";
        inline const std::string PersonalBestName = "Personal Best";
        inline const std::string ComboName = "Combo";
        inline const std::string MultiplierName = "Multiplier";
        inline const std::string HealthName = "Health";
        inline const std::string TimeName = "Time";
        inline const std::string AverageCutName = "Average Cut";
        inline const std::string TimeDependenceName = "Time Dependence";
        inline const std::string FailsName = "Fails";
        inline const std::string MistakesName = "Mistakes";
        inline const std::string NotesName = "Notes";
        inline const std::string PPName = "PP";
        inline const std::string SaberSpeedName = "Saber Speed";
        inline const std::string SpinometerName = "Spinometer";

        DECLARE_JSON_CLASS(Static,
            VALUE_DEFAULT(std::string, Input, "")
        )
        DECLARE_JSON_CLASS(Score,
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(int, Decimals, 1)
            VALUE_DEFAULT(bool, Percentage, true)
        )
        DECLARE_JSON_CLASS(Rank,
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(bool, PositiveModifiers, false)
            VALUE_DEFAULT(bool, NegativeModifiers, true)
        )
        DECLARE_JSON_CLASS(PersonalBest,
            VALUE_DEFAULT(int, Decimals, 1)
            VALUE_DEFAULT(bool, Percentage, true)
            VALUE_DEFAULT(bool, HideFirstScore, true)
            VALUE_DEFAULT(bool, Label, true)
        )
        DECLARE_JSON_CLASS(Combo,
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
        )
        // DECLARE_JSON_CLASS(Multiplier,
        // )
        DECLARE_JSON_CLASS(Health,
            VALUE_DEFAULT(int, Decimals, 1)
            VALUE_DEFAULT(bool, Percentage, true)
        )
        DECLARE_JSON_CLASS(Time,
            VALUE_DEFAULT(bool, Remaining, false)
            VALUE_DEFAULT(bool, Percentage, false)
        )
        DECLARE_JSON_CLASS(AverageCut,
            enum class Parts {
                Pre,
                Post,
                Acc,
                All,
            };
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(int, Decimals, 1)
            VALUE_DEFAULT(int, Part, (int) Parts::All)
        )
        DECLARE_JSON_CLASS(TimeDependence,
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(int, Decimals, 1)
            VALUE_DEFAULT(int, DecimalOffset, 0)
        )
        DECLARE_JSON_CLASS(Fails,
            VALUE_DEFAULT(bool, Restarts, false)
        )
        DECLARE_JSON_CLASS(Mistakes,
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(bool, Misses, true)
            VALUE_DEFAULT(bool, BadCuts, true)
            VALUE_DEFAULT(bool, Bombs, false)
            VALUE_DEFAULT(bool, Walls, false)
        )
        DECLARE_JSON_CLASS(Notes,
            enum class Displays {
                Cut,
                Remaining,
                Ratio,
                Percent,
            };
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(int, Display, (int) Displays::Remaining)
            VALUE_DEFAULT(int, Decimals, 1)
        )
        DECLARE_JSON_CLASS(PP,
            enum class Sources {
                ScoreSaber,
                BeatLeader,
            };
            VALUE_DEFAULT(int, Source, (int) Sources::ScoreSaber)
            VALUE_DEFAULT(int, Decimals, 1)
            VALUE_DEFAULT(bool, HideUnranked, true)
        )
        DECLARE_JSON_CLASS(SaberSpeed,
            enum class Modes {
                Average,
                Best5Seconds,
            };
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(int, Decimals, 1)
            VALUE_DEFAULT(int, Mode, (int) Modes::Average)
        )
        DECLARE_JSON_CLASS(Spinometer,
            enum class Modes {
                Average,
                Highest,
            };
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(int, Mode, (int) Modes::Average)
            // what the hell does this even measure though
        )

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
    }

    namespace ShapeSource {
        inline const std::string StaticName = "Static";
        inline const std::string ScoreName = "Score";
        inline const std::string MultiplierName = "Multiplier";
        inline const std::string HealthName = "Health";
        inline const std::string TimeName = "Time";
        inline const std::string AverageCutName = "Average Cut";
        inline const std::string NotesName = "Notes";

        DECLARE_JSON_CLASS(Static,
            VALUE_DEFAULT(float, Input, 1)
        )
        DECLARE_JSON_CLASS(Score,
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
        )
        DECLARE_JSON_CLASS(Multiplier,
            VALUE_DEFAULT(bool, Absolute, false);
        )
        // DECLARE_JSON_CLASS(Health,
        // )
        // DECLARE_JSON_CLASS(Time,
        // )
        DECLARE_JSON_CLASS(AverageCut,
            enum class Parts {
                Pre,
                Post,
                Acc,
                All,
            };
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(int, Part, (int) Parts::All)
        )
        DECLARE_JSON_CLASS(Notes,
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
        )

        float GetStatic(UnparsedJSON options);
        float GetScore(UnparsedJSON options);
        float GetMultiplier(UnparsedJSON options);
        float GetHealth(UnparsedJSON options);
        float GetTime(UnparsedJSON options);
        float GetAverageCut(UnparsedJSON options);
        float GetNotes(UnparsedJSON options);
    }

    namespace ColorSource {
        inline const std::string StaticName = "Static";
        inline const std::string PlayerName = "Player";
        inline const std::string RankName = "Rank";
        inline const std::string PersonalBestName = "Personal Best";
        inline const std::string ComboName = "Combo";
        inline const std::string MultiplierName = "Multiplier";
        inline const std::string HealthName = "Health";

        DECLARE_JSON_CLASS(Static,
            VALUE_DEFAULT(ConfigUtils::Color, Input, ConfigUtils::Color(1, 1, 1, 1))
        )
        DECLARE_JSON_CLASS(Player,
            enum class ColorSettings {
                LeftSaber,
                RightSaber,
                Lights1,
                Lights2,
                Walls,
                Boost1,
                Boost2,
            };
            VALUE_DEFAULT(int, Setting, (int) ColorSettings::LeftSaber)
        )
        DECLARE_JSON_CLASS(Rank,
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(bool, PositiveModifiers, false)
            VALUE_DEFAULT(bool, NegativeModifiers, true)
            VALUE_DEFAULT(ConfigUtils::Color, SS, ConfigUtils::Color(0, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, S, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, A, ConfigUtils::Color(0, 1, 0, 1));
            VALUE_DEFAULT(ConfigUtils::Color, B, ConfigUtils::Color(1, 0.92, 0.016, 1));
            VALUE_DEFAULT(ConfigUtils::Color, C, ConfigUtils::Color(1, 0.5, 0, 1));
            VALUE_DEFAULT(ConfigUtils::Color, D, ConfigUtils::Color(1, 0, 0, 1));
            VALUE_DEFAULT(ConfigUtils::Color, E, ConfigUtils::Color(1, 0, 0, 1));
        )
        DECLARE_JSON_CLASS(PersonalBest,
            VALUE_DEFAULT(ConfigUtils::Color, Better, ConfigUtils::Color(0, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, Worse, ConfigUtils::Color(1, 0.647, 0, 1));
        )
        DECLARE_JSON_CLASS(Combo,
            VALUE_DEFAULT(int, Saber, (int) Sabers::Both)
            VALUE_DEFAULT(ConfigUtils::Color, Full, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, NonFull, ConfigUtils::Color(1, 1, 1, 1));
        )
        DECLARE_JSON_CLASS(Multiplier,
            VALUE_DEFAULT(ConfigUtils::Color, One, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, Two, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, Four, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, Eight, ConfigUtils::Color(1, 1, 1, 1));
        )
        DECLARE_JSON_CLASS(Health,
            VALUE_DEFAULT(ConfigUtils::Color, Full, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, AboveHalf, ConfigUtils::Color(1, 1, 1, 1));
            VALUE_DEFAULT(ConfigUtils::Color, BelowHalf, ConfigUtils::Color(1, 1, 1, 1));
        )

        UnityEngine::Color GetStatic(UnparsedJSON options);
        UnityEngine::Color GetPlayer(UnparsedJSON options);
        UnityEngine::Color GetRank(UnparsedJSON options);
        UnityEngine::Color GetPersonalBest(UnparsedJSON options);
        UnityEngine::Color GetCombo(UnparsedJSON options);
        UnityEngine::Color GetMultiplier(UnparsedJSON options);
        UnityEngine::Color GetHealth(UnparsedJSON options);
    }
}
