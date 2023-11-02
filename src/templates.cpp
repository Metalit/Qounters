#include "templates.hpp"
#include "config.hpp"
#include "editor.hpp"
#include "main.hpp"
#include "sources.hpp"
#include "utils.hpp"
#include "customtypes/settings.hpp"

using namespace Qounters;
using namespace QuestUI;

std::vector<std::pair<std::string, TemplateUIFn>> Qounters::templates = {
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
};

const std::vector<std::string> NotesDisplayStrings = {
    "Cut",
    "Remaining",
    "Cut Ratio",
    "Cut Percent",
};

const std::vector<std::string> SongTimeDisplayStrings = {
    "Base Game",
    "Ring",
    "Percentage",
};

namespace Qounters::Templates {
    void CloseModal() {
        TemplatesViewController::GetInstance()->HideModal();
    }

    Group MakeGroup(int anchor, UnityEngine::Vector2 pos) {
        Group group;
        group.Anchor = anchor;
        group.Position = pos;
        return group;
    }

    void CreateButtons(UnityEngine::GameObject* parent, std::function<void ()> createFn) {
        auto buttons = BeatSaberUI::CreateHorizontalLayoutGroup(parent);
        buttons->set_spacing(3);
        BeatSaberUI::CreateUIButton(buttons, "Cancel", CloseModal);
        BeatSaberUI::CreateUIButton(buttons, "Create", "ActionButton", [createFn]() {
            createFn();
            CloseModal();
        });
    }

    Component& AddText(Group& group, std::string source, UnparsedJSON sourceOpts, float size = 15, TextOptions::Aligns align = TextOptions::Aligns::Center) {
        auto& ret = group.Components.emplace_back();
        ret.Type = (int) Component::Types::Text;
        TextOptions opts;
        opts.Align = (int) align;
        opts.Size = size;
        opts.TextSource = source;
        opts.SourceOptions = sourceOpts;
        ret.Options = opts;
        return ret;
    }

    void AddEmpty(int anchor, UnityEngine::Vector2 pos) {
        Editor::AddGroup(MakeGroup(anchor, pos));
    }

    void AddScore(int anchor, UnityEngine::Vector2 pos, bool score, bool percent, bool rank, int decimals, bool rankColors) {
        auto group = MakeGroup(anchor, pos);
        float yPos = 0;
        if (rank) {
            AddText(group, TextSource::RankName, TextSource::Rank(), 33);
            yPos += 18;
        }
        if (percent) {
            TextSource::Score opts;
            opts.Decimals = decimals;
            AddText(group, TextSource::ScoreName, opts, 12).Position = UnityEngine::Vector2(0, yPos);
            yPos += 15;
        }
        if (score) {
            TextSource::Score scoreOpts;
            scoreOpts.Percentage = false;
            AddText(group, TextSource::ScoreName, scoreOpts, 15).Position = UnityEngine::Vector2(0, yPos);
        }
        Editor::AddGroup(group);
    }
    void AddPersonalBest(int anchor, UnityEngine::Vector2 pos, bool absolute, bool hideFirst, int decimals) {
        auto group = MakeGroup(anchor, pos);
        TextSource::PersonalBest opts;
        opts.Percentage = !absolute;
        opts.HideFirstScore = hideFirst;
        opts.Decimals = decimals;
        AddText(group, TextSource::PersonalBestName, opts, 10);
        Editor::AddGroup(group);
    }
    void AddAverageCut(int anchor, UnityEngine::Vector2 pos, bool splitSaber, bool splitCut, int decimals) {
        auto group = MakeGroup(anchor, pos);
        TextSource::Static label;
        label.Input = "Average Cut";
        AddText(group, TextSource::StaticName, label, 11);
        auto addLine = [&splitSaber, &group](TextSource::AverageCut opts, float yPos) {
            if (splitSaber) {
                opts.Saber = (int) Sabers::Left;
                AddText(group, TextSource::AverageCutName, opts).Position = UnityEngine::Vector2(-10, yPos);
                opts.Saber = (int) Sabers::Right;
                AddText(group, TextSource::AverageCutName, opts).Position = UnityEngine::Vector2(10, yPos);
            } else
                AddText(group, TextSource::AverageCutName, opts).Position = UnityEngine::Vector2(0, yPos);
        };
        TextSource::AverageCut opts;
        opts.Decimals = decimals;
        if (splitCut) {
            float yPos = 12.5;
            opts.Part = (int) TextSource::AverageCut::Parts::Pre;
            addLine(opts, -yPos);
            yPos += 14;
            opts.Part = (int) TextSource::AverageCut::Parts::Post;
            addLine(opts, -yPos);
            yPos += 14;
            opts.Part = (int) TextSource::AverageCut::Parts::Acc;
            addLine(opts, -yPos);
        } else
            addLine(opts, -12.5);
        Editor::AddGroup(group);
    }
    void AddTimeDependence(int anchor, UnityEngine::Vector2 pos, bool splitSaber, int decimals, int decimalOffset) {
        auto group = MakeGroup(anchor, pos);
        TextSource::Static label;
        label.Input = "Time Dependence";
        AddText(group, TextSource::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
        TextSource::TimeDependence opts;
        opts.Decimals = decimals;
        opts.DecimalOffset = decimalOffset;
        if (splitSaber) {
            opts.Saber = (int) Sabers::Left;
            AddText(group, TextSource::TimeDependenceName, opts, 15, TextOptions::Aligns::Right).Position = UnityEngine::Vector2(-2, 0);
            opts.Saber = (int) Sabers::Right;
            AddText(group, TextSource::TimeDependenceName, opts, 15, TextOptions::Aligns::Left).Position = UnityEngine::Vector2(2, 0);
        } else
            AddText(group, TextSource::TimeDependenceName, opts);
        Editor::AddGroup(group);
    }
    void AddNotes(int anchor, UnityEngine::Vector2 pos, int display, int decimals) {
        auto group = MakeGroup(anchor, pos);
        TextSource::Static label;
        switch ((TextSource::Notes::Displays) display) {
            case TextSource::Notes::Displays::Cut:
            case TextSource::Notes::Displays::Ratio:
            case TextSource::Notes::Displays::Percent:
                label.Input = "Notes Cut";
                break;
            case TextSource::Notes::Displays::Remaining:
                label.Input = "Notes Remaining";
                break;
        }
        AddText(group, TextSource::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
        TextSource::Notes opts;
        opts.Display = display;
        opts.Decimals = decimals;
        AddText(group, TextSource::NotesName, opts);
        Editor::AddGroup(group);
    }
    void AddMistakes(int anchor, UnityEngine::Vector2 pos, bool badCuts, bool bombs, bool walls) {
        auto group = MakeGroup(anchor, pos);
        TextSource::Static label;
        label.Input = "Mistakes";
        AddText(group, TextSource::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
        TextSource::Mistakes opts;
        opts.BadCuts = badCuts;
        opts.Bombs = bombs;
        opts.Walls = walls;
        AddText(group, TextSource::MistakesName, opts);
        Editor::AddGroup(group);
    }
    void AddFails(int anchor, UnityEngine::Vector2 pos, bool restarts) {
        auto group = MakeGroup(anchor, pos);
        TextSource::Static label;
        label.Input = restarts ? "Restarts" : "Fails";
        AddText(group, TextSource::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
        TextSource::Fails opts;
        opts.Restarts = restarts;
        AddText(group, TextSource::FailsName, opts);
        Editor::AddGroup(group);
    }
    void AddSongTime(int anchor, UnityEngine::Vector2 pos, int display, bool timeLeft) {
        auto group = MakeGroup(anchor, pos);
        switch (display) {
            case 0: { // Base Game
                auto& comp = group.Components.emplace_back();
                comp.Type = (int) Component::Types::BaseGame;
                BaseGameOptions opts;
                opts.Component = (int) BaseGameOptions::Components::ProgressBar;
                comp.Options = opts;
                break;
            }
            case 1: { // Ring
                TextSource::Time timeOpts;
                timeOpts.Remaining = timeLeft;
                AddText(group, TextSource::TimeName, timeOpts);
                auto& comp = group.Components.emplace_back();
                comp.Type = (int) Component::Types::Shape;
                ShapeOptions opts;
                opts.Shape = (int) ShapeOptions::Shapes::CircleOutline;
                opts.Fill = (int) ShapeOptions::Fills::Circle;
                opts.FillSource = ShapeSource::TimeName;
                opts.Inverse = timeLeft;
                comp.Options = opts;
                break;
            }
            default: { // Percentage
                TextSource::Time timeOpts;
                timeOpts.Percentage = true;
                timeOpts.Remaining = timeLeft;
                AddText(group, TextSource::TimeName, timeOpts);
                break;
            }
        }
        Editor::AddGroup(group);
    }
    void AddPP(int anchor, UnityEngine::Vector2 pos, bool beatleader, bool scoresaber, bool hideUnranked, int decimals) {
        auto group = MakeGroup(anchor, pos);
        float yPos = 0;
        if (beatleader) {
            TextSource::PP ppOpts;
            ppOpts.Source = (int) TextSource::PP::Sources::BeatLeader;
            ppOpts.HideUnranked = hideUnranked;
            ppOpts.Decimals = decimals;
            AddText(group, TextSource::PPName, ppOpts, 12, TextOptions::Aligns::Left);
            yPos += 15;
            auto& comp = group.Components.emplace_back();
            comp.Type = (int) Component::Types::Image;
            ImageOptions opts;
            opts.Path = "Beatleader.png";
            comp.Options = opts;
            comp.Scale = UnityEngine::Vector2(0.5, 0.5);
            comp.Position = UnityEngine::Vector2(-10, 1);
        }
        if (scoresaber) {
            TextSource::PP ppOpts;
            ppOpts.Source = (int) TextSource::PP::Sources::ScoreSaber;
            ppOpts.HideUnranked = hideUnranked;
            ppOpts.Decimals = decimals;
            auto& text = AddText(group, TextSource::PPName, ppOpts, 12, TextOptions::Aligns::Left);
            text.Position = UnityEngine::Vector2(0, yPos);
            auto& comp = group.Components.emplace_back();
            comp.Type = (int) Component::Types::Image;
            ImageOptions opts;
            opts.Path = "Scoresaber.png";
            comp.Options = opts;
            comp.Scale = UnityEngine::Vector2(0.5, 0.5);
            comp.Position = UnityEngine::Vector2(-10, yPos + 1);
        }
        Editor::AddGroup(group);
    }
    void AddSaberSpeed(int anchor, UnityEngine::Vector2 pos, bool split, bool last5Secs, int decimals) {
        auto group = MakeGroup(anchor, pos);
        TextSource::Static label;
        label.Input = "Saber Speed";
        AddText(group, TextSource::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
        TextSource::SaberSpeed opts;
        opts.Decimals = decimals;
        opts.Mode = (int) (last5Secs ? TextSource::SaberSpeed::Modes::Best5Seconds : TextSource::SaberSpeed::Modes::Average);
        if (split) {
            opts.Saber = (int) Sabers::Left;
            AddText(group, TextSource::SaberSpeedName, opts, 15, TextOptions::Aligns::Right).Position = UnityEngine::Vector2(-2, 0);
            opts.Saber = (int) Sabers::Right;
            AddText(group, TextSource::SaberSpeedName, opts, 15, TextOptions::Aligns::Left).Position = UnityEngine::Vector2(2, 0);
        } else
            AddText(group, TextSource::SaberSpeedName, opts, 15);
        Editor::AddGroup(group);
    }
    void AddSpinometer(int anchor, UnityEngine::Vector2 pos, bool split, bool highest) {
        auto group = MakeGroup(anchor, pos);
        TextSource::Static label;
        label.Input = "Spinometer";
        AddText(group, TextSource::StaticName, label, 11).Position = UnityEngine::Vector2(0, 12.5);
        TextSource::Spinometer opts;
        opts.Mode = (int) (highest ? TextSource::Spinometer::Modes::Highest : TextSource::Spinometer::Modes::Average);
        if (split) {
            opts.Saber = (int) Sabers::Left;
            AddText(group, TextSource::SpinometerName, opts, 15, TextOptions::Aligns::Right).Position = UnityEngine::Vector2(-2, 0);
            opts.Saber = (int) Sabers::Right;
            AddText(group, TextSource::SpinometerName, opts, 15, TextOptions::Aligns::Left).Position = UnityEngine::Vector2(2, 0);
        } else
            AddText(group, TextSource::SpinometerName, opts, 15);
        Editor::AddGroup(group);
    }

    void EmptyUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        CreateButtons(parent, []() { AddEmpty(anchor, {}); });
    }
    void ScoreUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static bool score = false;
        static bool percent = true;
        static bool rank = true;
        static int decimals = 2;
        static bool rankColors = true;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        BeatSaberUI::CreateToggle(parent, "Show Numeric Score", score, [](bool val) { score = val; });
        BeatSaberUI::CreateToggle(parent, "Show Percentage", percent, [](bool val) { percent = val; });
        BeatSaberUI::CreateToggle(parent, "Show Rank", rank, [](bool val) { rank = val; });
        BeatSaberUI::CreateIncrementSetting(parent, "Percentage Decimals", 0, 1, decimals, [](float val) { decimals = val; });
        BeatSaberUI::CreateToggle(parent, "Color Rank Text", rankColors, [](bool val) { rankColors = val; });
        CreateButtons(parent, []() { AddScore(anchor, {}, score, percent, rank, decimals, rankColors); });
    }
    void PersonalBestUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static bool absolute = false;
        static bool hideFirst = true;
        static int decimals = 2;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        BeatSaberUI::CreateToggle(parent, "Absolute Score", absolute, [](bool val) { absolute = val; });
        BeatSaberUI::CreateToggle(parent, "Hide On First Score", hideFirst, [](bool val) { hideFirst = val; });
        BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; });
        CreateButtons(parent, []() { AddPersonalBest(anchor, {}, absolute, hideFirst, decimals); });
    }
    void AverageCutUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static bool splitSaber = true;
        static bool splitCut = false;
        static int decimals = 2;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        BeatSaberUI::CreateToggle(parent, "Split Sabers", splitSaber, [](bool val) { splitSaber = val; });
        BeatSaberUI::CreateToggle(parent, "Split Cut Parts", splitCut, [](bool val) { splitCut = val; });
        BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; });
        CreateButtons(parent, []() { AddAverageCut(anchor, {}, splitSaber, splitCut, decimals); });
    }
    void TimeDependenceUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static bool splitSaber = true;
        static int decimals = 2;
        static int decimalOffset = 0;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        BeatSaberUI::CreateToggle(parent, "Split Sabers", splitSaber, [](bool val) { splitSaber = val; });
        BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; });
        BeatSaberUI::CreateIncrementSetting(parent, "Decimal Offset", 0, 1, decimalOffset, [](float val) { decimalOffset = val; });
        CreateButtons(parent, []() { AddTimeDependence(anchor, {}, splitSaber, decimals, decimalOffset); });
    }
    void NotesUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static int display = 0;
        static int decimals = 2;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        Utils::CreateDropdownEnum(parent, "Display Type", display, NotesDisplayStrings, [](int val) { display = val; });
        BeatSaberUI::CreateIncrementSetting(parent, "Percentage Decimals", 0, 1, decimals, [](float val) { decimals = val; });
        CreateButtons(parent, []() { AddNotes(anchor, {}, display, decimals); });
    }
    void MistakesUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static bool badCuts = true;
        static bool bombs = false;
        static bool walls = false;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        BeatSaberUI::CreateToggle(parent, "Count Bad Cuts", badCuts, [](bool val) { badCuts = val; });
        BeatSaberUI::CreateToggle(parent, "Count Bombs", bombs, [](bool val) { bombs = val; });
        BeatSaberUI::CreateToggle(parent, "Count Walls", walls, [](bool val) { walls = val; });
        CreateButtons(parent, []() { AddMistakes(anchor, {}, badCuts, bombs, walls); });
    }
    void FailsUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static bool restarts = false;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        BeatSaberUI::CreateToggle(parent, "Count Restarts Instead", restarts, [](bool val) { restarts = val; });
        CreateButtons(parent, []() { AddFails(anchor, {}, restarts); });
    }
    void SongTimeUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static int display = 0;
        static bool timeLeft = false;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        Utils::CreateDropdownEnum(parent, "Display Type", display, SongTimeDisplayStrings, [](int val) { display = val; });
        BeatSaberUI::CreateToggle(parent, "Time Remaining", timeLeft, [](bool val) { timeLeft = val; });
        CreateButtons(parent, []() { AddSongTime(anchor, {}, display, timeLeft); });
    }
    void PPUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static bool beatleader = false;
        static bool scoresaber = false;
        static bool hideUnranked = false;
        static int decimals = 2;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        BeatSaberUI::CreateToggle(parent, "Show ScoreSaber", scoresaber, [](bool val) { scoresaber = val; });
        BeatSaberUI::CreateToggle(parent, "Show BeatLeader", beatleader, [](bool val) { beatleader = val; });
        BeatSaberUI::CreateToggle(parent, "Hide When Unranked", hideUnranked, [](bool val) { hideUnranked = val; });
        BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; });
        CreateButtons(parent, []() { AddPP(anchor, {}, beatleader, scoresaber, hideUnranked, decimals); });
    }
    void SaberSpeedUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static bool split = false;
        static bool last5Secs = false;
        static int decimals = 2;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        BeatSaberUI::CreateToggle(parent, "Split Sabers", split, [](bool val) { split = val; });
        BeatSaberUI::CreateToggle(parent, "Last 5 Seconds Only", last5Secs, [](bool val) { last5Secs = val; });
        BeatSaberUI::CreateIncrementSetting(parent, "Decimals", 0, 1, decimals, [](float val) { decimals = val; });
        CreateButtons(parent, []() { AddSaberSpeed(anchor, {}, split, last5Secs, decimals); });
    }
    void SpinometerUI(UnityEngine::GameObject* parent) {
        static int anchor = 0;
        static bool split = false;
        static bool highest = false;

        Utils::CreateDropdownEnum(parent, "Starting Anchor", anchor, AnchorStrings, [](int val) { anchor = val; });
        BeatSaberUI::CreateToggle(parent, "Split Sabers", split, [](bool val) { split = val; });
        BeatSaberUI::CreateToggle(parent, "Use Highest", highest, [](bool val) { highest = val; });
        CreateButtons(parent, []() { AddSpinometer(anchor, {}, split, highest); });
    }
}
