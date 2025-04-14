#include "config.hpp"

#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "copies.hpp"
#include "customtypes/components.hpp"
#include "customtypes/settings.hpp"
#include "editor.hpp"
#include "main.hpp"
#include "metacore/shared/ui.hpp"
#include "options.hpp"
#include "qounters.hpp"
#include "sources.hpp"
#include "sourceui.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace MetaCore;
using namespace UnityEngine;

#define MUI MetaCore::UI
#define UUI UnityEngine::UI

std::vector<std::string_view> const Options::SaberStrings = {
    "Left",
    "Right",
    "Both",
};
std::vector<std::string_view> const Options::DirectionStrings = {
    "Horizontal",
    "Vertical",
};
std::vector<std::string_view> const Options::AlignStrings = {
    "Left",
    "Right",
    "Center",
};
std::vector<std::string_view> const Options::ShapeStrings = {
    "Rectangle",
    "Rectangle Border",
    "Circle",
    "Circle Border",
};
std::vector<std::string_view> const Options::FillStrings = {
    "None",
    "Horizontal",
    "Vertical",
    "Circle",
};
std::vector<std::string_view> const Options::TypeStrings = {
    "Text",
    "Shape",
    "Image",
    "Premade",
};
std::vector<std::string_view> const Options::AnchorStrings = {
    "Left",
    "Right",
    "Top",
    "Bottom",
    "Center",
};
std::vector<std::string_view> const Options::BaseGameObjectStrings = {
    "Multiplier Ring", "Song Time Panel",
    // "Health Bar",
};

void Options::CreateTextUI(GameObject* parent, Text const& options) {
    static BSML::DropdownListSetting* sourceDropdown;
    static UUI::VerticalLayoutGroup* sourceOptions;
    static bool collapseOpen = true;

    auto align = MUI::CreateDropdownEnum(parent, "Align", options.Align, AlignStrings, [](int val) {
        static int id = Editor::GetActionId();
        auto opts = Editor::GetOptions<Text>(id);
        opts.Align = val;
        Editor::SetOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(align, "Choose the alignment for the text of this counter");

    auto inc = BSML::Lite::CreateIncrementSetting(parent, "Font Size", 1, 0.5, options.Size, true, false, 0, -1, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        auto opts = Editor::GetOptions<Text>(id);
        opts.Size = val;
        Editor::SetOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(inc, "Choose the font size for the text of this counter");

    auto italic = BSML::Lite::CreateToggle(parent, "Italic", options.Italic, [](bool val) {
        static int id = Editor::GetActionId();
        auto opts = Editor::GetOptions<Text>(id);
        opts.Italic = val;
        Editor::SetOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(italic, "Italicize the text of this counter");

    auto sourceCollapse = Utils::CreateCollapseArea(parent, "Text Source Options", collapseOpen, Copies::TextSource);

    sourceDropdown = MUI::CreateDropdown(parent, "Text Source", options.TextSource, Utils::GetKeys(Sources::texts), [parent](std::string val) {
        static int id = Editor::GetActionId();
        auto opts = Editor::GetOptions<Text>(id);
        if (val == opts.TextSource)
            return;
        sourceDropdown->dropdown->Hide(false);
        opts.TextSource = val;
        Editor::SetOptions(id, opts);
        Sources::Text::CreateUI(sourceOptions->gameObject, val, opts.SourceOptions);
        Utils::RebuildWithScrollPosition(parent->transform->parent->gameObject);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(sourceDropdown, "Change the driver for the text of this counter");

    sourceOptions = BSML::Lite::CreateVerticalLayoutGroup(parent);
    Sources::Text::CreateUI(sourceOptions->gameObject, options.TextSource, options.SourceOptions);

    sourceCollapse->AddContents({sourceDropdown->transform->parent, sourceOptions});
    sourceCollapse->onUpdate = [sourceCollapse]() {
        Qounters::OptionsViewController::UpdateScrollViewStatic();
        collapseOpen = sourceCollapse->open;
    };
}

void Options::CreateShapeUI(GameObject* parent, Shape const& options) {
    static BSML::IncrementSetting* borderIncrement;
    static BSML::DropdownListSetting* sourceDropdown;
    static UUI::VerticalLayoutGroup* sourceOptions;
    static bool collapseOpen = false;

    auto shape = MUI::CreateDropdownEnum(parent, "Shape", options.Shape, ShapeStrings, [](int val) {
        borderIncrement->gameObject->active = Shape::IsOutline(val);
        Qounters::OptionsViewController::UpdateScrollViewStatic();
        static int id = Editor::GetActionId();
        auto opts = Editor::GetOptions<Shape>(id);
        opts.Shape = val;
        Editor::SetOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(shape, "Change the shape of this counter");

    borderIncrement =
        BSML::Lite::CreateIncrementSetting(parent, "Border Width", 1, 0.1, options.OutlineWidth, true, false, 0.1, -1, {0, 0}, [](float val) {
            static int id = Editor::GetActionId();
            auto opts = Editor::GetOptions<Shape>(id);
            opts.OutlineWidth = val;
            Editor::SetOptions(id, opts);
            Editor::FinalizeAction();
        });
    BSML::Lite::AddHoverHint(borderIncrement, "Change the border width of the shape of this counter");

    auto fillCollapse = Utils::CreateCollapseArea(parent, "Fill Options", collapseOpen, Copies::Fill);

    auto directionDropdown = MUI::CreateDropdownEnum(parent, "Fill Direction", options.Fill, FillStrings, [](int val) {
        static int id = Editor::GetActionId();
        auto opts = Editor::GetOptions<Shape>(id);
        opts.Fill = val;
        Editor::SetOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(directionDropdown, "Change the way the shape of this counter gets partially filled");

    auto inverseToggle = BSML::Lite::CreateToggle(parent, "Inverse Fill", options.Inverse, [](bool val) {
        static int id = Editor::GetActionId();
        auto opts = Editor::GetOptions<Shape>(id);
        opts.Inverse = val;
        Editor::SetOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(inverseToggle, "Inverse the direction of the fill of this counter");

    sourceDropdown = MUI::CreateDropdown(parent, "Fill Source", options.FillSource, Utils::GetKeys(Sources::shapes), [parent](std::string val) {
        static int id = Editor::GetActionId();
        auto opts = Editor::GetOptions<Shape>(id);
        if (opts.FillSource != val) {
            sourceDropdown->dropdown->Hide(false);
            opts.FillSource = val;
            Editor::SetOptions(id, opts);
            Sources::Shape::CreateUI(sourceOptions->gameObject, val, opts.SourceOptions);
            Utils::RebuildWithScrollPosition(parent->transform->parent->gameObject);
        }
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(sourceDropdown, "Change the driver for the fill of this counter");

    sourceOptions = BSML::Lite::CreateVerticalLayoutGroup(parent);
    Sources::Shape::CreateUI(sourceOptions->gameObject, options.FillSource, options.SourceOptions);

    fillCollapse->AddContents({directionDropdown->transform->parent, inverseToggle, sourceDropdown->transform->parent, sourceOptions});
    fillCollapse->onUpdate = [fillCollapse]() {
        Qounters::OptionsViewController::UpdateScrollViewStatic();
        collapseOpen = fillCollapse->open;
    };
}

void Options::CreateImageUI(GameObject* parent, Image const& options) {
    static BSML::ModalView* modal;
    static HMUI::ImageView* currentImage;

    ImageSpriteCache::LoadAllSprites();
    auto currentSprite = ImageSpriteCache::GetSprite(options.Path);

    constexpr int width =
        SpritesListCell::CellSize * SpritesListCell::ImagesPerCell + SpritesListCell::ImageSpacing * (SpritesListCell::ImagesPerCell - 1);
    Vector2 listSize = {width, 60};
    modal = BSML::Lite::CreateModal(parent, Vector2::op_Addition(listSize, {5, 0}), nullptr);
    modal->moveToCenter = true;

    auto list = BSML::Lite::CreateScrollableCustomSourceList<SpritesListSource*>(modal);
    list->imageClickedCallback = [](int idx) {
        modal->Hide();
        currentImage->sprite = ImageSpriteCache::GetSpriteIdx(idx);
        static int actionId = Editor::GetActionId();
        auto opts = Editor::GetOptions<Image>(actionId);
        opts.Path = ImageSpriteCache::GetInstance()->spritePaths[idx];
        Editor::SetOptions(actionId, opts);
        Editor::FinalizeAction();
    };
    list->tableView->ReloadData();

    auto rect = list->transform->parent.cast<RectTransform>();
    rect->anchorMin = {0.5, 0.5};
    rect->anchorMax = {0.5, 0.5};
    rect->sizeDelta = listSize;
    rect->GetComponent<UUI::ContentSizeFitter*>()->horizontalFit = UUI::ContentSizeFitter::FitMode::Unconstrained;

    auto horizontal = BSML::Lite::CreateHorizontalLayoutGroup(parent);
    horizontal->spacing = 5;

    currentImage = BSML::Lite::CreateImage(horizontal, currentSprite);
    currentImage->preserveAspect = true;
    MUI::SetLayoutSize(currentImage, -1, 10);

    auto imageButton = BSML::Lite::CreateUIButton(horizontal, "Select Image", []() { modal->Show(); });
    BSML::Lite::AddHoverHint(imageButton, "Select the image for this counter");
}

void Options::CreatePremadeUI(GameObject* parent, Premade const& options) {
    std::vector<std::string_view> names = {};
    for (auto& [_, infos] : Sources::premades) {
        for (auto& info : infos)
            names.emplace_back(info.name);
    }

    auto objectDropdown = MUI::CreateDropdown(parent, "Object", options.Name, names, [](std::string val) {
        static int id = Editor::GetActionId();
        auto opts = Editor::GetOptions<Premade>(id);
        opts.Name = val;
        Editor::SetOptions(id, opts);
        Editor::FinalizeAction();
    });
    BSML::Lite::AddHoverHint(objectDropdown, "Select the premade object for this counter");

    // TODO: UI for missing mod?
    auto info = Sources::GetPremadeInfo(options.SourceMod, options.Name);
    if (info && info->uiFunction)
        info->uiFunction(parent, options.Options);
}

void Options::CreateTypeUI(Transform* parent, int type, Component::OptionsTypes const& options) {
    while (parent->GetChildCount() > 0)
        Object::DestroyImmediate(parent->GetChild(0)->gameObject);

    auto parentGO = parent->gameObject;

    switch ((Component::Types) type) {
        case Component::Types::Text:
            CreateTextUI(parentGO, options.GetValue<Text>().value_or(Text{}));
            break;
        case Component::Types::Shape:
            CreateShapeUI(parentGO, options.GetValue<Shape>().value_or(Shape{}));
            break;
        case Component::Types::Image:
            CreateImageUI(parentGO, options.GetValue<Image>().value_or(Image{}));
            break;
        case Component::Types::Premade:
            CreatePremadeUI(parentGO, options.GetValue<Premade>().value_or(Premade{}));
            break;
    }

    MUI::SetChildrenWidth(parent, parent->GetComponent<UUI::LayoutElement*>()->preferredWidth);
}

static Options::Group GetScoreGroup() {
    Options::Group ret;
    ret.Anchor = (int) Options::Group::Anchors::Left;
    ret.Position = Vector2(0, 25 - 37.5);
    Options::Component component;
    component.Type = (int) Options::Component::Types::Text;
    Sources::Color::Static colorOptions;
    Options::Text textOptions;
    Sources::Text::Score scoreOptions;
    // rank text
    component.Position = Vector2(0, -20);
    textOptions.TextSource = Sources::Text::RankName;
    textOptions.Size = 33;
    component.Options = textOptions;
    colorOptions.Input = Color(1, 1, 1, 0.502);
    component.ColorOptions = colorOptions;
    ret.Components.push_back(component);
    // percent text
    component.Position = Vector2(0, -2);
    textOptions.TextSource = Sources::Text::ScoreName;
    textOptions.SourceOptions = scoreOptions;
    textOptions.Size = 12;
    component.Options = textOptions;
    ret.Components.push_back(component);
    // score text
    component.Position = Vector2(0, 12.5);
    textOptions.TextSource = Sources::Text::ScoreName;
    scoreOptions.Percentage = false;
    textOptions.SourceOptions = scoreOptions;
    textOptions.Size = 15;
    component.Options = textOptions;
    colorOptions.Input = Color(1, 1, 1, 1);
    component.ColorOptions = colorOptions;
    ret.Components.push_back(component);
    return ret;
}

static Options::Group GetComboGroup() {
    Options::Group ret;
    ret.Anchor = (int) Options::Group::Anchors::Left;
    ret.Position = Vector2(0, 71.5 - 37.5);
    Options::Component component;
    component.Type = (int) Options::Component::Types::Text;
    Options::Text textOptions;
    Sources::Text::Static staticOptions;
    Options::Shape shapeOptions;
    shapeOptions.Shape = (int) Options::Shape::Shapes::Square;
    // combo text
    component.Position = Vector2(0, -7.5);
    textOptions.TextSource = Sources::Text::ComboName;
    textOptions.SourceOptions = Sources::Text::Combo();
    textOptions.Size = 20;
    component.Options = textOptions;
    ret.Components.push_back(component);
    // "combo" text
    component.Position = Vector2(0, 7.5);
    component.Scale = Vector2{0.7, 1};
    textOptions.TextSource = Sources::Text::StaticName;
    staticOptions.Input = "COMBO";
    textOptions.SourceOptions = staticOptions;
    textOptions.Size = 15;
    component.Options = textOptions;
    ret.Components.push_back(component);
    // lower line
    component.Type = (int) Options::Component::Types::Shape;
    component.Position = Vector2(0, -19);
    component.Scale = Vector2{2, 0.12};
    component.Options = shapeOptions;
    component.EnableSource = Sources::Enable::FullComboName;
    component.EnableOptions = Sources::Enable::FullCombo();
    ret.Components.push_back(component);
    // upper line
    component.Position = Vector2(0, 19);
    ret.Components.push_back(component);
    return ret;
}

static Options::Group GetMultiplierGroup() {
    Options::Group ret;
    ret.Anchor = (int) Options::Group::Anchors::Right;
    ret.Position = Vector2(0, 65 - 37.5);
    Options::Component component;
    component.Type = (int) Options::Component::Types::Premade;
    Options::Premade premadeOptions;
    premadeOptions.Name = Options::BaseGameObjectStrings[(int) BaseGameGraphic::Objects::Multiplier];
    component.Options = premadeOptions;
    ret.Components.push_back(component);
    return ret;
}

static Options::Group GetProgressGroup() {
    // I'm a little too lazy to split this up into the bar and then three text components
    Options::Group ret;
    ret.Anchor = (int) Options::Group::Anchors::Right;
    ret.Position = Vector2(0, 20 - 37.5);
    Options::Component component;
    component.Type = (int) Options::Component::Types::Premade;
    Options::Premade premadeOptions;
    premadeOptions.Name = Options::BaseGameObjectStrings[(int) BaseGameGraphic::Objects::ProgressBar];
    component.Options = premadeOptions;
    ret.Components.push_back(component);
    return ret;
}

static Options::Group GetHealthBarGroup() {
    Options::Group ret;
    ret.Anchor = (int) Options::Group::Anchors::Bottom;
    ret.Position = Vector2(0, 10);
    Options::Component component;
    component.Type = (int) Options::Component::Types::Premade;
    Options::Premade premadeOptions;
    premadeOptions.Name = Options::BaseGameObjectStrings[(int) BaseGameGraphic::Objects::HealthBar];
    component.Options = premadeOptions;
    ret.Components.push_back(component);
    return ret;
}

Options::Preset Options::GetDefaultHUDPreset() {
    Preset ret;
    ret.Qounters.push_back(GetScoreGroup());
    ret.Qounters.push_back(GetComboGroup());
    ret.Qounters.push_back(GetMultiplierGroup());
    ret.Qounters.push_back(GetProgressGroup());
    // crashes on enable, idk why
    // ret.Qounters.push_back(GetHealthBarGroup());
    return ret;
}
