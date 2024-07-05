#include "qounters.hpp"

#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/UI/Graphic.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "config.hpp"
#include "customtypes/components.hpp"
#include "customtypes/editing.hpp"
#include "editor.hpp"
#include "environment.hpp"
#include "main.hpp"
#include "sources.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace UnityEngine;

std::map<std::string, std::vector<std::pair<TMPro::TextMeshProUGUI*, UnparsedJSON>>> texts;
std::map<std::string, std::vector<std::pair<Shape*, UnparsedJSON>>> shapes;
std::map<std::string, std::vector<std::pair<UI::Graphic*, UnparsedJSON>>> colors;
std::map<std::string, std::vector<std::pair<GameObject*, std::pair<UnparsedJSON, bool>>>> enables;

template <class TComp, class TOpts>
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

template <class TComp, class TOpts>
void RemoveFromMap(std::map<std::string, std::vector<std::pair<TComp, TOpts>>>& map, void* remove) {
    auto cast = (TComp) remove;

    for (auto& [mapSource, vec] : map) {
        for (auto it = vec.begin(); it != vec.end(); it++) {
            if (it->first == cast) {
                vec.erase(it);
                break;
            }
        }
    }
}

void UpdateTextOptions(TMPro::TextMeshProUGUI* text, Qounters::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<TextOptions>().value_or(TextOptions{});

    text->fontStyle = options.Italic ? TMPro::FontStyles::Italic : TMPro::FontStyles::Normal;
    text->fontSize = options.Size;
    switch ((TextOptions::Aligns) options.Align) {
        case TextOptions::Aligns::Left:
            text->alignment = TMPro::TextAlignmentOptions::Left;
            break;
        case TextOptions::Aligns::Center:
            text->alignment = TMPro::TextAlignmentOptions::Center;
            break;
        case TextOptions::Aligns::Right:
            text->alignment = TMPro::TextAlignmentOptions::Right;
            break;
    }

    std::string source = options.TextSource;
    auto sourceFn = GetSource(textSources, source).first;
    if (!sourceFn)
        return;
    text->text = sourceFn(options.SourceOptions);

    UpdatePair(texts, text, source, options.SourceOptions, creation);

    if (auto outline = text->GetComponent<TextOutlineSizer*>())
        outline->SetDirty();
}

void UpdateShapeOptions(Shape* shape, Qounters::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<ShapeOptions>().value_or(ShapeOptions{});

    bool filled = false;
    int sideCount = 4;

    switch ((ShapeOptions::Shapes) options.Shape) {
        case ShapeOptions::Shapes::Square:
            filled = true;
        case ShapeOptions::Shapes::SquareOutline:
            sideCount = 4;
            break;
        case ShapeOptions::Shapes::Circle:
            filled = true;
        case ShapeOptions::Shapes::CircleOutline:
            sideCount = 50;
            break;
        case ShapeOptions::Shapes::Triangle:
            filled = true;
        case ShapeOptions::Shapes::TriangleOutline:
            sideCount = 3;
            break;
    }

    shape->SetFilled(filled);
    shape->SetSideCount(sideCount);
    shape->SetBorder(options.OutlineWidth);
    shape->SetMaskOptions(options.Fill, options.Inverse);

    std::string source = options.FillSource;
    auto sourceFn = GetSource(shapeSources, source).first;
    if (!sourceFn)
        return;
    shape->SetMaskAmount(sourceFn(options.SourceOptions));

    UpdatePair(shapes, shape, source, options.SourceOptions, creation);
}

void UpdateImageOptions(HMUI::ImageView* image, Qounters::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<ImageOptions>().value_or(ImageOptions{});

    Sprite* sprite = nullptr;
    if (fileexists(IMAGE_DIRECTORY + options.Path))
        sprite = ImageSpriteCache::GetSprite(options.Path);
    image->sprite = sprite;
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
            UpdateShapeOptions((Shape*) component, newOptions, false);
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
    component->color = sourceFn(options);

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
    component->active = enable;

    UpdatePair(enables, component, enableSource, {options, invert}, creation);
}

void Qounters::UpdateComponentEnabled(GameObject* component, std::string newSource, UnparsedJSON newOptions, bool invert) {
    UpdateEnableOptions(component, newSource, newOptions, false, invert);
}

void Qounters::UpdateComponentPosition(RectTransform* component, Component const& qounterComponent) {
    Shape* shape = nullptr;
    switch ((Component::Types) qounterComponent.Type) {
        case Component::Types::Text:
            component->sizeDelta = {0, 0};
            break;
        case Component::Types::Shape:
            shape = component->GetComponent<Shape*>();
            component = component->parent.cast<RectTransform>();
        case Component::Types::Image:
            component->sizeDelta = {25, 25};
            break;
        case Component::Types::BaseGame:
            break;
    }
    component->anchorMin = {0.5, 0.5};
    component->anchorMax = {0.5, 0.5};
    component->anchoredPosition = qounterComponent.Position;
    component->localEulerAngles = {0, 0, qounterComponent.Rotation};
    component->localScale = {qounterComponent.Scale.x, qounterComponent.Scale.y, 0};
    if (shape)
        shape->SetVerticesDirty();
    if (auto editing = component->GetComponent<EditingBase*>())
        editing->outline->UpdateSize();
}

void Qounters::UpdateGroupPosition(RectTransform* group, Group const& qounterGroup) {
    if (qounterGroup.Detached) {
        // parent to our own object instead of FlyingGameHUDRotation for more control
        if (auto parent = GameObject::Find("QountersRotationalAnchor"))
            group->SetParent(parent->transform, true);
        else
            group->SetParent(nullptr, true);
        group->position = qounterGroup.DetachedPosition;
        group->eulerAngles = qounterGroup.DetachedRotation;
        group->localScale = {0.02, 0.02, 0.02};
        return;
    }

    auto anchor = GetAnchor(qounterGroup.Anchor);
    if (!anchor)
        return;
    group->SetParent(anchor, false);

    group->anchorMin = {0.5, 0.5};
    group->anchorMax = {0.5, 0.5};
    group->sizeDelta = {0, 0};
    group->localPosition = {qounterGroup.Position.x, qounterGroup.Position.y, 0};
    group->localEulerAngles = {0, 0, qounterGroup.Rotation};
    group->localScale = {1, 1, 1};
    // necessary if scale changing becomes supported
    // for (auto& editing : group->GetComponentsInChildren<EditingBase*>())
    //     editing->outline->UpdateSize();
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
    RemoveFromMap(enables, component->get_gameObject());
}

template <class T>
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

std::map<std::string, HUDType> const supportedHUDs = {
    {"BasicGameHUD", HUDType::Basic},
    {"NarrowGameHUD", HUDType::Basic},
    {"LatticeHUD", HUDType::Basic},
    {"RockGameHUD", HUDType::Basic},
    {"FlyingGameHUD/Container", HUDType::Rotational},
    {"MultiplayerLocalActivePlayerController(Clone)/IsActiveObjects/HUD", HUDType::Multiplayer},
    {"MultiplayerDuelLocalActivePlayerController(Clone)/IsActiveObjects/HUD", HUDType::Multiplayer},
};

std::map<HUDType, std::map<Group::Anchors, std::tuple<std::string, Vector3, Vector3, Vector2, Vector2>>> const hudPanels = {
    {HUDType::Basic,
     {
         {Group::Anchors::Left, {"LeftPanel", {-3, 0.4, 7}, {0, 0, 0}, {50, 125}, {0, 0.75}}},
         {Group::Anchors::Right, {"RightPanel", {3, 0.4, 7}, {0, 0, 0}, {50, 125}, {0, 0.75}}},
         {Group::Anchors::Bottom, {"EnergyPanel", {0, -0.64, 7}, {0, 0, 0}, {125, 50}, {0, -10}}},
         {Group::Anchors::Top, {"QountersTopPanel", {0, 3.2, 7}, {0, 0, 0}, {125, 50}, {0, 0}}},
         {Group::Anchors::Center, {"QountersCenterPanel", {0, 1.5, 7}, {0, 0, 0}, {125, 80}, {0, 0}}},
     }},
    {HUDType::Rotational,
     {
         {Group::Anchors::Left, {"ComboPanel", {-1.6, 4, 12}, {345, 0, 0}, {50, 125}, {0, 0}}},
         {Group::Anchors::Right, {"MultiplierCanvas", {1.6, 4, 12}, {345, 0, 0}, {50, 125}, {0, 0}}},
         {Group::Anchors::Bottom, {"EnergyPanel", {0, 3.266, 12.197}, {345, 0, 0}, {100, 50}, {0, -5}}},
         {Group::Anchors::Top, {"SongProgressCanvas", {0.05, 4.58, 11.845}, {345, 0, 0}, {100, 50}, {-2.5, 15}}},
         {Group::Anchors::Center, {"QountersCenterPanel", {0, 1.5, 7}, {0, 0, 0}, {125, 80}, {0, 0}}},
     }},
    {HUDType::Multiplayer,
     {
         {Group::Anchors::Left, {"QountersLeftPanel", {-3, 0.4, 7}, {0, 0, 0}, {50, 125}, {0, 0.75}}},
         {Group::Anchors::Right, {"QountersRightPanel", {3, 0.4, 7}, {0, 0, 0}, {50, 125}, {0, 0.75}}},
         {Group::Anchors::Bottom, {"EnergyPanel", {0, 0, 2.3}, {90, 0, 0}, {125, 50}, {0, 0}}},
         {Group::Anchors::Top, {"QountersTopPanel", {0, 3.2, 7}, {0, 0, 0}, {125, 50}, {0, 0}}},
         {Group::Anchors::Center, {"QountersCenterPanel", {0, 1.5, 7}, {0, 0, 0}, {125, 80}, {0, 0}}},
     }},
};

RectTransform* GetCanvas(std::string parentName, Transform* hud, Vector3 fallback, Vector3 fallbackRot) {
    static ConstString name("QountersCanvas");

    auto parent = Utils::FindRecursive(hud, parentName);
    if (!parent && !parentName.starts_with("Qounters")) {
        logger.info("Failed to find parent {}!", parentName);
        parentName = "Qounters" + parentName;
        parent = Utils::FindRecursive(hud, parentName);
    }
    if (!parent) {
        logger.info("Creating custom parent object {}", parentName);
        parent = GameObject::New_ctor(parentName)->transform;
        parent->position = fallback;
        parent->eulerAngles = fallbackRot;
        parent->SetParent(hud, true);
    }

    if (auto ret = parent->Find(name))
        return ret.cast<RectTransform>();

    auto canvas = BSML::Lite::CreateCanvas();
    canvas->name = name;

    canvas->GetComponent<Canvas*>()->sortingOrder = 0;

    auto ret = canvas->GetComponent<RectTransform*>();
    ret->localScale = {0.02, 0.02, 0.02};
    ret->SetParent(parent, true);
    ret->localPosition = {0, 0, -0.5};
    ret->localEulerAngles = {0, 0, 0};
    ret->anchorMin = {0.5, 0.5};
    ret->anchorMax = {0.5, 0.5};

    return ret;
}

std::pair<Transform*, HUDType> Qounters::GetHUD() {
    for (auto& [name, type] : supportedHUDs) {
        if (auto hud = GameObject::Find(name))
            return {hud->transform, type};
    }
    logger.error("Unable to find HUD object");
    return {nullptr, HUDType::Unsupported};
}

Transform* Qounters::GetAnchor(int anchor) {
    auto [hud, type] = GetHUD();
    if (type == HUDType::Unsupported)
        return nullptr;

    auto [name, fallback, fallbackRot, size, pos] = hudPanels.at(type).at((Group::Anchors) anchor);
    auto ret = GetCanvas(name, hud, fallback, fallbackRot);
    ret->sizeDelta = size;
    ret->anchoredPosition = pos;
    return ret;
}

void Qounters::CreateQounterComponent(Component const& qounterComponent, int componentIdx, Transform* parent, bool editing) {
    logger.debug("Creating qounter component of type {}", qounterComponent.Type);

    UI::Graphic* component;

    switch ((Component::Types) qounterComponent.Type) {
        case Component::Types::Text: {
            auto text = BSML::Lite::CreateText(parent, "");
            text->enableWordWrapping = false;
            if (editing)
                text->gameObject->AddComponent<TextOutlineSizer*>();
            component = text;
            UpdateTextOptions(text, qounterComponent.Options, true);
            break;
        }
        case Component::Types::Shape: {
            auto shape = Shape::Create(parent);
            component = shape;
            UpdateShapeOptions(shape, qounterComponent.Options, true);
            break;
        }
        case Component::Types::Image: {
            auto image = BSML::Lite::CreateImage(parent, nullptr);
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
    UpdateEnableOptions(component->gameObject, qounterComponent.EnableSource, qounterComponent.EnableOptions, true, qounterComponent.InvertEnable);

    UpdateComponentPosition(component->rectTransform, qounterComponent);

    if (editing) {
        auto edit = component->gameObject->AddComponent<EditingComponent*>();
        edit->Init(component, componentIdx);
        Editor::RegisterEditingComponent(edit, edit->GetEditingGroup()->GetGroupIdx(), componentIdx);
    }
}

void Qounters::CreateQounterGroup(Group const& qounterGroup, int groupIdx, bool editing) {
    logger.debug("Creating qounter group");

    auto parent = BSML::Lite::CreateCanvas();
    parent->name = "QounterGroup";
    auto parentTransform = parent->GetComponent<RectTransform*>();
    parentTransform->localScale = {1, 1, 1};

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
    if (GetHUD().second == HUDType::Unsupported)
        return;

    auto presets = getConfig().Presets.GetValue();
    auto presetName = getConfig().Preset.GetValue();

    if (!presets.contains(presetName)) {
        presetName = presets.begin()->first;
        getConfig().Preset.SetValue(presetName);
    }

    auto& preset = presets[presetName];

    for (int i = 0; i < preset.Qounters.size(); i++)
        CreateQounterGroup(preset.Qounters[i], i, false);
}

void Qounters::Reset() {
    texts.clear();
    shapes.clear();
    colors.clear();
    enables.clear();
    BaseGameGraphic::Reset();
}

void Qounters::SetupObjects() {
    Reset();

    auto [hud, type] = GetHUD();
    if (type == HUDType::Unsupported)
        return;

    BaseGameGraphic::MakeClones();

    for (int i = 0; i <= (int) Group::Anchors::AnchorMax; i++)
        GetAnchor(i);
    Utils::DisableAllBut(hud, {"QountersCanvas", "EnergyPanel"});
}

void UpdateTexts(std::string source) {
    auto sourceFn = GetSource(textSources, source).first;

    if (!texts.contains(source) || !sourceFn)
        return;

    auto& elements = texts[source];

    for (auto& [element, options] : elements)
        element->text = sourceFn(options);
}

void UpdateShapes(std::string source) {
    auto sourceFn = GetSource(shapeSources, source).first;

    if (!shapes.contains(source) || !sourceFn)
        return;

    auto& elements = shapes[source];

    for (auto& [element, options] : elements)
        element->SetMaskAmount(sourceFn(options));
}

void UpdateColors(std::string source) {
    auto sourceFn = GetSource(colorSources, source).first;

    if (!colors.contains(source) || !sourceFn)
        return;

    auto& elements = colors[source];

    for (auto& [element, options] : elements)
        element->color = sourceFn(options);
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
        element->active = enable;
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
