#include "templates.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "config.hpp"
#include "customtypes/components.hpp"
#include "customtypes/settings.hpp"
#include "editor.hpp"
#include "main.hpp"
#include "options.hpp"
#include "sources.hpp"
#include "utils.hpp"

using namespace Qounters;

std::vector<std::pair<std::string, Types::TemplateUIFn>> Templates::registration = {
    {"Blank", Templates::EmptyUI},
    {"Score", Templates::ScoreUI},
    {"Personal Best", Templates::PersonalBestUI},
    {"Average Cut", Templates::AverageCutUI},
    {"Time Dependence", Templates::TimeDependenceUI},
    {"Notes", Templates::NotesUI},
    {"Mistakes", Templates::MistakesUI},
    {"Fails", Templates::FailsUI},
    {"Song Time", Templates::SongTimeUI},
    {"PP", Templates::PPUI},
    {"Saber Speed", Templates::SaberSpeedUI},
    {"Spinometer", Templates::SpinometerUI},
    {"FC Percentage", Templates::FCPercentUI},
};

static std::vector<std::string_view> const NotesDisplayStrings = {
    "Cut",
    "Remaining",
    "Cut Ratio",
    "Cut Percent",
};

static std::vector<std::string_view> const SongTimeDisplayStrings = {
    "Base Game",
    "Ring",
    "Percentage",
};

void Templates::CloseModal() {
    TemplatesViewController::GetInstance()->HideModal();
}

static Options::Group MakeGroup(int anchor, UnityEngine::Vector2 pos) {
    Options::Group group;
    group.Anchor = anchor;
    group.Position = pos;
    return group;
}

static void CreateAnchorDropdown(UnityEngine::GameObject* parent, int& anchor) {
    BSML::Lite::AddHoverHint(
        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, Options::AnchorStrings, [&anchor](int val) { anchor = val; }),
        "Select the anchor to create this counter group on"
    );
}

static void CreateButtons(UnityEngine::GameObject* parent, std::function<void()> createFn) {
    auto buttons = BSML::Lite::CreateHorizontalLayoutGroup(parent);
    buttons->spacing = 3;
    auto cancelButton = BSML::Lite::CreateUIButton(buttons, "Cancel", Templates::CloseModal);
    BSML::Lite::AddHoverHint(cancelButton, "Close the menu without creating anything");
    auto createButton = BSML::Lite::CreateUIButton(buttons, "Create", "ActionButton", [createFn]() {
        createFn();
        Templates::CloseModal();
    });
    BSML::Lite::AddHoverHint(createButton, "Create the counter group and add it to the preset");
}

static Options::Component& AddText(
    Options::Group& group, std::string source, UnparsedJSON sourceOpts, float size = 15, Options::Text::Aligns align = Options::Text::Aligns::Center
) {
    auto& ret = group.Components.emplace_back();
    ret.Type = (int) Options::Component::Types::Text;
    Options::Text opts;
    opts.Align = (int) align;
    opts.Size = size;
    opts.TextSource = source;
    opts.SourceOptions = sourceOpts;
    ret.Options = opts;
    return ret;
}

void Templates::AddEmpty(int anchor, UnityEngine::Vector2 pos) {
    Editor::AddGroup(MakeGroup(anchor, pos));
}

void Templates::AddScore(int anchor, UnityEngine::Vector2 pos, bool score, bool percent, bool rank, int decimals, bool rankColors) {
    auto group = MakeGroup(anchor, pos);
    float yPos = 0;
    if (rank) {
        auto& text = AddText(group, Sources::Text::RankName, Sources::Text::Rank(), 33);
        yPos += 18;
        if (rankColors) {
            text.ColorSource = Sources::Color::RankName;
            text.ColorOptions = Sources::Color::Rank();
        }
    }
    if (percent) {
        Sources::Text::Score opts;
        opts.Decimals = decimals;
        AddText(group, Sources::Text::ScoreName, opts, 12).Position = UnityEngine::Vector2(0, yPos);
        yPos += 15;
    }
    if (score) {
        Sources::Text::Score scoreOpts;
        scoreOpts.Percentage = false;
        AddText(group, Sources::Text::ScoreName, scoreOpts, 15).Position = UnityEngine::Vector2(0, yPos);
    }
    Editor::AddGroup(group);
}
void Templates::AddPersonalBest(int anchor, UnityEngine::Vector2 pos, bool absolute, bool hideFirst, int decimals) {
    auto group = MakeGroup(anchor, pos);
    Sources::Text::PersonalBest opts;
    opts.Percentage = !absolute;
    opts.HideFirstScore = hideFirst;
    opts.Decimals = decimals;
    AddText(group, Sources::Text::PersonalBestName, opts, 10);
    Editor::AddGroup(group);
}
void Templates::AddAverageCut(int anchor, UnityEngine::Vector2 pos, bool splitSaber, bool splitCut, int decimals) {
    auto group = MakeGroup(anchor, pos);
    Sources::Text::Static label;
    label.Input = "Average Cut";
    AddText(group, Sources::Text::StaticName, label, 11);
    auto addLine = [&splitSaber, &group](Sources::Text::AverageCut opts, float yPos) {
        if (splitSaber) {
            opts.Saber = (int) Types::Sabers::Left;
            AddText(group, Sources::Text::AverageCutName, opts, 15, Options::Text::Aligns::Right).Position = UnityEngine::Vector2(-2, yPos);
            opts.Saber = (int) Types::Sabers::Right;
            AddText(group, Sources::Text::AverageCutName, opts, 15, Options::Text::Aligns::Left).Position = UnityEngine::Vector2(2, yPos);
        } else
            AddText(group, Sources::Text::AverageCutName, opts).Position = UnityEngine::Vector2(0, yPos);
    };
    Sources::Text::AverageCut opts;
    opts.Decimals = decimals;
    if (splitCut) {
        float yPos = 12.5;
        opts.Part = (int) Sources::Text::AverageCut::Parts::Pre;
        addLine(opts, -yPos);
        yPos += 14;
        opts.Part = (int) Sources::Text::AverageCut::Parts::Post;
        addLine(opts, -yPos);
        yPos += 14;
        opts.Part = (int) Sources::Text::AverageCut::Parts::Acc;
        addLine(opts, -yPos);
    } else
        addLine(opts, -12.5);
    Editor::AddGroup(group);
}
void Templates::AddTimeDependence(int anchor, UnityEngine::Vector2 pos, bool splitSaber, int decimals, int decimalOffset) {
    auto group = MakeGroup(anchor, pos);
    Sources::Text::Static label;
    label.Input = "Time Dependence";
    AddText(group, Sources::Text::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
    Sources::Text::TimeDependence opts;
    opts.Decimals = decimals;
    opts.DecimalOffset = decimalOffset;
    if (splitSaber) {
        opts.Saber = (int) Types::Sabers::Left;
        AddText(group, Sources::Text::TimeDependenceName, opts, 15, Options::Text::Aligns::Right).Position = UnityEngine::Vector2(-2, 0);
        opts.Saber = (int) Types::Sabers::Right;
        AddText(group, Sources::Text::TimeDependenceName, opts, 15, Options::Text::Aligns::Left).Position = UnityEngine::Vector2(2, 0);
    } else
        AddText(group, Sources::Text::TimeDependenceName, opts);
    Editor::AddGroup(group);
}
void Templates::AddNotes(int anchor, UnityEngine::Vector2 pos, int display, int decimals) {
    auto group = MakeGroup(anchor, pos);
    Sources::Text::Static label;
    switch ((Sources::Text::Notes::Displays) display) {
        case Sources::Text::Notes::Displays::Cut:
        case Sources::Text::Notes::Displays::Ratio:
        case Sources::Text::Notes::Displays::Percent:
            label.Input = "Notes Cut";
            break;
        case Sources::Text::Notes::Displays::Remaining:
            label.Input = "Notes Remaining";
            break;
    }
    AddText(group, Sources::Text::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
    Sources::Text::Notes opts;
    opts.Display = display;
    opts.Decimals = decimals;
    AddText(group, Sources::Text::NotesName, opts);
    Editor::AddGroup(group);
}
void Templates::AddMistakes(int anchor, UnityEngine::Vector2 pos, bool badCuts, bool bombs, bool walls) {
    auto group = MakeGroup(anchor, pos);
    Sources::Text::Static label;
    label.Input = "Mistakes";
    AddText(group, Sources::Text::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
    Sources::Text::Mistakes opts;
    opts.BadCuts = badCuts;
    opts.Bombs = bombs;
    opts.Walls = walls;
    AddText(group, Sources::Text::MistakesName, opts);
    Editor::AddGroup(group);
}
void Templates::AddFails(int anchor, UnityEngine::Vector2 pos, bool restarts) {
    auto group = MakeGroup(anchor, pos);
    Sources::Text::Static label;
    label.Input = restarts ? "Restarts" : "Fails";
    AddText(group, Sources::Text::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
    Sources::Text::Fails opts;
    opts.Restarts = restarts;
    AddText(group, Sources::Text::FailsName, opts);
    Editor::AddGroup(group);
}
void Templates::AddSongTime(int anchor, UnityEngine::Vector2 pos, int display, bool timeLeft) {
    auto group = MakeGroup(anchor, pos);
    switch (display) {
        case 0: {  // Base Game
            auto& comp = group.Components.emplace_back();
            comp.Type = (int) Options::Component::Types::Premade;
            Options::Premade premade;
            premade.Name = Options::BaseGameObjectStrings[(int) BaseGameGraphic::Objects::ProgressBar];
            comp.Options = premade;
            break;
        }
        case 1: {  // Ring
            Sources::Text::Time timeOpts;
            timeOpts.Remaining = timeLeft;
            AddText(group, Sources::Text::TimeName, timeOpts);
            auto& comp = group.Components.emplace_back();
            comp.Type = (int) Options::Component::Types::Shape;
            Options::Shape opts;
            opts.Shape = (int) Options::Shape::Shapes::CircleOutline;
            opts.Fill = (int) Options::Shape::Fills::Circle;
            opts.FillSource = Sources::Shape::TimeName;
            opts.Inverse = timeLeft;
            comp.Options = opts;
            comp.Scale = ConfigUtils::Vector2(1.5, 1.5);
            break;
        }
        default: {  // Percentage
            Sources::Text::Time timeOpts;
            timeOpts.Percentage = true;
            timeOpts.Remaining = timeLeft;
            AddText(group, Sources::Text::TimeName, timeOpts);
            break;
        }
    }
    Editor::AddGroup(group);
}
void Templates::AddPP(int anchor, UnityEngine::Vector2 pos, bool beatleader, bool scoresaber, bool hideUnranked, int decimals) {
    auto group = MakeGroup(anchor, pos);
    float yPos = 0;
    if (beatleader) {
        Sources::Text::PP ppOpts;
        ppOpts.Leaderboard = (int) Sources::Text::PP::Leaderboards::BeatLeader;
        ppOpts.Decimals = decimals;
        AddText(group, Sources::Text::PPName, ppOpts, 12, Options::Text::Aligns::Left);
        auto& image = group.Components.emplace_back();
        auto& text = *(group.Components.end() - 2);
        yPos += 15;
        image.Type = (int) Options::Component::Types::Image;
        Options::Image opts;
        opts.Path = "Beatleader.png";
        image.Options = opts;
        image.Scale = UnityEngine::Vector2(0.5, 0.5);
        image.Position = UnityEngine::Vector2(-10, 1);
        if (hideUnranked) {
            Sources::Enable::Ranked enableOpts;
            enableOpts.Leaderboard = (int) Sources::Enable::Ranked::Leaderboards::BeatLeader;
            text.EnableSource = image.EnableSource = Sources::Enable::RankedName;
            text.EnableOptions = image.EnableOptions = enableOpts;
        }
    }
    if (scoresaber) {
        Sources::Text::PP ppOpts;
        ppOpts.Leaderboard = (int) Sources::Text::PP::Leaderboards::ScoreSaber;
        ppOpts.Decimals = decimals;
        AddText(group, Sources::Text::PPName, ppOpts, 12, Options::Text::Aligns::Left);
        auto& image = group.Components.emplace_back();
        auto& text = *(group.Components.end() - 2);
        text.Position = UnityEngine::Vector2(0, yPos);
        image.Type = (int) Options::Component::Types::Image;
        Options::Image opts;
        opts.Path = "Scoresaber.png";
        image.Options = opts;
        image.Scale = UnityEngine::Vector2(0.5, 0.5);
        image.Position = UnityEngine::Vector2(-10, yPos + 1);
        if (hideUnranked) {
            Sources::Enable::Ranked enableOpts;
            enableOpts.Leaderboard = (int) Sources::Enable::Ranked::Leaderboards::ScoreSaber;
            text.EnableSource = image.EnableSource = Sources::Enable::RankedName;
            text.EnableOptions = image.EnableOptions = enableOpts;
        }
    }
    Editor::AddGroup(group);
}
void Templates::AddSaberSpeed(int anchor, UnityEngine::Vector2 pos, bool split, bool last5Secs, int decimals) {
    auto group = MakeGroup(anchor, pos);
    Sources::Text::Static label;
    label.Input = "Saber Speed";
    AddText(group, Sources::Text::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
    Sources::Text::SaberSpeed opts;
    opts.Decimals = decimals;
    opts.Mode = (int) (last5Secs ? Sources::Text::SaberSpeed::Modes::Best5Seconds : Sources::Text::SaberSpeed::Modes::Average);
    if (split) {
        opts.Saber = (int) Types::Sabers::Left;
        AddText(group, Sources::Text::SaberSpeedName, opts, 15, Options::Text::Aligns::Right).Position = UnityEngine::Vector2(-2, 0);
        opts.Saber = (int) Types::Sabers::Right;
        AddText(group, Sources::Text::SaberSpeedName, opts, 15, Options::Text::Aligns::Left).Position = UnityEngine::Vector2(2, 0);
    } else
        AddText(group, Sources::Text::SaberSpeedName, opts, 15);
    Editor::AddGroup(group);
}
void Templates::AddSpinometer(int anchor, UnityEngine::Vector2 pos, bool split, bool highest) {
    auto group = MakeGroup(anchor, pos);
    Sources::Text::Static label;
    label.Input = "Spinometer";
    AddText(group, Sources::Text::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
    Sources::Text::Spinometer opts;
    opts.Mode = (int) (highest ? Sources::Text::Spinometer::Modes::Highest : Sources::Text::Spinometer::Modes::Average);
    if (split) {
        opts.Saber = (int) Types::Sabers::Left;
        AddText(group, Sources::Text::SpinometerName, opts, 15, Options::Text::Aligns::Right).Position = UnityEngine::Vector2(-2, 0);
        opts.Saber = (int) Types::Sabers::Right;
        AddText(group, Sources::Text::SpinometerName, opts, 15, Options::Text::Aligns::Left).Position = UnityEngine::Vector2(2, 0);
    } else
        AddText(group, Sources::Text::SpinometerName, opts, 15);
    Editor::AddGroup(group);
}
void Templates::AddFCPercent(int anchor, UnityEngine::Vector2 pos, bool split, bool saberColors, bool hideInFC, int decimals) {
    auto group = MakeGroup(anchor, pos);
    Sources::Text::Static label;
    label.Input = "FC Percentage";
    AddText(group, Sources::Text::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
    Sources::Text::FCPercent opts;
    opts.Decimals = decimals;
    if (split) {
        opts.Saber = (int) Types::Sabers::Left;
        AddText(group, Sources::Text::FCPercentName, opts, 15, Options::Text::Aligns::Right);
        opts.Saber = (int) Types::Sabers::Right;
        auto& rightText = AddText(group, Sources::Text::FCPercentName, opts, 15, Options::Text::Aligns::Left);
        auto& leftText = *(group.Components.end() - 2);
        leftText.Position = UnityEngine::Vector2(-2, 0);
        rightText.Position = UnityEngine::Vector2(2, 0);
        if (saberColors) {
            Sources::Color::Player colOpts;
            colOpts.Setting = (int) Sources::Color::Player::ColorSettings::LeftSaber;
            leftText.ColorSource = Sources::Color::PlayerName;
            leftText.ColorOptions = colOpts;
            colOpts.Setting = (int) Sources::Color::Player::ColorSettings::RightSaber;
            rightText.ColorSource = Sources::Color::PlayerName;
            rightText.ColorOptions = colOpts;
        }
        if (hideInFC) {
            Sources::Enable::FullCombo showOpts;
            showOpts.Saber = (int) Types::Sabers::Left;
            leftText.EnableSource = Sources::Enable::FullComboName;
            leftText.InvertEnable = true;
            leftText.EnableOptions = showOpts;
            showOpts.Saber = (int) Types::Sabers::Right;
            rightText.EnableSource = Sources::Enable::FullComboName;
            rightText.InvertEnable = true;
            rightText.EnableOptions = showOpts;
        }
    } else {
        auto& text = AddText(group, Sources::Text::FCPercentName, opts, 15);
        if (hideInFC) {
            text.EnableSource = Sources::Enable::FullComboName;
            text.EnableOptions = Sources::Enable::FullCombo();
        }
    }
    Editor::AddGroup(group);
}

void Templates::EmptyUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;

    CreateAnchorDropdown(parent, anchor);
    CreateButtons(parent, []() { AddEmpty(anchor, {0, 0}); });
}
void Templates::ScoreUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static bool score = false;
    static bool percent = true;
    static bool rank = true;
    static int decimals = 2;
    static bool rankColors = true;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Show Numeric Score", score, [](bool val) { score = val; }), "Shows the absolute score value"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Show Percentage", percent, [](bool val) { percent = val; }),
        "Shows the current score percentage of the maximum"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Show Rank", rank, [](bool val) { rank = val; }), "Shows the rank of the current score percentage"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateIncrementSetting(parent, "Percentage Decimals", 0, 1, decimals, [](float val) { decimals = val; }),
        "The number of decimals in the score percentage"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Color Rank Text", rankColors, [](bool val) { rankColors = val; }),
        "Make the text of the rank change colors based on its value"
    );
    CreateButtons(parent, []() { AddScore(anchor, {0, 0}, score, percent, rank, decimals, rankColors); });
}
void Templates::PersonalBestUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static bool absolute = false;
    static bool hideFirst = true;
    static int decimals = 2;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Absolute Score", absolute, [](bool val) { absolute = val; }),
        "Show the absolute personal best value of the personal best instead of the percentage"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Show 0 On First Score", !hideFirst, [](bool val) { hideFirst = !val; }),
        "Shows 0 if you have no personal best instead of \"--\""
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; }),
        "The number of decimals in the personal best percentage, if shown"
    );
    CreateButtons(parent, []() { AddPersonalBest(anchor, {0, 0}, absolute, hideFirst, decimals); });
}
void Templates::AverageCutUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static bool splitSaber = true;
    static bool splitCut = false;
    static int decimals = 2;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Split Sabers", splitSaber, [](bool val) { splitSaber = val; }), "Show separate averages per saber"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Split Cut Parts", splitCut, [](bool val) { splitCut = val; }),
        "Show separate averages for the preswing, postswing, and accuracy"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; }),
        "The number of decimals in the averages"
    );
    CreateButtons(parent, []() { AddAverageCut(anchor, {0, 0}, splitSaber, splitCut, decimals); });
}
void Templates::TimeDependenceUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static bool splitSaber = true;
    static int decimals = 2;
    static int decimalOffset = 0;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Split Sabers", splitSaber, [](bool val) { splitSaber = val; }), "Show separate time dependencies per saber"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; }),
        "The number of decimals (after the decimal point) to show"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateIncrementSetting(parent, "Decimal Offset", 0, 1, decimalOffset, [](float val) { decimalOffset = val; }),
        "Multiplies the time dependence, so 0.1 could be displayed as 1.0 or 10.0"
    );
    CreateButtons(parent, []() { AddTimeDependence(anchor, {0, 0}, splitSaber, decimals, decimalOffset); });
}
void Templates::NotesUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static int display = 0;
    static int decimals = 2;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        Utils::CreateDropdownEnum(parent, "Display Type", display, NotesDisplayStrings, [](int val) { display = val; }),
        "The note-related value to show"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateIncrementSetting(parent, "Percentage Decimals", 0, 1, decimals, [](float val) { decimals = val; }),
        "The number of decimals in any percentages"
    );
    CreateButtons(parent, []() { AddNotes(anchor, {0, 0}, display, decimals); });
}
void Templates::MistakesUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static bool badCuts = true;
    static bool bombs = false;
    static bool walls = false;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Count Bad Cuts", badCuts, [](bool val) { badCuts = val; }), "Include bad cuts in the mistake count"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Count Bombs", bombs, [](bool val) { bombs = val; }), "Include bomb hits in the mistake count"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Count Walls", walls, [](bool val) { walls = val; }), "Include wall hits in the mistake count"
    );
    CreateButtons(parent, []() { AddMistakes(anchor, {0, 0}, badCuts, bombs, walls); });
}
void Templates::FailsUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static bool restarts = false;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Count Restarts Instead", restarts, [](bool val) { restarts = val; }),
        "Count the number of consecutive map restarts instead of total fails"
    );
    CreateButtons(parent, []() { AddFails(anchor, {0, 0}, restarts); });
}
void Templates::SongTimeUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static int display = 0;
    static bool timeLeft = false;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        Utils::CreateDropdownEnum(parent, "Display Type", display, SongTimeDisplayStrings, [](int val) { display = val; }),
        "How to display the song time"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Time Remaining", timeLeft, [](bool val) { timeLeft = val; }),
        "Show the time remaining instead of current time for non-base game displays"
    );
    CreateButtons(parent, []() { AddSongTime(anchor, {0, 0}, display, timeLeft); });
}
void Templates::PPUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static bool beatleader = true;
    static bool scoresaber = true;
    static bool hideUnranked = true;
    static int decimals = 2;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Show ScoreSaber", scoresaber, [](bool val) { scoresaber = val; }), "Show ScoreSaber PP value"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Show BeatLeader", beatleader, [](bool val) { beatleader = val; }), "Show BeatLeader PP value"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Hide When Unranked", hideUnranked, [](bool val) { hideUnranked = val; }),
        "Disable displaying individual leaderboards when the map is not ranked on them"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; }),
        "The number of decimals in the PP values"
    );
    CreateButtons(parent, []() { AddPP(anchor, {0, 0}, beatleader, scoresaber, hideUnranked, decimals); });
}
void Templates::SaberSpeedUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static bool split = false;
    static bool last5Secs = true;
    static int decimals = 2;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Split Sabers", split, [](bool val) { split = val; }), "Show separate speeds per saber"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Last 5 Seconds Only", last5Secs, [](bool val) { last5Secs = val; }),
        "Show the average speed from the last 5 seconds instead of all time"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; }),
        "The number of decimals in the saber speeds"
    );
    CreateButtons(parent, []() { AddSaberSpeed(anchor, {0, 0}, split, last5Secs, decimals); });
}
void Templates::SpinometerUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static bool split = false;
    static bool highest = false;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Split Sabers", split, [](bool val) { split = val; }), "Show separate spinometers per saber"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Use Highest", highest, [](bool val) { highest = val; }),
        "Use the overall highest angular velocity instead of from the last second"
    );
    CreateButtons(parent, []() { AddSpinometer(anchor, {0, 0}, split, highest); });
}
void Templates::FCPercentUI(UnityEngine::GameObject* parent) {
    static int anchor = 0;
    static bool split = false;
    static bool saberColors = false;
    static bool hideInFC = true;
    static int decimals = 2;

    CreateAnchorDropdown(parent, anchor);
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Split Sabers", split, [](bool val) { split = val; }), "Show separate full combo percentages per saber"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Use Saber Colors", saberColors, [](bool val) { saberColors = val; }),
        "Set FC percentages to their respective saber colors, if separate"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Hide With FC", hideInFC, [](bool val) { hideInFC = val; }), "Hide FC percentages with a full combo"
    );
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; }),
        "The number of decimals in the FC percentages"
    );
    CreateButtons(parent, []() { AddFCPercent(anchor, {0, 0}, split, saberColors, hideInFC, decimals); });
}
