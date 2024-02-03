#include "sourceui.hpp"
#include "editor.hpp"
#include "sources.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "utils.hpp"

using namespace UnityEngine;
using namespace QuestUI;

namespace Qounters::TextSource {
    void StaticUI(GameObject* parent, UnparsedJSON unparsed) {
        static Static opts;
        opts = unparsed.Parse<Static>();

        auto input = BeatSaberUI::CreateStringSetting(parent, "Text", opts.Input, [](StringW val) {
            static int id = Editor::GetActionId();
            opts.Input = (std::string) val;
            Editor::SetSourceOptions(id, opts);
        });
        Utils::AddStringSettingOnClose(input, [](std::string _) {
            Editor::FinalizeAction();
        });
    }
    void ScoreUI(GameObject* parent, UnparsedJSON unparsed) {
        static Score opts;
        opts = unparsed.Parse<Score>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        auto inc = BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
            static int id = Editor::GetActionId();
            opts.Decimals = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
        SetButtons(inc);

        BeatSaberUI::CreateToggle(parent, "Percentage", opts.Percentage, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Percentage = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void RankUI(GameObject* parent, UnparsedJSON unparsed) {
        static Rank opts;
        opts = unparsed.Parse<Rank>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Positive Modifiers", opts.PositiveModifiers, [](bool val) {
            static int id = Editor::GetActionId();
            opts.PositiveModifiers = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Negative Modifiers", opts.NegativeModifiers, [](bool val) {
            static int id = Editor::GetActionId();
            opts.NegativeModifiers = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void PersonalBestUI(GameObject* parent, UnparsedJSON unparsed) {
        static PersonalBest opts;
        opts = unparsed.Parse<PersonalBest>();

        auto inc = BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
            static int id = Editor::GetActionId();
            opts.Decimals = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
        SetButtons(inc);

        BeatSaberUI::CreateToggle(parent, "Percentage", opts.Percentage, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Percentage = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Hide On First Score", opts.HideFirstScore, [](bool val) {
            static int id = Editor::GetActionId();
            opts.HideFirstScore = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Label Text", opts.Label, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Label = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void ComboUI(GameObject* parent, UnparsedJSON unparsed) {
        static Combo opts;
        opts = unparsed.Parse<Combo>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void MultiplierUI(GameObject* parent, UnparsedJSON unparsed) {
        // static Multiplier opts;
        // opts = unparsed.Parse<Multiplier>();
    }
    void HealthUI(GameObject* parent, UnparsedJSON unparsed) {
        static Health opts;
        opts = unparsed.Parse<Health>();

        auto inc = BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
            static int id = Editor::GetActionId();
            opts.Decimals = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
        SetButtons(inc);

        BeatSaberUI::CreateToggle(parent, "Percentage", opts.Percentage, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Percentage = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void TimeUI(GameObject* parent, UnparsedJSON unparsed) {
        static Time opts;
        opts = unparsed.Parse<Time>();

        BeatSaberUI::CreateToggle(parent, "Remaining", opts.Remaining, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Remaining = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Percentage", opts.Percentage, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Percentage = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void AverageCutUI(GameObject* parent, UnparsedJSON unparsed) {
        static AverageCut opts;
        opts = unparsed.Parse<AverageCut>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        auto inc = BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
            static int id = Editor::GetActionId();
            opts.Decimals = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
        SetButtons(inc);

        Utils::CreateDropdownEnum(parent, "Part", opts.Part, AverageCutPartStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Part = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void TimeDependenceUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
        static TimeDependence opts;
        opts = unparsed.Parse<TimeDependence>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        auto inc = BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
            static int id = Editor::GetActionId();
            opts.Decimals = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
        SetButtons(inc);

        auto inc2 = BeatSaberUI::CreateIncrementSetting(parent, "Decimal Offset", 0, 1, opts.DecimalOffset, 0, 10, [](float val) {
            static int id = Editor::GetActionId();
            opts.DecimalOffset = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
        SetButtons(inc2);
    }
    void FailsUI(GameObject* parent, UnparsedJSON unparsed) {
        static Fails opts;
        opts = unparsed.Parse<Fails>();

        BeatSaberUI::CreateToggle(parent, "Restarts", opts.Restarts, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Restarts = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void MistakesUI(GameObject* parent, UnparsedJSON unparsed) {
        static Mistakes opts;
        opts = unparsed.Parse<Mistakes>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Misses", opts.Misses, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Misses = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Bad Cuts", opts.BadCuts, [](bool val) {
            static int id = Editor::GetActionId();
            opts.BadCuts = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Bombs", opts.Bombs, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Bombs = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Walls", opts.Walls, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Walls = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void NotesUI(GameObject* parent, UnparsedJSON unparsed) {
        static Notes opts;
        opts = unparsed.Parse<Notes>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        Utils::CreateDropdownEnum(parent, "Display", opts.Display, NotesDisplayStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Display = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void PPUI(GameObject* parent, UnparsedJSON unparsed) {
        static PP opts;
        opts = unparsed.Parse<PP>();

        Utils::CreateDropdownEnum(parent, "Source", opts.Leaderboard, PPLeaderboardStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Leaderboard = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        auto inc = BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
            static int id = Editor::GetActionId();
            opts.Decimals = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
        SetButtons(inc);
    }
    void SaberSpeedUI(GameObject* parent, UnparsedJSON unparsed) {
        static SaberSpeed opts;
        opts = unparsed.Parse<SaberSpeed>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        auto inc = BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
            static int id = Editor::GetActionId();
            opts.Decimals = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
        SetButtons(inc);

        Utils::CreateDropdownEnum(parent, "Mode", opts.Mode, SaberSpeedModeStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Mode = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void SpinometerUI(GameObject* parent, UnparsedJSON unparsed) {
        static Spinometer opts;
        opts = unparsed.Parse<Spinometer>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        Utils::CreateDropdownEnum(parent, "Mode", opts.Mode, SpinometerModeStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Mode = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateText(parent, "nyi - idk what this counts");
    }
    void FCPercentUI(GameObject* parent, UnparsedJSON unparsed) {
        static FCPercent opts;
        opts = unparsed.Parse<FCPercent>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        auto inc = BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
            static int id = Editor::GetActionId();
            opts.Decimals = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
        SetButtons(inc);
    }

    void CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options) {
        auto trans = parent->get_transform();
        while (trans->GetChildCount() > 0)
            UnityEngine::Object::DestroyImmediate(trans->GetChild(0)->get_gameObject());

        auto fn = GetSource(textSources, source).second;
        fn(parent, options);
    }
}

namespace Qounters::ShapeSource {
    void StaticUI(GameObject* parent, UnparsedJSON unparsed) {
        static Static opts;
        opts = unparsed.Parse<Static>();

        auto slider = BeatSaberUI::CreateSliderSetting(parent, "Fill", 0.01, opts.Input, 0, 1, 0, [](float val) {
            static int id = Editor::GetActionId();
            opts.Input = val;
            Editor::SetSourceOptions(id, opts);
        });
        Utils::AddSliderEndDrag(slider, [](float _) {
            Editor::FinalizeAction();
        });
        slider->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});
        AddSliderIncrement(slider, 0.01);
    }
    void ScoreUI(GameObject* parent, UnparsedJSON unparsed) {
        static Score opts;
        opts = unparsed.Parse<Score>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void MultiplierUI(GameObject* parent, UnparsedJSON unparsed) {
        static Multiplier opts;
        opts = unparsed.Parse<Multiplier>();

        BeatSaberUI::CreateToggle(parent, "Absolute", opts.Absolute, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Absolute = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void HealthUI(GameObject* parent, UnparsedJSON unparsed) {
        // static Health opts;
        // opts = unparsed.Parse<Health>();
    }
    void TimeUI(GameObject* parent, UnparsedJSON unparsed) {
        // static Time opts;
        // opts = unparsed.Parse<Time>();
    }
    void AverageCutUI(GameObject* parent, UnparsedJSON unparsed) {
        static AverageCut opts;
        opts = unparsed.Parse<AverageCut>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });

        Utils::CreateDropdownEnum(parent, "Part", opts.Part, AverageCutPartStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Part = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void NotesUI(GameObject* parent, UnparsedJSON unparsed) {
        static Notes opts;
        opts = unparsed.Parse<Notes>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetSourceOptions(id, opts);
            Editor::FinalizeAction();
        });
    }

    void CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options) {
        auto trans = parent->get_transform();
        while (trans->GetChildCount() > 0)
            UnityEngine::Object::DestroyImmediate(trans->GetChild(0)->get_gameObject());

        auto fn = GetSource(shapeSources, source).second;
        fn(parent, options);
    }
}

namespace Qounters::ColorSource {
    const std::vector<std::string> ColorSettingsStrings = {
        "Left Saber",
        "Right Saber",
        "Lights 1",
        "Lights 2",
        "Walls",
        "Boost Lights 1",
        "Boost Lights 2",
    };

    void StaticUI(GameObject* parent, UnparsedJSON unparsed) {
        static Static opts;
        opts = unparsed.Parse<Static>();

        Utils::CreateColorPicker(parent, "Color", opts.Input, [](Color val) {
            static int id = Editor::GetActionId();
            opts.Input = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });
    }
    void PlayerUI(GameObject* parent, UnparsedJSON unparsed) {
        static Player opts;
        opts = unparsed.Parse<Player>();

        Utils::CreateDropdownEnum(parent, "Color Setting", opts.Setting, ColorSettingsStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Setting = val;
            Editor::SetColorOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void RankUI(GameObject* parent, UnparsedJSON unparsed) {
        static Rank opts;
        opts = unparsed.Parse<Rank>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetColorOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Positive Modifiers", opts.PositiveModifiers, [](bool val) {
            static int id = Editor::GetActionId();
            opts.PositiveModifiers = val;
            Editor::SetColorOptions(id, opts);
            Editor::FinalizeAction();
        });

        BeatSaberUI::CreateToggle(parent, "Negative Modifiers", opts.NegativeModifiers, [](bool val) {
            static int id = Editor::GetActionId();
            opts.NegativeModifiers = val;
            Editor::SetColorOptions(id, opts);
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "SS Color", opts.SS, [](Color val) {
            static int id = Editor::GetActionId();
            opts.SS = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "S Color", opts.S, [](Color val) {
            static int id = Editor::GetActionId();
            opts.S = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "A Color", opts.A, [](Color val) {
            static int id = Editor::GetActionId();
            opts.A = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "B Color", opts.B, [](Color val) {
            static int id = Editor::GetActionId();
            opts.B = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "C Color", opts.C, [](Color val) {
            static int id = Editor::GetActionId();
            opts.C = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "D Color", opts.D, [](Color val) {
            static int id = Editor::GetActionId();
            opts.D = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "E Color", opts.E, [](Color val) {
            static int id = Editor::GetActionId();
            opts.E = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });
    }
    void PersonalBestUI(GameObject* parent, UnparsedJSON unparsed) {
        static PersonalBest opts;
        opts = unparsed.Parse<PersonalBest>();

        Utils::CreateColorPicker(parent, "Better Color", opts.Better, [](Color val) {
            static int id = Editor::GetActionId();
            opts.Better = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "Worse Color", opts.Worse, [](Color val) {
            static int id = Editor::GetActionId();
            opts.Worse = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });
    }
    void ComboUI(GameObject* parent, UnparsedJSON unparsed) {
        static Combo opts;
        opts = unparsed.Parse<Combo>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetColorOptions(id, opts);
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "FC Color", opts.Full, [](Color val) {
            static int id = Editor::GetActionId();
            opts.Full = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "Non FC Color", opts.NonFull, [](Color val) {
            static int id = Editor::GetActionId();
            opts.NonFull = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });
    }
    void MultiplierUI(GameObject* parent, UnparsedJSON unparsed) {
        static Multiplier opts;
        opts = unparsed.Parse<Multiplier>();

        Utils::CreateColorPicker(parent, "One", opts.One, [](Color val) {
            static int id = Editor::GetActionId();
            opts.One = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "Two", opts.Two, [](Color val) {
            static int id = Editor::GetActionId();
            opts.Two = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "Four", opts.Four, [](Color val) {
            static int id = Editor::GetActionId();
            opts.Four = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "Eight", opts.Eight, [](Color val) {
            static int id = Editor::GetActionId();
            opts.Eight = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });
    }
    void HealthUI(GameObject* parent, UnparsedJSON unparsed) {
        static Health opts;
        opts = unparsed.Parse<Health>();

        Utils::CreateColorPicker(parent, "Full", opts.Full, [](Color val) {
            static int id = Editor::GetActionId();
            opts.Full = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "Above Half", opts.AboveHalf, [](Color val) {
            static int id = Editor::GetActionId();
            opts.AboveHalf = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });

        Utils::CreateColorPicker(parent, "Below Half", opts.BelowHalf, [](Color val) {
            static int id = Editor::GetActionId();
            opts.BelowHalf = val;
            Editor::SetColorOptions(id, opts);
        }, []() {
            Editor::FinalizeAction();
        });
    }

    void CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options) {
        auto trans = parent->get_transform();
        while (trans->GetChildCount() > 0)
            UnityEngine::Object::DestroyImmediate(trans->GetChild(0)->get_gameObject());

        auto fn = GetSource(colorSources, source).second;
        fn(parent, options);
    }
}

namespace Qounters::EnableSource {
    void StaticUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
        // static Static opts;
        // opts = unparsed.Parse<Static>();
    }
    void RankedUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
        static Ranked opts;
        opts = unparsed.Parse<Ranked>();

        Utils::CreateDropdownEnum(parent, "Leaderboard", opts.Leaderboard, RankedStatusLeaderboardStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Leaderboard = val;
            Editor::SetEnableOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void FullComboUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
        static FullCombo opts;
        opts = unparsed.Parse<FullCombo>();

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetEnableOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void PercentageUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
        static Percentage opts;
        opts = unparsed.Parse<Percentage>();

        auto slider = BeatSaberUI::CreateSliderSetting(parent, "Percentage", 1, opts.Percent, 0, 100, 0, [](float val) {
            static int id = Editor::GetActionId();
            opts.Percent = val;
            Editor::SetSourceOptions(id, opts);
        });
        Utils::AddSliderEndDrag(slider, [](float _) {
            Editor::FinalizeAction();
        });
        slider->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});
        AddSliderIncrement(slider, 1);

        Utils::CreateDropdownEnum(parent, "Saber", opts.Saber, SaberStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Saber = val;
            Editor::SetEnableOptions(id, opts);
            Editor::FinalizeAction();
        });
    }
    void FailedUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
        // static Failed opts;
        // opts = unparsed.Parse<Failed>();
    }

    void CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options) {
        auto trans = parent->get_transform();
        while (trans->GetChildCount() > 0)
            UnityEngine::Object::DestroyImmediate(trans->GetChild(0)->get_gameObject());

        auto fn = GetSource(enableSources, source).second;
        fn(parent, options);
    }
}
