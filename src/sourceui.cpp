#include "sourceui.hpp"

#include "UnityEngine/Events/UnityAction_1.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "editor.hpp"
#include "metacore/shared/ui.hpp"
#include "sources.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace UnityEngine;
namespace MUI = MetaCore::UI;

void Sources::Text::StaticUI(GameObject* parent, UnparsedJSON unparsed) {
    static Static opts;
    opts = unparsed.Parse<Static>();

    auto input = BSML::Lite::CreateStringSetting(parent, "Text", opts.Input, [](StringW val) {
        static int id = Editor::GetActionId();
        opts.Input = (std::string) val;
        Editor::SetSourceOptions(id, opts);
    });
    MUI::AddStringSettingOnClose(input, []() { Editor::FinalizeAction(); });
    BSML::Lite::AddHoverHint(input, "The static text to show");
}
void Sources::Text::ScoreUI(GameObject* parent, UnparsedJSON unparsed) {
    static Score opts;
    opts = unparsed.Parse<Score>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to show the score for");

    auto percentage = BSML::Lite::CreateToggle(parent, "Percentage", opts.Percentage, [parent](bool val) {
        static int id = Editor::GetActionId();
        opts.Percentage = val;
        // logic to enable/disable decimals
        auto decimalGO = parent->Find("decimals");
        auto decimals = decimalGO->GetComponent<BSML::IncrementSetting*>();
        decimals->set_interactable(val);
        auto decimalCanvasGroup = decimals->get_gameObject()->GetComponent<UnityEngine::CanvasGroup*>();
        if (val) {
            decimalCanvasGroup->set_alpha(1);
        } else {
            decimalCanvasGroup->set_alpha(0.5f);  // dimmed
        }
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(percentage, "Show the score as a percentage instead of absolute value");

    auto inc = BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
        static int id = Editor::GetActionId();
        opts.Decimals = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(inc, "The number of decimals in the score, if a percentage");

    auto separator = MUI::CreateDropdownEnum(parent, "Separator", opts.Separator, Options::SeparatorStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Separator = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(separator, "The thousands separator style to use if absolute value");

    // logic to set intractability of decimals on initialization
    inc->set_name("decimals");
    inc->set_interactable(opts.Percentage);
    auto decimalCanvasGroup = inc->get_gameObject()->AddComponent<UnityEngine::CanvasGroup*>();
    if (opts.Percentage) {
        decimalCanvasGroup->set_alpha(1);
    } else {
        decimalCanvasGroup->set_alpha(0.5f);  // dimmed
    }
}
void Sources::Text::RankUI(GameObject* parent, UnparsedJSON unparsed) {
    static Rank opts;
    opts = unparsed.Parse<Rank>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to show the score for");

    auto posMods = BSML::Lite::CreateToggle(parent, "Positive Modifiers", opts.PositiveModifiers, [](bool val) {
        static int id = Editor::GetActionId();
        opts.PositiveModifiers = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(posMods, "Include positive modifiers in the rank calculation");

    auto negMods = BSML::Lite::CreateToggle(parent, "Negative Modifiers", opts.NegativeModifiers, [](bool val) {
        static int id = Editor::GetActionId();
        opts.NegativeModifiers = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(negMods, "Include negative modifiers in the rank calculation");
}
void Sources::Text::PersonalBestUI(GameObject* parent, UnparsedJSON unparsed) {
    static PersonalBest opts;
    opts = unparsed.Parse<PersonalBest>();

    auto percentage = BSML::Lite::CreateToggle(parent, "Percentage", opts.Percentage, [parent](bool val) {
        static int id = Editor::GetActionId();
        opts.Percentage = val;
        auto decimalGO = parent->Find("decimals");
        auto decimals = decimalGO->GetComponent<BSML::IncrementSetting*>();
        decimals->set_interactable(val);
        auto decimalCanvasGroup = decimals->get_gameObject()->GetComponent<UnityEngine::CanvasGroup*>();
        if (val) {
            decimalCanvasGroup->set_alpha(1);
        } else {
            decimalCanvasGroup->set_alpha(0.5f);  // dimmed
        }
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(percentage, "Display the difference from personal best as a percentage instead of absolute value");

    auto decimals = BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
        static int id = Editor::GetActionId();
        opts.Decimals = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(decimals, "The number of decimals to show, if a percentage");

    auto label = BSML::Lite::CreateToggle(parent, "Label Text", opts.Label, [](bool val) {
        static int id = Editor::GetActionId();
        opts.Label = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(label, "Labels the text with \"PB: \" or \"PB Gap: \"");

    auto separator = MUI::CreateDropdownEnum(parent, "Separator", opts.Separator, Options::SeparatorStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Separator = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(separator, "The thousands separator style to use if absolute value");

    auto display = MUI::CreateDropdownEnum(parent, "Display", opts.Display, PBDisplayStrings, [parent](int val) {
        static int id = Editor::GetActionId();
        opts.Display = val;
        // logic to enable/disable sign and showZero
        auto signGO = parent->get_transform()->Find("sign");
        auto sign = signGO->GetComponent<BSML::ToggleSetting*>();
        auto showZeroGO = parent->get_transform()->Find("showZero");
        auto showZero = showZeroGO->GetComponent<BSML::ToggleSetting*>();
        sign->set_interactable(val == (int) PersonalBest::Displays::PBGap);
        showZero->set_interactable(val == (int) PersonalBest::Displays::PersonalBest);
        auto signCanvasGroup = sign->get_gameObject()->GetComponent<UnityEngine::CanvasGroup*>();
        auto zeroCanvasGroup = showZero->get_gameObject()->GetComponent<UnityEngine::CanvasGroup*>();
        if (val) {
            signCanvasGroup->set_alpha(1);
            zeroCanvasGroup->set_alpha(0.5f);  // dimmed
        } else {
            signCanvasGroup->set_alpha(0.5f);  // dimmed
            zeroCanvasGroup->set_alpha(1);
        }
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(display, "The personal best related value to show");

    auto sign = BSML::Lite::CreateToggle(parent, "Sign", opts.Sign, [](bool val) {
        static int id = Editor::GetActionId();
        opts.Sign = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(sign, "Display a positive or negative sign next to the difference if PB Gap is enabled");

    auto showZero = BSML::Lite::CreateToggle(parent, "Show 0 On First Score", !opts.HideFirstScore, [](bool val) {
        static int id = Editor::GetActionId();
        opts.HideFirstScore = !val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(showZero, "Shows 0 if you have no personal best instead of \"--\" if PB Gap is disabled");

    // logic to set intractability of the settings on initialization
    // decimals
    decimals->set_name("decimals");
    decimals->set_interactable(opts.Percentage);
    auto decimalCanvasGroup = decimals->get_gameObject()->AddComponent<UnityEngine::CanvasGroup*>();
    if (opts.Percentage) {
        decimalCanvasGroup->set_alpha(1);
    } else {
        decimalCanvasGroup->set_alpha(0.5f);  // dimmed
    }
    // Sign
    sign->set_name("sign");
    sign->set_interactable(opts.Display == (int) PersonalBest::Displays::PBGap);
    auto signCanvasGroup = sign->get_gameObject()->AddComponent<UnityEngine::CanvasGroup*>();
    if (opts.Display == (int) PersonalBest::Displays::PBGap) {
        signCanvasGroup->set_alpha(1);
    } else {
        signCanvasGroup->set_alpha(0.5f);  // dimmed
    }
    // showZero
    showZero->set_name("showZero");
    showZero->set_interactable(opts.Display == (int) PersonalBest::Displays::PersonalBest);
    auto zeroCanvasGroup = showZero->get_gameObject()->AddComponent<UnityEngine::CanvasGroup*>();
    if (opts.Display == (int) PersonalBest::Displays::PersonalBest) {
        zeroCanvasGroup->set_alpha(1);
    } else {
        zeroCanvasGroup->set_alpha(0.5f);  // dimmed
    }
}
void Sources::Text::ComboUI(GameObject* parent, UnparsedJSON unparsed) {
    static Combo opts;
    opts = unparsed.Parse<Combo>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to show the combo for");
}
void Sources::Text::MultiplierUI(GameObject* parent, UnparsedJSON unparsed) {
    // static Multiplier opts;
    // opts = unparsed.Parse<Multiplier>();
}
void Sources::Text::HealthUI(GameObject* parent, UnparsedJSON unparsed) {
    static Health opts;
    opts = unparsed.Parse<Health>();

    auto decimals = BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
        static int id = Editor::GetActionId();
        opts.Decimals = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(decimals, "The number of decimals to show");

    auto percentage = BSML::Lite::CreateToggle(parent, "Percentage", opts.Percentage, [](bool val) {
        static int id = Editor::GetActionId();
        opts.Percentage = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(percentage, "Shows the health as a percentage instead of absolute value");
}
void Sources::Text::TimeUI(GameObject* parent, UnparsedJSON unparsed) {
    static Time opts;
    opts = unparsed.Parse<Time>();

    auto remaining = BSML::Lite::CreateToggle(parent, "Remaining", opts.Remaining, [](bool val) {
        static int id = Editor::GetActionId();
        opts.Remaining = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(remaining, "Shows the time remaining");

    auto percentage = BSML::Lite::CreateToggle(parent, "Percentage", opts.Percentage, [](bool val) {
        static int id = Editor::GetActionId();
        opts.Percentage = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(percentage, "Shows the song time as a percentage, instead of absolute value");
}
void Sources::Text::AverageCutUI(GameObject* parent, UnparsedJSON unparsed) {
    static AverageCut opts;
    opts = unparsed.Parse<AverageCut>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to show the average for");

    auto decimals = BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
        static int id = Editor::GetActionId();
        opts.Decimals = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(decimals, "The number of decimals to show");

    auto part = MUI::CreateDropdownEnum(parent, "Part", opts.Part, AverageCutPartStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Part = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(part, "Which part of the swing to show the average of");
}
void Sources::Text::TimeDependenceUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    static TimeDependence opts;
    opts = unparsed.Parse<TimeDependence>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to show the time dependence for");

    auto decimals = BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
        static int id = Editor::GetActionId();
        opts.Decimals = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(decimals, "The number of decimals (after the decimal point) to show");

    auto offset = BSML::Lite::CreateIncrementSetting(parent, "Decimal Offset", 0, 1, opts.DecimalOffset, 0, 10, [](float val) {
        static int id = Editor::GetActionId();
        opts.DecimalOffset = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(offset, "");
}
void Sources::Text::FailsUI(GameObject* parent, UnparsedJSON unparsed) {
    static Fails opts;
    opts = unparsed.Parse<Fails>();

    auto restarts = BSML::Lite::CreateToggle(parent, "Restarts", opts.Restarts, [](bool val) {
        static int id = Editor::GetActionId();
        opts.Restarts = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(restarts, "Count the number of consecutive restarts instead of total fails");
}
void Sources::Text::MistakesUI(GameObject* parent, UnparsedJSON unparsed) {
    static Mistakes opts;
    opts = unparsed.Parse<Mistakes>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to show the mistake count for");

    auto misses = BSML::Lite::CreateToggle(parent, "Misses", opts.Misses, [](bool val) {
        static int id = Editor::GetActionId();
        opts.Misses = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(misses, "Include misses in the mistake count");

    auto badCuts = BSML::Lite::CreateToggle(parent, "Bad Cuts", opts.BadCuts, [](bool val) {
        static int id = Editor::GetActionId();
        opts.BadCuts = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(badCuts, "Include bad cuts in the mistake count");

    auto bombs = BSML::Lite::CreateToggle(parent, "Bombs", opts.Bombs, [](bool val) {
        static int id = Editor::GetActionId();
        opts.Bombs = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(bombs, "Include bombs in the mistake count");

    auto walls = BSML::Lite::CreateToggle(parent, "Walls", opts.Walls, [](bool val) {
        static int id = Editor::GetActionId();
        opts.Walls = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(walls, "Include walls in the mistake count");
}
void Sources::Text::NotesUI(GameObject* parent, UnparsedJSON unparsed) {
    static Notes opts;
    opts = unparsed.Parse<Notes>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to show the notes value for");

    auto display = MUI::CreateDropdownEnum(parent, "Display", opts.Display, NotesDisplayStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Display = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(display, "The note-related value to show");

    auto decimals = BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
        static int id = Editor::GetActionId();
        opts.Decimals = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(decimals, "The number of decimals to show, if a percentage");
}
void Sources::Text::PPUI(GameObject* parent, UnparsedJSON unparsed) {
    static PP opts;
    opts = unparsed.Parse<PP>();

    auto source = MUI::CreateDropdownEnum(parent, "Source", opts.Leaderboard, PPLeaderboardStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Leaderboard = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(source, "The leaderboard to show the PP value for");

    auto decimals = BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
        static int id = Editor::GetActionId();
        opts.Decimals = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(decimals, "The number of decimals to show");
}
void Sources::Text::SaberSpeedUI(GameObject* parent, UnparsedJSON unparsed) {
    static SaberSpeed opts;
    opts = unparsed.Parse<SaberSpeed>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to show the speed of");

    auto decimals = BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
        static int id = Editor::GetActionId();
        opts.Decimals = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(decimals, "The number of decimals to show");

    auto mode = MUI::CreateDropdownEnum(parent, "Mode", opts.Mode, SaberSpeedModeStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Mode = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(mode, "How to show the average saber speed");
}
void Sources::Text::SpinometerUI(GameObject* parent, UnparsedJSON unparsed) {
    static Spinometer opts;
    opts = unparsed.Parse<Spinometer>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to show the angular velocity of");

    auto mode = MUI::CreateDropdownEnum(parent, "Mode", opts.Mode, SpinometerModeStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Mode = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(mode, "How to show the angular velocity");
}
void Sources::Text::FCPercentUI(GameObject* parent, UnparsedJSON unparsed) {
    static FCPercent opts;
    opts = unparsed.Parse<FCPercent>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to show the full combo percentage for");

    auto decimals = BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, opts.Decimals, 0, 10, [](float val) {
        static int id = Editor::GetActionId();
        opts.Decimals = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(decimals, "The number of decimals to show");
}

void Sources::Text::CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options) {
    auto trans = parent->transform;
    while (trans->GetChildCount() > 0)
        UnityEngine::Object::DestroyImmediate(trans->GetChild(0)->gameObject);

    auto fn = GetSource(Sources::texts, source).second;
    fn(parent, options);
}

void Sources::Shape::StaticUI(GameObject* parent, UnparsedJSON unparsed) {
    static Static opts;
    opts = unparsed.Parse<Static>();

    auto slider = BSML::Lite::CreateSliderSetting(parent, "Fill", 0.01, opts.Input, 0, 1, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        opts.Input = val;
        Editor::SetSourceOptions(id, opts);
    });
    MUI::AddSliderEndDrag(slider, [](float) { Editor::FinalizeAction(); });
    slider->GetComponent<RectTransform*>()->sizeDelta = {0, 8};
    BSML::Lite::AddHoverHint(slider, "The static fill level to use");
}
void Sources::Shape::ScoreUI(GameObject* parent, UnparsedJSON unparsed) {
    static Score opts;
    opts = unparsed.Parse<Score>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to use the score of");
}
void Sources::Shape::MultiplierUI(GameObject* parent, UnparsedJSON unparsed) {
    static Multiplier opts;
    opts = unparsed.Parse<Multiplier>();

    auto absolute = BSML::Lite::CreateToggle(parent, "Absolute", opts.Absolute, [](bool val) {
        static int id = Editor::GetActionId();
        opts.Absolute = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(absolute, "Use the absolute multiplier progress instead of the progress to the next increment");
}
void Sources::Shape::HealthUI(GameObject* parent, UnparsedJSON unparsed) {
    // static Health opts;
    // opts = unparsed.Parse<Health>();
}
void Sources::Shape::TimeUI(GameObject* parent, UnparsedJSON unparsed) {
    // static Time opts;
    // opts = unparsed.Parse<Time>();
}
void Sources::Shape::AverageCutUI(GameObject* parent, UnparsedJSON unparsed) {
    static AverageCut opts;
    opts = unparsed.Parse<AverageCut>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to use the average cut of");

    auto part = MUI::CreateDropdownEnum(parent, "Part", opts.Part, AverageCutPartStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Part = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(part, "The part of the cut to use the average of");
}
void Sources::Shape::NotesUI(GameObject* parent, UnparsedJSON unparsed) {
    static Notes opts;
    opts = unparsed.Parse<Notes>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetSourceOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to use the notes value of");
}

void Sources::Shape::CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options) {
    auto trans = parent->transform;
    while (trans->GetChildCount() > 0)
        UnityEngine::Object::DestroyImmediate(trans->GetChild(0)->gameObject);

    auto fn = GetSource(Sources::shapes, source).second;
    fn(parent, options);
}

static std::vector<std::string_view> const ColorSettingsStrings = {
    "Left Saber",
    "Right Saber",
    "Lights 1",
    "Lights 2",
    "Walls",
    "Boost Lights 1",
    "Boost Lights 2",
};

void Sources::Color::StaticUI(GameObject* parent, UnparsedJSON unparsed) {
    static Static opts;
    opts = unparsed.Parse<Static>();

    auto color = Utils::CreateColorPicker(
        parent,
        "Color",
        opts.Input,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.Input = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(color->editButton, "The constant color to use");
}
void Sources::Color::PlayerUI(GameObject* parent, UnparsedJSON unparsed) {
    static Player opts;
    opts = unparsed.Parse<Player>();

    auto setting = MUI::CreateDropdownEnum(parent, "Color Setting", opts.Setting, ColorSettingsStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Setting = val;
        Editor::SetColorOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(setting, "Which setting from the color scheme to use");
}
void Sources::Color::RankUI(GameObject* parent, UnparsedJSON unparsed) {
    static Rank opts;
    opts = unparsed.Parse<Rank>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetColorOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to use the score of for the rank calculation");

    auto posMods = BSML::Lite::CreateToggle(parent, "Positive Modifiers", opts.PositiveModifiers, [](bool val) {
        static int id = Editor::GetActionId();
        opts.PositiveModifiers = val;
        Editor::SetColorOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(posMods, "Include positive modifiers in the rank calculation");

    auto negMods = BSML::Lite::CreateToggle(parent, "Negative Modifiers", opts.NegativeModifiers, [](bool val) {
        static int id = Editor::GetActionId();
        opts.NegativeModifiers = val;
        Editor::SetColorOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(negMods, "Include negative modifiers in the rank calculation");

    auto sss = Utils::CreateColorPicker(
        parent,
        "SSS Color",
        opts.SSS,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.SSS = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(sss, "The color to use for SSS rank");

    auto ss = Utils::CreateColorPicker(
        parent,
        "SS Color",
        opts.SS,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.SS = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(ss, "The color to use for SS rank");

    auto s = Utils::CreateColorPicker(
        parent,
        "S Color",
        opts.S,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.S = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(s, "The color to use for S rank");

    auto a = Utils::CreateColorPicker(
        parent,
        "A Color",
        opts.A,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.A = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(a, "The color to use for A rank");

    auto b = Utils::CreateColorPicker(
        parent,
        "B Color",
        opts.B,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.B = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(b, "The color to use for B rank");

    auto c = Utils::CreateColorPicker(
        parent,
        "C Color",
        opts.C,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.C = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(c, "The color to use for C rank");

    auto d = Utils::CreateColorPicker(
        parent,
        "D Color",
        opts.D,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.D = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(d, "The color to use for D rank");

    auto e = Utils::CreateColorPicker(
        parent,
        "E Color",
        opts.E,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.E = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(e, "The color to use for E rank");
}
void Sources::Color::PersonalBestUI(GameObject* parent, UnparsedJSON unparsed) {
    static PersonalBest opts;
    opts = unparsed.Parse<PersonalBest>();

    auto better = Utils::CreateColorPicker(
        parent,
        "Better Color",
        opts.Better,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.Better = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(better, "The color to use if the current percentage is better than your personal best");

    auto worse = Utils::CreateColorPicker(
        parent,
        "Worse Color",
        opts.Worse,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.Worse = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(worse, "The color to use if the current percentage is worse than your personal best");
}
void Sources::Color::ComboUI(GameObject* parent, UnparsedJSON unparsed) {
    static Combo opts;
    opts = unparsed.Parse<Combo>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetColorOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to use the score of");

    auto fc = Utils::CreateColorPicker(
        parent,
        "FC Color",
        opts.Full,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.Full = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(fc, "The color to use for a full combo");

    auto nonFc = Utils::CreateColorPicker(
        parent,
        "Non FC Color",
        opts.NonFull,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.NonFull = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(nonFc, "The color to use for a non full combo");
}
void Sources::Color::MultiplierUI(GameObject* parent, UnparsedJSON unparsed) {
    static Multiplier opts;
    opts = unparsed.Parse<Multiplier>();

    auto one = Utils::CreateColorPicker(
        parent,
        "One",
        opts.One,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.One = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(one, "The color to use for a multiplier of 1");

    auto two = Utils::CreateColorPicker(
        parent,
        "Two",
        opts.Two,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.Two = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(two, "The color to use for a multiplier of 2");

    auto four = Utils::CreateColorPicker(
        parent,
        "Four",
        opts.Four,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.Four = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(four, "The color to use for a multiplier of 4");

    auto eight = Utils::CreateColorPicker(
        parent,
        "Eight",
        opts.Eight,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.Eight = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(eight, "The color to use for a multiplier of 8");
}
void Sources::Color::HealthUI(GameObject* parent, UnparsedJSON unparsed) {
    static Health opts;
    opts = unparsed.Parse<Health>();

    auto full = Utils::CreateColorPicker(
        parent,
        "Full",
        opts.Full,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.Full = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(full, "The color to use for full health");

    auto overHalf = Utils::CreateColorPicker(
        parent,
        "Above Half",
        opts.AboveHalf,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.AboveHalf = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(overHalf, "The color to use for above half health");

    auto underHalf = Utils::CreateColorPicker(
        parent,
        "Below Half",
        opts.BelowHalf,
        [](UnityEngine::Color val) {
            static int id = Editor::GetActionId();
            opts.BelowHalf = val;
            Editor::SetColorOptions(id, opts);
        },
        Editor::FinalizeAction
    );
    BSML::Lite::AddHoverHint(underHalf, "The color to use for below half health");
}

void Sources::Color::CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options) {
    auto trans = parent->transform;
    while (trans->GetChildCount() > 0)
        UnityEngine::Object::DestroyImmediate(trans->GetChild(0)->gameObject);

    auto fn = GetSource(Sources::colors, source).second;
    fn(parent, options);
}

void Sources::Enable::StaticUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    // static Static opts;
    // opts = unparsed.Parse<Static>();
}
void Sources::Enable::RankedUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    static Ranked opts;
    opts = unparsed.Parse<Ranked>();

    auto leaderboard = MUI::CreateDropdownEnum(parent, "Leaderboard", opts.Leaderboard, RankedStatusLeaderboardStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Leaderboard = val;
        Editor::SetEnableOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(leaderboard, "The leaderboard to be ranked on");
}
void Sources::Enable::FullComboUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    static FullCombo opts;
    opts = unparsed.Parse<FullCombo>();

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetEnableOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber that must have a full combo");
}
void Sources::Enable::PercentageUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    static Percentage opts;
    opts = unparsed.Parse<Percentage>();

    auto slider = BSML::Lite::CreateSliderSetting(parent, "Percentage", 1, opts.Percent, 0, 100, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        opts.Percent = val;
        Editor::SetEnableOptions(id, opts);
    });
    MUI::AddSliderEndDrag(slider, [](float) { Editor::FinalizeAction(); });
    slider->GetComponent<RectTransform*>()->sizeDelta = {0, 8};
    BSML::Lite::AddHoverHint(slider, "The percentage the score must be above");

    auto saber = MUI::CreateDropdownEnum(parent, "Saber", opts.Saber, Options::SaberStrings, [](int val) {
        static int id = Editor::GetActionId();
        opts.Saber = val;
        Editor::SetEnableOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(saber, "The saber to use the score of for the percentage calculation");
}
void Sources::Enable::FailedUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    // static Failed opts;
    // opts = unparsed.Parse<Failed>();
}

void Sources::Enable::CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options) {
    auto trans = parent->transform;
    while (trans->GetChildCount() > 0)
        UnityEngine::Object::DestroyImmediate(trans->GetChild(0)->gameObject);

    auto fn = GetSource(Sources::enables, source).second;
    fn(parent, options);
}
