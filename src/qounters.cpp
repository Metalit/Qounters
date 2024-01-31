#include "qounters.hpp"
#include "config.hpp"
#include "customtypes/editing.hpp"
#include "customtypes/components.hpp"
#include "editor.hpp"
#include "environment.hpp"
#include "main.hpp"
#include "sources.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace UnityEngine;

#include "TMPro/TextMeshProUGUI.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/UI/Graphic.hpp"

std::map<std::string, std::vector<std::pair<TMPro::TextMeshProUGUI*, UnparsedJSON>>> texts;
std::map<std::string, std::vector<std::pair<HMUI::ImageView*, UnparsedJSON>>> shapes;
std::map<std::string, std::vector<std::pair<UI::Graphic*, UnparsedJSON>>> colors;
std::map<std::string, std::vector<std::pair<GameObject*, std::pair<UnparsedJSON, bool>>>> enables;

template<class TComp, class TOpts>
void UpdatePair(std::map<std::string, std::vector<std::pair<TComp, TOpts>>>& map, TComp update, std::string source, TOpts value, bool forceAdd) {
    if (!map.contains(source))
        map[source] = {};

    bool add = true;
    if (!forceAdd) {
        for (auto& [mapSource, vec] : map) {
            for (auto it = vec.begin(); it != vec.end(); it++) {
                if (it->first == update) {
                    add = mapSource != source;
                    if (add)
                        vec.erase(it);
                    else
                        it->second = value;
                    break;
                }
            }
        }
    }
    if (add)
        map[source].emplace_back(update, value);
}

template<class T>
void RemoveFromMap(std::map<std::string, std::vector<std::pair<T, UnparsedJSON>>>& map, void* remove) {
    auto cast = (T) remove;

    for (auto& [mapSource, vec] : map) {
        for (auto it = vec.begin(); it != vec.end(); it++) {
            if (it->first == cast) {
                vec.erase(it);
                break;
            }
        }
    }
}

Sprite* GetShapeSprite(int shape) { // TODO
    return QuestUI::BeatSaberUI::Base64ToSprite("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAIAAACQd1PeAAAACXBIWXMAAAABAAAAAQBPJcTWAAAADElEQVR4nGP4//8/AAX+Av4N70a4AAAAAElFTkSuQmCC");
    switch ((ShapeOptions::Shapes) shape) {
        case ShapeOptions::Shapes::Square:
            return nullptr;
        case ShapeOptions::Shapes::SquareOutline:
            return nullptr;
        case ShapeOptions::Shapes::Circle:
            return nullptr;
        case ShapeOptions::Shapes::CircleOutline:
            return nullptr;
        case ShapeOptions::Shapes::Triangle:
            return nullptr;
        case ShapeOptions::Shapes::TriangleOutline:
            return nullptr;
    }
}

void UpdateTextOptions(TMPro::TextMeshProUGUI* text, Qounters::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<TextOptions>().value_or(TextOptions{});

    text->set_fontStyle(options.Italic ? TMPro::FontStyles::Italic : TMPro::FontStyles::Normal);
    text->set_fontSize(options.Size);
    switch ((TextOptions::Aligns) options.Align) {
        case TextOptions::Aligns::Left:
            text->set_alignment(TMPro::TextAlignmentOptions::Left);
            break;
        case TextOptions::Aligns::Center:
            text->set_alignment(TMPro::TextAlignmentOptions::Center);
            break;
        case TextOptions::Aligns::Right:
            text->set_alignment(TMPro::TextAlignmentOptions::Right);
            break;
    }

    std::string source = options.TextSource;
    auto sourceFn = GetSource(textSources, source).first;
    if (!sourceFn)
        return;
    std::string string = sourceFn(options.SourceOptions);
    text->set_text(string);

    UpdatePair(texts, text, source, options.SourceOptions, creation);

    if (auto outline = text->GetComponent<TextOutlineSizer*>())
        outline->SetDirty();
}

#include "UnityEngine/UI/Image_OriginHorizontal.hpp"
#include "UnityEngine/UI/Image_OriginVertical.hpp"
#include "UnityEngine/UI/Image_Origin360.hpp"

void UpdateShapeOptions(HMUI::ImageView* shape, Qounters::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<ShapeOptions>().value_or(ShapeOptions{});

    shape->set_sprite(GetShapeSprite(options.Shape));

    auto fill = options.Fill;
    switch ((ShapeOptions::Fills) fill) {
        case ShapeOptions::Fills::None:
            // could remove it from the source shapes map as well, but it's probably insignificant
            shape->set_type(UI::Image::Type::Simple);
            return;
        case ShapeOptions::Fills::Horizontal:
            shape->set_fillMethod(UI::Image::FillMethod::Horizontal);
            shape->set_fillOrigin(options.Inverse
                ? UI::Image::OriginHorizontal::Right
                : UI::Image::OriginHorizontal::Left);
            break;
        case ShapeOptions::Fills::Vertical:
            shape->set_fillMethod(UI::Image::FillMethod::Vertical);
            shape->set_fillOrigin(options.Inverse
                ? UI::Image::OriginVertical::Bottom
                : UI::Image::OriginVertical::Top);
            break;
        case ShapeOptions::Fills::Circle:
            shape->set_fillMethod(UI::Image::FillMethod::Radial360);
            shape->set_fillOrigin(UI::Image::Origin360::Top);
            shape->set_fillClockwise(!options.Inverse);
            break;
    }
    shape->set_type(UI::Image::Type::Filled);

    std::string source = options.FillSource;
    auto sourceFn = GetSource(shapeSources, source).first;
    if (!sourceFn)
        return;
    float fillLevel = sourceFn(options.SourceOptions);
    shape->set_fillAmount(fillLevel);

    UpdatePair(shapes, shape, source, options.SourceOptions, creation);
}

#include "questui/shared/BeatSaberUI.hpp"

using namespace QuestUI;

void UpdateImageOptions(HMUI::ImageView* image, Qounters::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<ImageOptions>().value_or(ImageOptions{});

    Sprite* sprite = nullptr;
    if (fileexists(IMAGE_DIRECTORY + options.Path))
        sprite = ImageSpriteCache::GetSprite(options.Path);
    image->set_sprite(sprite);
}

void UpdateBaseGameOptions(BaseGameGraphic* base, Qounters::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<BaseGameOptions>().value_or(BaseGameOptions{});

    base->SetComponent(options.Component);
}

void Qounters::UpdateComponentOptions(int componentType, UnityEngine::Component* component, Qounters::Component::OptionsTypes newOptions) {
    switch ((Component::Types) componentType) {
        case Component::Types::Text:
            UpdateTextOptions((TMPro::TextMeshProUGUI*) component, newOptions, false);
            break;
        case Component::Types::Shape:
            UpdateShapeOptions((HMUI::ImageView*) component, newOptions, false);
            break;
        case Component::Types::Image:
            UpdateImageOptions((HMUI::ImageView*) component, newOptions, false);
            break;
        case Component::Types::BaseGame:
            UpdateBaseGameOptions((BaseGameGraphic*) component, newOptions, false);
            break;
    }
}

void UpdateColorOptions(UI::Graphic* component, std::string colorSource, UnparsedJSON options, bool creation) {
    auto sourceFn = GetSource(colorSources, colorSource).first;
    if (!sourceFn)
        return;
    auto color = sourceFn(options);
    component->set_color(color);

    UpdatePair(colors, component, colorSource, options, creation);
}

void Qounters::UpdateComponentColor(UI::Graphic* component, std::string newSource, UnparsedJSON newOptions) {
    UpdateColorOptions(component, newSource, newOptions, false);
}

void UpdateEnableOptions(GameObject* component, std::string enableSource, UnparsedJSON options, bool creation, bool invert) {
    auto sourceFn = GetSource(enableSources, enableSource).first;
    if (!sourceFn)
        return;
    bool enable = sourceFn(options);
    if (invert)
        enable = !enable;
    if (InSettingsEnvironment() && !Editor::GetPreviewMode())
        enable = true;
    component->SetActive(enable);

    UpdatePair(enables, component, enableSource, {options, invert}, creation);
}

void Qounters::UpdateComponentEnabled(GameObject* component, std::string newSource, UnparsedJSON newOptions, bool invert) {
    UpdateEnableOptions(component, newSource, newOptions, false, invert);
}

void Qounters::UpdateComponentPosition(RectTransform* component, Component const& qounterComponent) {
    switch ((Component::Types) qounterComponent.Type) {
        case Component::Types::Text:
            component->set_sizeDelta({0, 0});
            break;
        case Component::Types::Shape:
        case Component::Types::Image:
            component->set_sizeDelta({25, 25});
            break;
        case Component::Types::BaseGame:
            break;
    }
    component->set_anchorMin({0.5, 0.5});
    component->set_anchorMax({0.5, 0.5});
    component->set_anchoredPosition(qounterComponent.Position);
    component->set_localEulerAngles({0, 0, qounterComponent.Rotation});
    component->set_localScale({qounterComponent.Scale.x, qounterComponent.Scale.y, 0});
}

void Qounters::UpdateGroupPosition(RectTransform* group, Group const& qounterGroup) {
    auto anchor = GetAnchor(qounterGroup.Anchor);
    if (!anchor)
        return;
    group->SetParent(anchor, false);

    group->set_anchorMin({0.5, 0.5});
    group->set_anchorMax({0.5, 0.5});
    group->set_sizeDelta({0, 0});
    group->set_anchoredPosition(qounterGroup.Position);
    group->set_localEulerAngles({0, 0, qounterGroup.Rotation});
}

void Qounters::RemoveComponent(int componentType, UnityEngine::Component* component) {
    switch ((Component::Types) componentType) {
        case Component::Types::Text:
            RemoveFromMap(texts, component);
            break;
        case Component::Types::Shape:
            RemoveFromMap(shapes, component);
            break;
        case Component::Types::Image:
        case Component::Types::BaseGame:
            break;
    }
    RemoveFromMap(colors, component);
}

template<class T>
inline void SetSourceOptions(Qounters::Component::OptionsTypes& options, UnparsedJSON newOptions) {
    auto opts = options.GetValue<T>().value_or(T());
    opts.SourceOptions = newOptions;
    options.SetValue(opts);
}

void Qounters::SetSourceOptions(Component& component, UnparsedJSON newOptions) {
    switch ((Component::Types) component.Type) {
        case Component::Types::Text:
            SetSourceOptions<TextOptions>(component.Options, newOptions);
            break;
        case Component::Types::Shape:
            SetSourceOptions<ShapeOptions>(component.Options, newOptions);
            component.Options = ShapeOptions();
            break;
        case Component::Types::Image:
        case Component::Types::BaseGame:
            break;
    }
}

void Qounters::SetDefaultOptions(Component& component) {
    switch ((Component::Types) component.Type) {
        case Component::Types::Text:
            component.Options = TextOptions();
            break;
        case Component::Types::Shape:
            component.Options = ShapeOptions();
            break;
        case Component::Types::Image:
            component.Options = ImageOptions();
            break;
        case Component::Types::BaseGame:
            component.Options = BaseGameOptions();
            break;
    }
}

const std::map<std::string, HUDType> supportedHUDs = {
    {"BasicGameHUD", HUDType::Basic},
    {"NarrowGameHUD", HUDType::Basic},
    {"FlyingGameHUD/Container", HUDType::Rotational},
    {"MultiplayerLocalActivePlayerController(Clone)/IsActiveObjects/HUD", HUDType::Multiplayer},
    {"MultiplayerDuelLocalActivePlayerController(Clone)/IsActiveObjects/HUD", HUDType::Multiplayer},
};

const std::map<HUDType, std::map<Group::Anchors, std::tuple<std::string, Vector3, Vector2, Vector2>>> hudPanels = {
    {HUDType::Basic, {
        {Group::Anchors::Left, {"LeftPanel", {-3, 0.4, 7}, {50, 125}, {0, 0.75}}},
        {Group::Anchors::Right, {"RightPanel", {3, 0.4, 7}, {50, 125}, {0, 0.75}}},
        {Group::Anchors::Bottom, {"EnergyPanel", {0, -0.64, 7}, {-25, 25}, {0, -10}}},
        {Group::Anchors::Top, {"QountersTopPanel", {0, 3, 7}, {125, 50}, {}}},
    }},
    {HUDType::Rotational, {
        {Group::Anchors::Left, {"ComboPanel", {-80, 0, 0}, {10, 10}, {}}},
        {Group::Anchors::Right, {"MultiplierCanvas", {80, 0, 0}, {10, 10}, {}}},
        {Group::Anchors::Bottom, {"EnergyPanel", {0, -38, 0}, {10, 10}, {}}},
        {Group::Anchors::Top, {"SongProgressCanvas", {0, 30, 0}, {10, 10}, {}}},
    }},
    {HUDType::Multiplayer, {
        {Group::Anchors::Left, {"QountersLeftPanel", {-3, 0.4, 7}, {50, 125}, {0, 0.75}}},
        {Group::Anchors::Right, {"QountersRightPanel", {3, 0.4, 7}, {50, 125}, {0, 0.75}}},
        {Group::Anchors::Bottom, {"EnergyPanel", {0, -38, 0}, {10, 10}, {}}},
        {Group::Anchors::Top, {"QountersTopPanel", {0, 3, 7}, {125, 50}, {}}},
    }},
};

Transform* GetCanvas(std::string parentName, Transform* hud, Vector3 fallback) {
    static ConstString name("QountersCanvas");

    auto parent = Utils::FindRecursive(hud, parentName);
    if (!parent && !parentName.starts_with("Qounters")) {
        getLogger().info("Failed to find parent %s!", parentName.c_str());
        parentName = "Qounters" + parentName;
        parent = Utils::FindRecursive(hud, parentName);
    }
    if (!parent) {
        getLogger().info("Creating replacement parent object");
        parent = GameObject::New_ctor(parentName)->get_transform();
        parent->set_position(fallback);
    }

    if (auto ret = parent->Find(name))
        return ret;
    parent->SetParent(hud, false);

    auto canvas = BeatSaberUI::CreateCanvas();
    canvas->set_name(name);

    canvas->GetComponent<Canvas*>()->set_sortingOrder(0);

    auto ret = canvas->get_transform();
    ret->set_localScale({0.02, 0.02, 0.02});
    ret->SetParent(parent, true);
    ret->set_localPosition({});
    ret->set_localEulerAngles({});

    return ret;
}

std::pair<Transform*, HUDType> Qounters::GetHUD() {
    for (auto& [name, type] : supportedHUDs) {
        if (auto hud = GameObject::Find(name))
            return {hud->get_transform(), type};
    }
    getLogger().error("Unable to find HUD object");
    return {nullptr, HUDType::Unsupported};
}

Transform* Qounters::GetAnchor(int anchor) {
    auto [hud, type] = GetHUD();
    if (type == HUDType::Unsupported)
        return nullptr;

    auto [name, fallback, size, pos] = hudPanels.at(type).at((Group::Anchors) anchor);
    auto ret = (RectTransform*) GetCanvas(name, hud, fallback);
    ret->set_sizeDelta(size);
    ret->set_anchoredPosition(pos);
    return ret;
}

void Qounters::CreateQounterComponent(Component const& qounterComponent, int componentIdx, Transform* parent, bool editing) {
    getLogger().debug("Creating qounter component of type %i", qounterComponent.Type);

    UI::Graphic* component;

    switch ((Component::Types) qounterComponent.Type) {
        case Component::Types::Text: {
            auto text = BeatSaberUI::CreateText(parent, "");
            if (editing)
                text->get_gameObject()->AddComponent<TextOutlineSizer*>();
            component = text;
            UpdateTextOptions(text, qounterComponent.Options, true);
            break;
        }
        case Component::Types::Shape: {
            auto shape = BeatSaberUI::CreateImage(parent, nullptr);
            component = shape;
            UpdateShapeOptions(shape, qounterComponent.Options, true);
            break;
        }
        case Component::Types::Image: {
            auto image = BeatSaberUI::CreateImage(parent, nullptr);
            component = image;
            UpdateImageOptions(image, qounterComponent.Options, true);
            break;
        }
        case Component::Types::BaseGame: {
            auto base = BaseGameGraphic::Create(parent);
            component = base;
            UpdateBaseGameOptions(base, qounterComponent.Options, true);
            break;
        }
    }

    UpdateColorOptions(component, qounterComponent.ColorSource, qounterComponent.ColorOptions, true);
    UpdateEnableOptions(component->get_gameObject(), qounterComponent.EnableSource, qounterComponent.EnableOptions, true, qounterComponent.InvertEnable);

    UpdateComponentPosition(component->get_rectTransform(), qounterComponent);

    if (editing) {
        auto edit = component->get_gameObject()->AddComponent<EditingComponent*>();
        edit->Init(component, componentIdx);
        Editor::RegisterEditingComponent(edit, edit->GetEditingGroup()->GetGroupIdx(), componentIdx);
    }
}

void Qounters::CreateQounterGroup(Group const& qounterGroup, int groupIdx, bool editing) {
    getLogger().debug("Creating qounter group");

    auto parent = GameObject::New_ctor("QounterGroup");
    auto parentTransform = parent->AddComponent<RectTransform*>();

    UpdateGroupPosition(parentTransform, qounterGroup);

    if (editing) {
        auto edit = parent->AddComponent<EditingGroup*>();
        edit->Init(groupIdx);
        Editor::RegisterEditingGroup(edit, groupIdx);
    }

    for (int i = 0; i < qounterGroup.Components.size(); i++)
        CreateQounterComponent(qounterGroup.Components[i], i, parentTransform, editing);
}

void Qounters::CreateQounters() {
    auto presets = getConfig().Presets.GetValue();
    auto presetName = getConfig().Preset.GetValue();

    if (!presets.contains(presetName))
        presetName = getConfig().Preset.GetDefaultValue();

    auto& preset = presets[presetName];

    // TODO: get correct preset for environment

    for (int i = 0; i < preset.Qounters.size(); i++)
        CreateQounterGroup(preset.Qounters[i], i, false);
}

void Qounters::Reset() {
    texts.clear();
    shapes.clear();
    colors.clear();
}

void Qounters::SetupObjects() {
    Reset();

    auto hud = GetHUD().first;
    if (!hud)
        return;

    BaseGameGraphic::MakeClones();

    for (int i = 0; i <= (int) Group::Anchors::AnchorMax; i++) GetAnchor(i);
    Utils::DisableAllBut(hud, {"QountersCanvas", "EnergyPanel"});
}

void UpdateTexts(std::string source) {
    auto sourceFn = GetSource(textSources, source).first;

    if (!texts.contains(source) || !sourceFn)
        return;

    auto& elements = texts[source];

    for (auto& [element, options] : elements)
        element->set_text(sourceFn(options));
}

void UpdateShapes(std::string source) {
    auto sourceFn = GetSource(shapeSources, source).first;

    if (!shapes.contains(source) || !sourceFn)
        return;

    auto& elements = shapes[source];

    for (auto& [element, options] : elements)
        element->set_fillAmount(sourceFn(options));
}

void UpdateColors(std::string source) {
    auto sourceFn = GetSource(colorSources, source).first;

    if (!colors.contains(source) || !sourceFn)
        return;

    auto& elements = colors[source];

    for (auto& [element, options] : elements)
        element->set_color(sourceFn(options));
}

void UpdateEnables(std::string source) {
    auto sourceFn = GetSource(enableSources, source).first;

    if (!enables.contains(source) || !sourceFn)
        return;

    auto& elements = enables[source];

    for (auto& [element, options] : elements) {
        auto& [json, invert] = options;
        bool enable = sourceFn(json);
        if (invert)
            enable = !enable;
        if (InSettingsEnvironment() && !Editor::GetPreviewMode())
            enable = true;
        element->SetActive(enable);
    }
}

void Qounters::UpdateSource(Sources sourceType, std::string source) {
    switch (sourceType) {
        case Sources::Text:
            UpdateTexts(source);
            break;
        case Sources::Shape:
            UpdateShapes(source);
            break;
        case Sources::Color:
            UpdateColors(source);
            break;
        case Sources::Enable:
            UpdateEnables(source);
            break;
    }
}

void Qounters::UpdateAllEnables() {
    for (auto& [source, _] : enables)
        UpdateEnables(source);
}
