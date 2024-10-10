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
#include "internals.hpp"
#include "main.hpp"
#include "options.hpp"
#include "sources.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace UnityEngine;

static void DisableGradient(UI::Graphic* component) {
    if (auto image = Utils::ptr_cast<HMUI::ImageView>(component)) {
        image->gradient = false;
    } else if (auto shape = Utils::ptr_cast<Shape>(component)) {
        shape->gradient = false;
        shape->SetVerticesDirty();
    } else if (auto text = Utils::ptr_cast<TMPro::TextMeshProUGUI>(component)) {
        if (auto gradient = text->GetComponent<TextGradient*>())
            gradient->enabled = false;
    }
}

static void SetGradient(UI::Graphic* component, Options::Gradient const& options) {
    auto& startHsvMod = options.StartModifierHSV;
    auto& endHsvMod = options.EndModifierHSV;
    float h, s, v;
    Color::RGBToHSV(component->color, byref(h), byref(s), byref(v));
    auto startColor = Utils::GetClampedColor({h + startHsvMod.x, s + startHsvMod.y, v + startHsvMod.z});
    auto endColor = Utils::GetClampedColor({h + endHsvMod.x, s + endHsvMod.y, v + endHsvMod.z});

    if (auto image = Utils::ptr_cast<HMUI::ImageView>(component)) {
        image->_gradientDirection = options.Direction;  // same enum values
        image->color0 = startColor;
        image->color1 = endColor;
        image->gradient = true;
    } else if (auto shape = Utils::ptr_cast<Shape>(component)) {
        shape->gradient = true;
        shape->gradientDirection = options.Direction;
        shape->startColor = startColor;
        shape->endColor = endColor;
        shape->SetVerticesDirty();
    } else if (auto text = Utils::ptr_cast<TMPro::TextMeshProUGUI>(component)) {
        auto gradient = Utils::GetOrAddComponent<TextGradient*>(text);
        gradient->gradientDirection = options.Direction;
        gradient->startColor = startColor;
        gradient->endColor = endColor;
        if (gradient->enabled)
            gradient->UpdateGradient();
        else
            gradient->enabled = true;
    }
}

static std::map<std::string, std::vector<std::pair<TMPro::TextMeshProUGUI*, UnparsedJSON>>> texts;
static std::map<std::string, std::vector<std::pair<Shape*, UnparsedJSON>>> shapes;
static std::map<std::string, std::vector<std::pair<UI::Graphic*, std::pair<UnparsedJSON, Options::Gradient>>>> colors;
static std::map<std::string, std::vector<std::pair<GameObject*, std::pair<UnparsedJSON, bool>>>> enables;

template <class TComp, class TOpts>
static void
UpdatePair(std::map<std::string, std::vector<std::pair<TComp, TOpts>>>& map, TComp update, std::string source, TOpts value, bool forceAdd) {
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
static void RemoveFromMap(std::map<std::string, std::vector<std::pair<TComp, TOpts>>>& map, void* remove) {
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

static void AddEditingComponent(UI::Graphic* component, int componentIndex) {
    auto edit = component->gameObject->AddComponent<EditingComponent*>();
    edit->Init(component, componentIndex);
    Editor::RegisterEditingComponent(edit, edit->GetEditingGroup()->GetGroupIdx(), componentIndex);
}

static void UpdateTextOptions(TMPro::TextMeshProUGUI* text, Options::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<Options::Text>().value_or(Options::Text{});

    text->fontStyle = options.Italic ? TMPro::FontStyles::Italic : TMPro::FontStyles::Normal;
    text->fontSize = options.Size;
    switch ((Options::Text::Aligns) options.Align) {
        case Options::Text::Aligns::Left:
            text->alignment = TMPro::TextAlignmentOptions::Left;
            break;
        case Options::Text::Aligns::Center:
            text->alignment = TMPro::TextAlignmentOptions::Center;
            break;
        case Options::Text::Aligns::Right:
            text->alignment = TMPro::TextAlignmentOptions::Right;
            break;
    }

    std::string source = options.TextSource;
    auto sourceFn = Sources::GetSource(Sources::texts, source).first;
    if (!sourceFn)
        return;
    text->text = sourceFn(options.SourceOptions);

    UpdatePair(texts, text, source, options.SourceOptions, creation);

    if (auto outline = text->GetComponent<TextOutlineSizer*>())
        outline->SetDirty();
}

static void UpdateShapeOptions(Shape* shape, Options::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<Options::Shape>().value_or(Options::Shape{});

    bool filled = false;
    int sideCount = 4;

    switch ((Options::Shape::Shapes) options.Shape) {
        case Options::Shape::Shapes::Square:
            filled = true;
        case Options::Shape::Shapes::SquareOutline:
            sideCount = 4;
            break;
        case Options::Shape::Shapes::Circle:
            filled = true;
        case Options::Shape::Shapes::CircleOutline:
            sideCount = 50;
            break;
        case Options::Shape::Shapes::Triangle:
            filled = true;
        case Options::Shape::Shapes::TriangleOutline:
            sideCount = 3;
            break;
    }

    shape->SetFilled(filled);
    shape->SetSideCount(sideCount);
    shape->SetBorder(options.OutlineWidth);
    shape->SetMaskOptions(options.Fill, options.Inverse);

    std::string source = options.FillSource;
    auto sourceFn = Sources::GetSource(Sources::shapes, source).first;
    if (!sourceFn)
        return;
    shape->SetMaskAmount(sourceFn(options.SourceOptions));

    UpdatePair(shapes, shape, source, options.SourceOptions, creation);
}

static void UpdateImageOptions(HMUI::ImageView* image, Options::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<Options::Image>().value_or(Options::Image{});

    Sprite* sprite = nullptr;
    if (fileexists(IMAGE_DIRECTORY + options.Path))
        sprite = ImageSpriteCache::GetSprite(options.Path);
    image->sprite = sprite;
}

static PremadeParent* UpdatePremadeOptions(PremadeParent* premade, Options::Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<Options::Premade>().value_or(Options::Premade{});

    auto info = Sources::GetPremadeInfo(options.SourceMod, options.Name);
    if (!info) {
        logger.error("premade {}.{} not found!", options.SourceMod, options.Name);
        return nullptr;
    }

    if (!creation) {
        if (!info->update || premade->options.SourceMod != options.SourceMod || premade->options.Name != options.Name) {
            Object::Destroy(premade->GetGraphic()->gameObject);
            premade->graphic = nullptr;
            creation = true;
        } else
            info->update(premade->GetGraphic()->GetComponent<UI::Graphic*>(), options.Options);
    }
    if (creation)
        premade->graphic = info->creation(premade->gameObject, options.Options);
    premade->options = options;
    return premade;
}

void HUD::UpdateComponentOptions(int componentType, UnityEngine::Component* component, Options::Component::OptionsTypes newOptions) {
    switch ((Options::Component::Types) componentType) {
        case Options::Component::Types::Text:
            UpdateTextOptions((TMPro::TextMeshProUGUI*) component, newOptions, false);
            break;
        case Options::Component::Types::Shape:
            UpdateShapeOptions((Shape*) component, newOptions, false);
            break;
        case Options::Component::Types::Image:
            UpdateImageOptions((HMUI::ImageView*) component, newOptions, false);
            break;
        case Options::Component::Types::Premade:
            UpdatePremadeOptions((PremadeParent*) component, newOptions, false);
            break;
    }
}

static void
UpdateColorOptions(UI::Graphic* component, std::string colorSource, UnparsedJSON options, Options::Gradient gradientOptions, bool creation) {
    auto sourceFn = Sources::GetSource(Sources::colors, colorSource).first;
    if (!sourceFn)
        return;
    component->color = sourceFn(options);

    if (gradientOptions.Enabled)
        SetGradient(component, gradientOptions);
    else
        DisableGradient(component);

    UpdatePair(colors, component, colorSource, {options, gradientOptions}, creation);
}

void HUD::UpdateComponentColor(UI::Graphic* component, std::string newSource, UnparsedJSON newOptions, Options::Gradient gradientOptions) {
    UpdateColorOptions(component, newSource, newOptions, gradientOptions, false);
}

void UpdateEnableOptions(GameObject* component, std::string enableSource, UnparsedJSON options, bool creation, bool invert) {
    auto sourceFn = Sources::GetSource(Sources::enables, enableSource).first;
    if (!sourceFn)
        return;
    bool enable = sourceFn(options);
    if (invert)
        enable = !enable;
    if (Environment::InSettings() && !Editor::GetPreviewMode())
        enable = true;
    component->active = enable;

    UpdatePair(enables, component, enableSource, {options, invert}, creation);
}

void HUD::UpdateComponentEnabled(GameObject* component, std::string newSource, UnparsedJSON newOptions, bool invert) {
    UpdateEnableOptions(component, newSource, newOptions, false, invert);
}

void HUD::UpdateComponentPosition(RectTransform* component, Options::Component const& qounterComponent) {
    Shape* shape = nullptr;
    switch ((Options::Component::Types) qounterComponent.Type) {
        case Options::Component::Types::Text:
            component->sizeDelta = {0, 0};
            break;
        case Options::Component::Types::Shape:
            shape = component->GetComponent<Shape*>();
            component = component->parent.cast<RectTransform>();
        case Options::Component::Types::Image:
            component->sizeDelta = {25, 25};
            break;
        case Options::Component::Types::Premade:
            break;
    }
    component->anchorMin = {0.5, 0.5};
    component->anchorMax = {0.5, 0.5};
    component->anchoredPosition = qounterComponent.Position;
    component->localEulerAngles = {0, 0, qounterComponent.Rotation};
    component->localScale = {qounterComponent.Scale.x, qounterComponent.Scale.y, 0};
    if (shape) {
        shape->SetVerticesDirty();
        component = shape->rectTransform;
    }
    if (auto editing = component->GetComponent<EditingBase*>())
        editing->outline->UpdateSize();
}

void HUD::UpdateGroupPosition(RectTransform* group, Options::Group const& qounterGroup) {
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

void HUD::RemoveComponent(int componentType, UnityEngine::Component* component) {
    switch ((Options::Component::Types) componentType) {
        case Options::Component::Types::Text:
            RemoveFromMap(texts, component);
            break;
        case Options::Component::Types::Shape:
            RemoveFromMap(shapes, component);
            break;
        case Options::Component::Types::Image:
        case Options::Component::Types::Premade:
            break;
    }
    RemoveFromMap(colors, component);
    RemoveFromMap(enables, component->gameObject);
}

template <class T>
static inline void SetSourceOptions(Options::Component::OptionsTypes& options, UnparsedJSON newOptions) {
    auto opts = options.GetValue<T>().value_or(T());
    opts.SourceOptions = newOptions;
    options.SetValue(opts);
}

void HUD::SetSourceOptions(Options::Component& component, UnparsedJSON newOptions) {
    switch ((Options::Component::Types) component.Type) {
        case Options::Component::Types::Text:
            SetSourceOptions<Options::Text>(component.Options, newOptions);
            break;
        case Options::Component::Types::Shape:
            SetSourceOptions<Options::Shape>(component.Options, newOptions);
            break;
        case Options::Component::Types::Image:
        case Options::Component::Types::Premade:
            break;
    }
}

void HUD::SetDefaultOptions(Options::Component& component) {
    switch ((Options::Component::Types) component.Type) {
        case Options::Component::Types::Text:
            component.Options = Options::Text();
            break;
        case Options::Component::Types::Shape:
            component.Options = Options::Shape();
            break;
        case Options::Component::Types::Image:
            component.Options = Options::Image();
            break;
        case Options::Component::Types::Premade:
            component.Options = Options::Premade();
            break;
    }
}

static std::map<std::string, HUD::Type> const SupportedHUDs = {
    {"BasicGameHUD", HUD::Type::Basic},
    {"NarrowGameHUD", HUD::Type::Basic},
    {"NarrowGameHUDVariant", HUD::Type::Basic},
    {"LatticeHUD", HUD::Type::Basic},
    {"RockGameHUD", HUD::Type::Basic},
    {"FlyingGameHUD/Container", HUD::Type::Rotational},
    {"MultiplayerLocalActivePlayerController(Clone)/IsActiveObjects/HUD", HUD::Type::Multiplayer},
    {"MultiplayerDuelLocalActivePlayerController(Clone)/IsActiveObjects/HUD", HUD::Type::Multiplayer},
};

static std::map<HUD::Type, std::map<Options::Group::Anchors, std::tuple<std::string, Vector3, Vector3, Vector2, Vector2>>> const HudPanels = {
    {HUD::Type::Basic,
     {
         {Options::Group::Anchors::Left, {"LeftPanel", {-3, 0.4, 7}, {0, 0, 0}, {50, 125}, {0, 0.75}}},
         {Options::Group::Anchors::Right, {"RightPanel", {3, 0.4, 7}, {0, 0, 0}, {50, 125}, {0, 0.75}}},
         {Options::Group::Anchors::Bottom, {"EnergyPanel", {0, -0.64, 7}, {0, 0, 0}, {125, 50}, {0, -10}}},
         {Options::Group::Anchors::Top, {"QountersTopPanel", {0, 3.2, 7}, {0, 0, 0}, {125, 50}, {0, 0}}},
         {Options::Group::Anchors::Center, {"QountersCenterPanel", {0, 1.5, 7}, {0, 0, 0}, {125, 80}, {0, 0}}},
     }},
    {HUD::Type::Rotational,
     {
         {Options::Group::Anchors::Left, {"ComboPanel", {-1.6, 4, 12}, {345, 0, 0}, {50, 125}, {0, 0}}},
         {Options::Group::Anchors::Right, {"MultiplierCanvas", {1.6, 4, 12}, {345, 0, 0}, {50, 125}, {0, 0}}},
         {Options::Group::Anchors::Bottom, {"EnergyPanel", {0, 3.266, 12.197}, {345, 0, 0}, {100, 50}, {0, -5}}},
         {Options::Group::Anchors::Top, {"SongProgressCanvas", {0.05, 4.58, 11.845}, {345, 0, 0}, {100, 50}, {-2.5, 15}}},
         {Options::Group::Anchors::Center, {"QountersCenterPanel", {0, 1.5, 7}, {0, 0, 0}, {125, 80}, {0, 0}}},
     }},
    {HUD::Type::Multiplayer,
     {
         {Options::Group::Anchors::Left, {"QountersLeftPanel", {-3, 0.4, 7}, {0, 0, 0}, {50, 125}, {0, 0.75}}},
         {Options::Group::Anchors::Right, {"QountersRightPanel", {3, 0.4, 7}, {0, 0, 0}, {50, 125}, {0, 0.75}}},
         {Options::Group::Anchors::Bottom, {"EnergyPanel", {0, 0, 2.3}, {90, 0, 0}, {125, 50}, {0, 0}}},
         {Options::Group::Anchors::Top, {"QountersTopPanel", {0, 3.2, 7}, {0, 0, 0}, {125, 50}, {0, 0}}},
         {Options::Group::Anchors::Center, {"QountersCenterPanel", {0, 1.5, 7}, {0, 0, 0}, {125, 80}, {0, 0}}},
     }},
};

static RectTransform* GetCanvas(std::string parentName, Transform* hud, Vector3 fallback, Vector3 fallbackRot) {
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
    Utils::SetCanvasSorting(canvas, 0);

    auto ret = canvas->GetComponent<RectTransform*>();
    ret->localScale = {0.02, 0.02, 0.02};
    ret->SetParent(parent, true);
    ret->localPosition = {0, 0, -0.01};
    ret->localEulerAngles = {0, 0, 0};
    ret->anchorMin = {0.5, 0.5};
    ret->anchorMax = {0.5, 0.5};

    // I have no idea why but this fixes sprites being blurry on the 360 degree right panel
    canvas->GetComponent<Canvas*>()->rootCanvas->referencePixelsPerUnit = 8;

    return ret;
}

std::pair<Transform*, HUD::Type> HUD::GetHUD() {
    for (auto& [name, type] : SupportedHUDs) {
        if (auto hud = GameObject::Find(name))
            return {hud->transform, type};
    }
    logger.error("Unable to find HUD object");
    return {nullptr, HUD::Type::Unsupported};
}

Transform* HUD::GetAnchor(int anchor) {
    auto [hud, type] = GetHUD();
    if (type == HUD::Type::Unsupported)
        return nullptr;

    auto [name, fallback, fallbackRot, size, pos] = HudPanels.at(type).at((Options::Group::Anchors) anchor);
    auto ret = GetCanvas(name, hud, fallback, fallbackRot);
    ret->sizeDelta = size;
    ret->anchoredPosition = pos;
    return ret;
}

void HUD::CreateQounterComponent(Options::Component const& qounterComponent, int componentIdx, Transform* parent, bool editing) {
    logger.debug("Creating qounter component of type {}", qounterComponent.Type);

    UI::Graphic* component;

    switch ((Options::Component::Types) qounterComponent.Type) {
        case Options::Component::Types::Text: {
            auto text = BSML::Lite::CreateText(parent, "");
            text->enableWordWrapping = false;
            if (editing)
                text->gameObject->AddComponent<TextOutlineSizer*>();
            component = text;
            UpdateTextOptions(text, qounterComponent.Options, true);
            break;
        }
        case Options::Component::Types::Shape: {
            auto shape = Shape::Create(parent);
            component = shape;
            UpdateShapeOptions(shape, qounterComponent.Options, true);
            break;
        }
        case Options::Component::Types::Image: {
            auto image = BSML::Lite::CreateImage(parent, nullptr);
            component = image;
            UpdateImageOptions(image, qounterComponent.Options, true);
            break;
        }
        case Options::Component::Types::Premade: {
            auto premade = GameObject::New_ctor("QountersPremade")->AddComponent<PremadeParent*>();
            premade->transform->SetParent(parent, false);
            component = premade;
            UpdatePremadeOptions(premade, qounterComponent.Options, true);
            break;
        }
    }

    UpdateColorOptions(component, qounterComponent.ColorSource, qounterComponent.ColorOptions, qounterComponent.GradientOptions, true);
    UpdateEnableOptions(component->gameObject, qounterComponent.EnableSource, qounterComponent.EnableOptions, true, qounterComponent.InvertEnable);

    UpdateComponentPosition(component->rectTransform, qounterComponent);

    if (editing) {
        auto edit = component->gameObject->AddComponent<EditingComponent*>();
        edit->Init(component, componentIdx);
        Editor::RegisterEditingComponent(edit, edit->GetEditingGroup()->GetGroupIdx(), componentIdx);
    }
}

void HUD::CreateQounterGroup(Options::Group const& qounterGroup, int groupIdx, bool editing) {
    logger.debug("Creating qounter group");

    auto parent = BSML::Lite::CreateCanvas();
    parent->name = "QounterGroup";
    Utils::SetCanvasSorting(parent, 0);
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

static Options::Preset GetPreset() {
    auto presets = getConfig().Presets.GetValue();

    if (Internals::environment) {
        std::string serializedName = Internals::environment->serializedName;
        auto specificPresets = getConfig().SpecificPresets.GetValue();
        if (specificPresets.contains(serializedName) && specificPresets[serializedName].Enabled) {
            auto ret = specificPresets[serializedName].Preset;
            if (presets.contains(ret))
                return presets[ret];
            specificPresets[serializedName].Enabled = false;
            specificPresets[serializedName].Preset = presets.begin()->first;
        }

        std::string hudTypeString = std::to_string((int) Environment::GetHUDType(Internals::environment->serializedName));
        auto typePresets = getConfig().TypePresets.GetValue();
        if (typePresets.contains(hudTypeString) && typePresets[hudTypeString].Enabled) {
            auto ret = typePresets[hudTypeString].Preset;
            if (presets.contains(ret))
                return presets[ret];
            typePresets[hudTypeString].Enabled = false;
            typePresets[hudTypeString].Preset = presets.begin()->first;
        }
    }

    auto ret = getConfig().Preset.GetValue();
    if (!presets.contains(ret)) {
        ret = presets.begin()->first;
        getConfig().Preset.SetValue(ret);
    }
    return presets[ret];
}

void HUD::CreateQounters() {
    if (GetHUD().second == HUD::Type::Unsupported)
        return;
    if (getConfig().Noodle.GetValue() && !Utils::GetSimplifiedRequirements(Internals::beatmapKey).empty())
        return;

    auto preset = GetPreset();
    for (int i = 0; i < preset.Qounters.size(); i++)
        CreateQounterGroup(preset.Qounters[i], i, false);
}

void HUD::Reset(bool sceneEnd) {
    texts.clear();
    shapes.clear();
    colors.clear();
    enables.clear();
    if (sceneEnd)
        BaseGameGraphic::Reset();
}

void HUD::SetupObjects() {
    Reset();

    auto [hud, type] = GetHUD();
    if (type == HUD::Type::Unsupported)
        return;

    BaseGameGraphic::MakeClones();

    for (int i = 0; i <= (int) Options::Group::Anchors::AnchorMax; i++)
        GetAnchor(i);
    Utils::DisableAllBut(hud, {"QountersCanvas", "EnergyPanel"});
}

static void UpdateTexts(std::string source) {
    auto sourceFn = Sources::GetSource(Sources::texts, source).first;

    if (!texts.contains(source) || !sourceFn)
        return;

    auto& elements = texts[source];

    for (auto& [element, options] : elements)
        element->text = sourceFn(options);
}

static void UpdateShapes(std::string source) {
    auto sourceFn = Sources::GetSource(Sources::shapes, source).first;

    if (!shapes.contains(source) || !sourceFn)
        return;

    auto& elements = shapes[source];

    for (auto& [element, options] : elements)
        element->SetMaskAmount(sourceFn(options));
}

static void UpdateColors(std::string source) {
    auto sourceFn = Sources::GetSource(Sources::colors, source).first;

    if (!colors.contains(source) || !sourceFn)
        return;

    auto& elements = colors[source];

    for (auto& [element, options] : elements) {
        auto& [json, gradient] = options;
        element->color = sourceFn(json);
        if (gradient.Enabled)
            SetGradient(element, gradient);
    }
}

static void UpdateEnables(std::string source) {
    auto sourceFn = Sources::GetSource(Sources::enables, source).first;

    if (!enables.contains(source) || !sourceFn)
        return;

    auto& elements = enables[source];

    for (auto& [element, options] : elements) {
        auto& [json, invert] = options;
        bool enable = sourceFn(json);
        if (invert)
            enable = !enable;
        if (Environment::InSettings() && !Editor::GetPreviewMode())
            enable = true;
        element->active = enable;
    }
}

void HUD::UpdateSource(Types::Sources sourceType, std::string source) {
    switch (sourceType) {
        case Types::Sources::Text:
            UpdateTexts(source);
            break;
        case Types::Sources::Shape:
            UpdateShapes(source);
            break;
        case Types::Sources::Color:
            UpdateColors(source);
            break;
        case Types::Sources::Enable:
            UpdateEnables(source);
            break;
    }
}

void HUD::UpdateAllEnables() {
    for (auto& [source, _] : Sources::enables)
        UpdateEnables(source);
}

void HUD::UpdateAllSources() {
    for (auto& [source, _] : Sources::texts)
        UpdateTexts(source);
    for (auto& [source, _] : Sources::shapes)
        UpdateShapes(source);
    for (auto& [source, _] : Sources::colors)
        UpdateColors(source);
    for (auto& [source, _] : Sources::enables)
        UpdateEnables(source);
}
