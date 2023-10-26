#include "config.hpp"
#include "customtypes/components.hpp"
#include "customtypes/settings.hpp"
#include "editor.hpp"
#include "main.hpp"
#include "sources.hpp"
#include "sourceui.hpp"
#include "utils.hpp"

std::vector<std::string> Qounters::TypeStrings = {
    "Text",
    "Shape",
    "Image",
    "Base Game",
};
std::vector<std::string> Qounters::AnchorStrings = {
    "Left",
    "Right",
    "Top",
    "Bottom",
};
std::vector<std::string> Qounters::AlignStrings = {
    "Left",
    "Right",
    "Center",
};
std::vector<std::string> Qounters::ShapeStrings = {
    "Rectangle",
    "Rectangle Border",
    "Circle",
    "Circle Border",
    "Triangle",
    "Triangle Border",
};
std::vector<std::string> Qounters::FillStrings = {
    "None",
    "Horizontal",
    "Vertical",
    "Circle",
};
std::vector<std::string> Qounters::ComponentStrings = {
    "Multiplier",
    "Song Time",
    // "Health Bar",
};

#include "questui/shared/BeatSaberUI.hpp"

using namespace QuestUI;
using namespace UnityEngine;

namespace Qounters {
    void CreateTextOptionsUI(GameObject* parent, TextOptions const& options) {
        static TextOptions opts;
        static HMUI::SimpleTextDropdown* sourceDropdown;
        static UI::VerticalLayoutGroup* sourceOptions;

        opts = options;

        Utils::CreateDropdownEnum(parent, "Align", options.Align, AlignStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Align = val;
            Editor::SetOptions(id, opts);
            Editor::FinalizeAction();
        });

        auto inc = BeatSaberUI::CreateIncrementSetting(parent, "Font Size", 1, 0.5, options.Size, true, false, 0, -1, Vector2(), [](float val) {
            static int id = Editor::GetActionId();
            opts.Size = val;
            Editor::SetOptions(id, opts);
            Editor::FinalizeAction();
        });
        Utils::AddIncrementIncrement(inc, 5);
        SetButtons(inc);

        BeatSaberUI::CreateToggle(parent, "Italic", options.Italic, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Italic = val;
            Editor::SetOptions(id, opts);
            Editor::FinalizeAction();
        });

        sourceDropdown = Utils::CreateDropdown(parent, "Text Source", options.TextSource, Utils::GetKeys(textSources), [parent](std::string val) {
            static int id = Editor::GetActionId();
            if (val == opts.TextSource)
                return;
            sourceDropdown->Hide(false);
            opts.TextSource = val;
            Editor::SetOptions(id, opts);
            TextSource::CreateUI(sourceOptions->get_gameObject(), val, opts.SourceOptions);
            Utils::RebuildWithScrollPosition(parent->get_transform()->GetParent()->get_gameObject());
            Editor::FinalizeAction();
        });

        sourceOptions = BeatSaberUI::CreateVerticalLayoutGroup(parent);
        TextSource::CreateUI(sourceOptions->get_gameObject(), opts.TextSource, opts.SourceOptions);
    }

    void CreateShapeOptionsUI(GameObject* parent, ShapeOptions const& options) {
        static ShapeOptions opts;
        static HMUI::SimpleTextDropdown* sourceDropdown;
        static UI::VerticalLayoutGroup* sourceOptions;

        opts = options;

        Utils::CreateDropdownEnum(parent, "Shape", options.Shape, ShapeStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Shape = val;
            Editor::SetOptions(id, opts);
            Editor::FinalizeAction();
        });

        auto inc = BeatSaberUI::CreateIncrementSetting(parent, "Outline Width", 1, 0.1, options.OutlineWidth, true, false, 0.1, -1, Vector2(), [](float val) {
            static int id = Editor::GetActionId();
            opts.OutlineWidth = val;
            Editor::SetOptions(id, opts);
            Editor::FinalizeAction();
        });
        SetButtons(inc);

        Utils::CreateDropdownEnum(parent, "Fill", options.Fill, FillStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Fill = val;
            Editor::SetOptions(id, opts);
            Editor::FinalizeAction();
        });

        sourceDropdown = Utils::CreateDropdown(parent, "Fill Source", options.FillSource, Utils::GetKeys(shapeSources), [parent](std::string val) {
            static int id = Editor::GetActionId();
            if (opts.FillSource != val) {
                sourceDropdown->Hide(false);
                opts.FillSource = val;
                Editor::SetOptions(id, opts);
                ShapeSource::CreateUI(sourceOptions->get_gameObject(), val, opts.SourceOptions);
                Utils::RebuildWithScrollPosition(parent->get_transform()->GetParent()->get_gameObject());
            }
            Editor::FinalizeAction();
        });

        sourceOptions = BeatSaberUI::CreateVerticalLayoutGroup(parent);
        ShapeSource::CreateUI(sourceOptions->get_gameObject(), opts.FillSource, opts.SourceOptions);

        BeatSaberUI::CreateToggle(parent, "Inverse Fill", options.Inverse, [](bool val) {
            static int id = Editor::GetActionId();
            opts.Inverse = val;
            Editor::SetOptions(id, opts);
            Editor::FinalizeAction();
        });
    }

    void CreateImageOptionsUI(GameObject* parent, ImageOptions const& options) {
        static ImageOptions opts;
        static HMUI::ModalView* modal;
        static HMUI::ImageView* currentImage;
        opts = options;

        ImageSpriteCache::LoadAllSprites();
        auto currentSprite = ImageSpriteCache::GetSprite(options.Path);

        constexpr int width = SpritesListCell::cellSize * SpritesListCell::imagesPerCell + SpritesListCell::imageSpacing * (SpritesListCell::imagesPerCell - 1);
        Vector2 listSize = {width, 60};
        modal = BeatSaberUI::CreateModal(parent, listSize + Vector2(5, 0), nullptr);

        auto list = BeatSaberUI::CreateScrollableCustomSourceList<SpritesListSource*>(modal);
        list->imageClickedCallback = [](int idx) {
            modal->Hide(true, nullptr);
            currentImage->set_sprite(ImageSpriteCache::GetSpriteIdx(idx));
            static int actionId = Editor::GetActionId();
            opts.Path = ImageSpriteCache::GetInstance()->spritePaths[idx];
            Editor::SetOptions(actionId, opts);
            Editor::FinalizeAction();
        };
        list->tableView->ReloadData();

        auto rect = (RectTransform*) list->get_transform()->GetParent();
        rect->set_anchorMin({0.5, 0.5});
        rect->set_anchorMax({0.5, 0.5});
        rect->set_sizeDelta(listSize);
        rect->GetComponent<UI::ContentSizeFitter*>()->set_horizontalFit(UI::ContentSizeFitter::FitMode::Unconstrained);

        auto horizontal = BeatSaberUI::CreateHorizontalLayoutGroup(parent);
        horizontal->set_spacing(5);

        currentImage = BeatSaberUI::CreateImage(horizontal, currentSprite);
        currentImage->set_preserveAspect(true);

        BeatSaberUI::CreateUIButton(horizontal, "Select Image", []() {
            modal->Show(true, true, nullptr);
        });
    }

    void CreateBaseGameOptionsUI(GameObject* parent, BaseGameOptions const& options) {
        static BaseGameOptions opts;
        opts = options;

        Utils::CreateDropdownEnum(parent, "Component", options.Component, ComponentStrings, [](int val) {
            static int id = Editor::GetActionId();
            opts.Component = val;
            Editor::SetOptions(id, opts);
            Editor::FinalizeAction();
        });
    }

    void CreateTypeOptionsUI(Transform* parent, int type, Component::OptionsTypes const& options) {
        while (parent->GetChildCount() > 0)
            Object::DestroyImmediate(parent->GetChild(0)->get_gameObject());

        auto parentGO = parent->get_gameObject();

        switch ((Component::Types) type) {
            case Component::Types::Text:
                CreateTextOptionsUI(parentGO, options.GetValue<TextOptions>().value_or(TextOptions{}));
                break;
            case Component::Types::Shape:
                CreateShapeOptionsUI(parentGO, options.GetValue<ShapeOptions>().value_or(ShapeOptions{}));
                break;
            case Component::Types::Image:
                CreateImageOptionsUI(parentGO, options.GetValue<ImageOptions>().value_or(ImageOptions{}));
                break;
            case Component::Types::BaseGame:
                CreateBaseGameOptionsUI(parentGO, options.GetValue<BaseGameOptions>().value_or(BaseGameOptions{}));
                break;
        }

        Utils::SetChildrenWidth(parent, parent->GetComponent<UI::LayoutElement*>()->get_preferredWidth());
    }

    using Color = Color;
    using Vector2 = Vector2;

    Group GetScoreGroup() {
        Group ret;
        ret.Anchor = (int) Group::Anchors::Left;
        ret.Position = Vector2(0, 25 - 37.5);
        Component component;
        component.Type = (int) Component::Types::Text;
        ColorSource::Static colorOptions;
        TextOptions textOptions;
        TextSource::Score scoreOptions;
        // rank text
        component.Position = Vector2(0, -20);
        textOptions.TextSource = "Rank";
        textOptions.Size = 33;
        component.Options = textOptions;
        colorOptions.Input = Color(1, 1, 1, 0.502);
        component.ColorOptions = colorOptions;
        ret.Components.push_back(component);
        // percent text
        component.Position = Vector2(0, -2);
        textOptions.TextSource = "Score";
        textOptions.SourceOptions = scoreOptions;
        textOptions.Size = 12;
        component.Options = textOptions;
        ret.Components.push_back(component);
        // score text
        component.Position = Vector2(0, 12.5);
        textOptions.TextSource = "Score";
        scoreOptions.Percentage = false;
        textOptions.SourceOptions = scoreOptions;
        textOptions.Size = 15;
        component.Options = textOptions;
        colorOptions.Input = Color(1, 1, 1, 1);
        component.ColorOptions = colorOptions;
        ret.Components.push_back(component);
        return ret;
    }

    Group GetComboGroup() {
        Group ret;
        ret.Anchor = (int) Group::Anchors::Left;
        ret.Position = Vector2(0, 71.5 - 37.5);
        Component component;
        component.Type = (int) Component::Types::Text;
        TextOptions textOptions;
        TextSource::Static staticOptions;
        ShapeOptions shapeOptions;
        shapeOptions.Shape = (int) ShapeOptions::Shapes::Square;
        // combo text
        component.Position = Vector2(0, -7.5);
        textOptions.TextSource = "Combo";
        textOptions.SourceOptions = TextSource::Combo();
        textOptions.Size = 20;
        component.Options = textOptions;
        ret.Components.push_back(component);
        // "combo" text
        component.Position = Vector2(0, 7.5);
        component.Scale = Vector2{0.7, 1};
        textOptions.TextSource = "Static";
        staticOptions.Input = "COMBO";
        textOptions.SourceOptions = staticOptions;
        textOptions.Size = 15;
        component.Options = textOptions;
        ret.Components.push_back(component);
        // lower line
        component.Type = (int) Component::Types::Shape;
        component.Position = Vector2(0, -19);
        component.Scale = Vector2{2, 0.12};
        component.Options = shapeOptions;
        ret.Components.push_back(component);
        // upper line
        component.Position = Vector2(0, 19);
        ret.Components.push_back(component);
        return ret;
    }

    Group GetMultiplierGroup() {
        Group ret;
        ret.Anchor = (int) Group::Anchors::Right;
        ret.Position = Vector2(0, 65 - 37.5);
        Component component;
        component.Type = (int) Component::Types::BaseGame;
        BaseGameOptions baseOptions;
        baseOptions.Component = (int) BaseGameOptions::Components::Multiplier;
        component.Options = baseOptions;
        ret.Components.push_back(component);
        return ret;
    }

    Group GetProgressGroup() {
        // I'm a little too lazy to split this up into the bar and then three text components
        Group ret;
        ret.Anchor = (int) Group::Anchors::Right;
        ret.Position = Vector2(0, 20 - 37.5);
        Component component;
        component.Type = (int) Component::Types::BaseGame;
        BaseGameOptions baseOptions;
        baseOptions.Component = (int) BaseGameOptions::Components::ProgressBar;
        component.Options = baseOptions;
        ret.Components.push_back(component);
        return ret;
    }

    Group GetHealthBarGroup() {
        Group ret;
        ret.Anchor = (int) Group::Anchors::Bottom;
        ret.Position = Vector2(0, 10);
        Component component;
        component.Type = (int) Component::Types::BaseGame;
        BaseGameOptions baseOptions;
        baseOptions.Component = (int) BaseGameOptions::Components::HealthBar;
        component.Options = baseOptions;
        ret.Components.push_back(component);
        return ret;
    }

    Preset GetDefaultHUDPreset() {
        Preset ret;
        ret.Qounters.push_back(GetScoreGroup());
        ret.Qounters.push_back(GetComboGroup());
        ret.Qounters.push_back(GetMultiplierGroup());
        ret.Qounters.push_back(GetProgressGroup());
        // crashes on enable, idk why
        // ret.Qounters.push_back(GetHealthBarGroup());
        return ret;
    }
}
