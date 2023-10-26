#include "qounters.hpp"
#include "config.hpp"
#include "customtypes/editing.hpp"
#include "customtypes/components.hpp"
#include "editor.hpp"
#include "main.hpp"
#include "sources.hpp"
#include "utils.hpp"

using namespace Qounters;

#include "TMPro/TextMeshProUGUI.hpp"
#include "HMUI/ImageView.hpp"
#include "UnityEngine/UI/Graphic.hpp"

std::map<std::string, std::vector<std::pair<TMPro::TextMeshProUGUI*, UnparsedJSON>>> texts;
std::map<std::string, std::vector<std::pair<HMUI::ImageView*, UnparsedJSON>>> shapes;
std::map<std::string, std::vector<std::pair<UnityEngine::UI::Graphic*, UnparsedJSON>>> colors;

template<class T>
void UpdatePair(std::map<std::string, std::vector<std::pair<T, UnparsedJSON>>>& map, T update, std::string source, UnparsedJSON& value, bool forceAdd) {
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

UnityEngine::Sprite* GetShapeSprite(int shape) { // TODO
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

void UpdateTextOptions(TMPro::TextMeshProUGUI* text, Component::OptionsTypes newOptions, bool creation) {
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

void UpdateShapeOptions(HMUI::ImageView* shape, Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<ShapeOptions>().value_or(ShapeOptions{});

    shape->set_sprite(GetShapeSprite(options.Shape));

    auto fill = options.Fill;
    switch ((ShapeOptions::Fills) fill) {
        case ShapeOptions::Fills::None:
            // could remove it from the source shapes map as well, but it's probably insignificant
            shape->set_type(UnityEngine::UI::Image::Type::Simple);
            return;
        case ShapeOptions::Fills::Horizontal:
            shape->set_fillMethod(UnityEngine::UI::Image::FillMethod::Horizontal);
            shape->set_fillOrigin(options.Inverse
                ? UnityEngine::UI::Image::OriginHorizontal::Right
                : UnityEngine::UI::Image::OriginHorizontal::Left);
            break;
        case ShapeOptions::Fills::Vertical:
            shape->set_fillMethod(UnityEngine::UI::Image::FillMethod::Vertical);
            shape->set_fillOrigin(options.Inverse
                ? UnityEngine::UI::Image::OriginVertical::Bottom
                : UnityEngine::UI::Image::OriginVertical::Top);
            break;
        case ShapeOptions::Fills::Circle:
            shape->set_fillMethod(UnityEngine::UI::Image::FillMethod::Radial360);
            shape->set_fillOrigin(UnityEngine::UI::Image::Origin360::Top);
            shape->set_fillClockwise(!options.Inverse);
            break;
    }
    shape->set_type(UnityEngine::UI::Image::Type::Filled);

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

void UpdateImageOptions(HMUI::ImageView* image, Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<ImageOptions>().value_or(ImageOptions{});

    UnityEngine::Sprite* sprite = nullptr;
    if (fileexists(IMAGE_DIRECTORY + options.Path))
        sprite = ImageSpriteCache::GetSprite(options.Path);
    image->set_sprite(sprite);
}

void UpdateBaseGameOptions(BaseGameGraphic* base, Component::OptionsTypes newOptions, bool creation) {
    auto options = newOptions.GetValue<BaseGameOptions>().value_or(BaseGameOptions{});

    base->SetComponent(options.Component);
}

void Qounters::UpdateComponentOptions(int componentType, UnityEngine::Component* component, Component::OptionsTypes newOptions) {
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

void UpdateColorOptions(UnityEngine::UI::Graphic* component, std::string colorSource, UnparsedJSON options, bool creation) {
    auto sourceFn = GetSource(colorSources, colorSource).first;
    if (!sourceFn)
        return;
    auto color = sourceFn(options);
    component->set_color(color);

    UpdatePair(colors, component, colorSource, options, creation);
}

void Qounters::UpdateComponentColor(UnityEngine::UI::Graphic* component, std::string newSource, UnparsedJSON newOptions) {
    UpdateColorOptions(component, newSource, newOptions, false);
}

void Qounters::UpdateComponentPosition(UnityEngine::RectTransform* component, Component const& qounterComponent) {
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

void Qounters::UpdateGroupPosition(UnityEngine::RectTransform* group, Group const& qounterGroup) {
    auto anchor = GetAnchor(qounterGroup.Anchor);
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
inline void SetSourceOptions(Component::OptionsTypes& options, UnparsedJSON newOptions) {
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

UnityEngine::Transform* GetCanvas(std::string parentName) {
    static ConstString name("QountersCanvas");

    if (!UnityEngine::GameObject::Find(parentName))
        getLogger().error("Failed to find parent %s!", parentName.c_str());
    // TODO: fallback, set world pos without parent

    auto parent = UnityEngine::GameObject::Find(parentName)->get_transform();
    if (auto ret = parent->Find(name))
        return ret;

    auto canvas = BeatSaberUI::CreateCanvas();
    canvas->set_name(name);

    canvas->GetComponent<UnityEngine::Canvas*>()->set_sortingOrder(0);

    auto ret = canvas->get_transform();
    ret->set_localScale({0.02, 0.02, 0.02});
    ret->SetParent(parent, true);
    ret->set_localPosition({});
    ret->set_localEulerAngles({});

    return ret;
}

UnityEngine::Transform* GetTopAnchor() {
    static ConstString name("QountersTopPanel");
    static ConstString canvasName("QountersCanvas");

    if (auto ret = UnityEngine::GameObject::Find(name))
        return ret->get_transform()->Find(canvasName);

    getLogger().debug("Creating top anchor");
    auto hud = UnityEngine::GameObject::Find("LeftPanel")->get_transform()->get_parent();

    auto parent = UnityEngine::GameObject::New_ctor(name)->get_transform();
    parent->set_position({0, 3, 7});
    parent->SetParent(hud, false);

    auto canvas = BeatSaberUI::CreateCanvas();
    canvas->set_name(canvasName);

    canvas->GetComponent<UnityEngine::Canvas*>()->set_sortingOrder(0);

    auto ret = canvas->get_transform();
    ret->set_localScale({0.02, 0.02, 0.02});
    ret->SetParent(parent, true);
    ret->set_localPosition({0, 0, 0});
    ret->set_localEulerAngles({0, 0, 0});

    return ret;
}

UnityEngine::Transform* Qounters::GetAnchor(int anchor) {
    UnityEngine::RectTransform* ret = nullptr;
    switch ((Group::Anchors) anchor) {
        case Group::Anchors::Left:
            ret = (UnityEngine::RectTransform*) GetCanvas("LeftPanel");
            ret->set_sizeDelta({50, 125});
            ret->set_anchoredPosition({0, 0.75});
            break;
        case Group::Anchors::Right:
            ret = (UnityEngine::RectTransform*) GetCanvas("RightPanel");
            ret->set_sizeDelta({50, 125});
            ret->set_anchoredPosition({0, 0.75});
            break;
        case Group::Anchors::Bottom:
            ret = (UnityEngine::RectTransform*) GetCanvas("EnergyPanel");
            ret->set_sizeDelta({-25, 25});
            ret->set_anchoredPosition({0, -10});
            break;
        case Group::Anchors::Top:
            ret = (UnityEngine::RectTransform*) GetTopAnchor();
            ret->set_sizeDelta({125, 50});
            break;
    }
    if (!ret)
        getLogger().error("Qounter group anchor %i was not in enum", anchor);
    return ret;
}

void Qounters::CreateQounterComponent(Component const& qounterComponent, int componentIdx, UnityEngine::Transform* parent, bool editing) {
    getLogger().debug("Creating qounter component of type %i", qounterComponent.Type);

    UnityEngine::UI::Graphic* component;

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

    UpdateComponentPosition(component->get_rectTransform(), qounterComponent);

    if (editing) {
        auto edit = component->get_gameObject()->AddComponent<EditingComponent*>();
        edit->Init(component, componentIdx);
        Editor::RegisterEditingComponent(edit, edit->GetEditingGroup()->GetGroupIdx(), componentIdx);
    }
}

void Qounters::CreateQounterGroup(Group const& qounterGroup, int groupIdx, bool editing) {
    getLogger().debug("Creating qounter group");

    auto parent = UnityEngine::GameObject::New_ctor("QounterGroup");
    auto parentTransform = parent->AddComponent<UnityEngine::RectTransform*>();

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

    BaseGameGraphic::MakeClones();

    for (int i = 0; i <= (int) Group::Anchors::AnchorMax; i++)
        Utils::DisableAllBut(GetAnchor(i)->get_parent(), {"QountersCanvas", "EnergyPanel"});
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
    }
}
