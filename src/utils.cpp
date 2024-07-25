#include "utils.hpp"

#include <iomanip>
#include <sstream>

#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "HMUI/AnimatedSwitchView.hpp"
#include "HMUI/ButtonBinder.hpp"
#include "HMUI/ScrollView.hpp"
#include "System/Action.hpp"
#include "UnityEngine/CanvasGroup.hpp"
#include "UnityEngine/EventSystems/EventSystem.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/UI/LayoutRebuilder.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/Helpers/delegates.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "customtypes/editing.hpp"
#include "customtypes/settings.hpp"
#include "main.hpp"

using namespace GlobalNamespace;
using namespace Qounters;

std::string Utils::FormatDecimals(double num, int decimals) {
    if (decimals < 0)
        decimals = 0;
    std::stringstream stream;
    stream << std::fixed << std::setprecision(decimals) << num;
    return stream.str();
}

std::string Utils::SecondsToString(int value) {
    int minutes = value / 60;
    int seconds = value - minutes * 60;

    std::string minutesString = std::to_string(minutes);
    std::string secondsString = std::to_string(seconds);
    if (seconds < 10)
        secondsString = "0" + secondsString;

    return minutesString + ":" + secondsString;
}

std::tuple<std::string, std::string, int> Utils::GetBeatmapDetails(BeatmapKey beatmap) {
    std::string id = beatmap.levelId;
    std::string characteristic = beatmap.beatmapCharacteristic->serializedName;
    int difficulty = (int) beatmap.difficulty;
    return {id, characteristic, difficulty};
}

std::string Utils::GetBeatmapIdentifier(BeatmapKey beatmap) {
    if (!beatmap.IsValid())
        return "Unknown";
    auto [id, characteristic, difficulty] = GetBeatmapDetails(beatmap);
    return fmt::format("{}_{}_{}", id, characteristic, difficulty);
}

void DisableAllBut(UnityEngine::Transform* original, UnityEngine::Transform* source, std::set<std::string> enabled, std::set<std::string> disabled) {
    for (int i = 0; i < source->GetChildCount(); i++) {
        auto child = source->GetChild(i).unsafePtr();
        std::string name = child->name;
        if (enabled.contains(name)) {
            auto loopback = child;
            while (loopback != original) {
                loopback->gameObject->active = true;
                loopback = loopback->parent;
            }
        } else {
            child->gameObject->active = false;
            if (!disabled.contains(name))
                DisableAllBut(original, child, enabled, disabled);
        }
    }
}

void Utils::DisableAllBut(UnityEngine::Transform* parent, std::set<std::string> enabled, std::set<std::string> disabled) {
    if (!enabled.contains(parent->name))
        DisableAllBut(parent, parent, enabled, disabled);
}

UnityEngine::Transform* Utils::FindRecursive(UnityEngine::Transform* parent, std::string name) {
    for (int i = 0; i < parent->GetChildCount(); i++) {
        auto child = parent->GetChild(i);
        if (child->name == name)
            return child;
    }
    // breadth first
    for (int i = 0; i < parent->GetChildCount(); i++) {
        if (auto ret = FindRecursive(parent->GetChild(i), name))
            return ret;
    }
    return nullptr;
}

std::string Utils::GetTransformPath(UnityEngine::Transform* parent, UnityEngine::Transform* child) {
    if (parent == child || !child->IsChildOf(parent))
        return "";
    return GetTransformPath(parent, child->parent) + "/" + static_cast<std::string>(child->name);
}

void Utils::SetLayoutSize(UnityEngine::Component* object, int width, int height) {
    auto layout = object->GetComponent<UnityEngine::UI::LayoutElement*>();
    layout->preferredWidth = width;
    layout->preferredHeight = height;
}

void Utils::SetChildrenWidth(UnityEngine::Transform* parent, float width) {
    for (int i = 0; i < parent->GetChildCount(); i++) {
        bool first = true;
        for (auto layout : parent->GetChild(i)->GetComponents<UnityEngine::UI::LayoutElement*>()) {
            if (first)
                layout->preferredWidth = width;
            else
                UnityEngine::Object::Destroy(layout);
            first = false;
        }
    }
}

void Utils::SetCanvasSorting(UnityEngine::GameObject* canvas, int value) {
    auto comp = canvas->GetComponent<UnityEngine::Canvas*>();
    comp->overrideSorting = true;
    comp->sortingOrder = value;
}

void Utils::InstantSetToggle(BSML::ToggleSetting* toggle, bool value) {
    if (toggle->toggle->m_IsOn == value)
        return;
    toggle->toggle->m_IsOn = value;
    auto animatedSwitch = toggle->toggle->GetComponent<HMUI::AnimatedSwitchView*>();
    animatedSwitch->HandleOnValueChanged(value);
    animatedSwitch->_switchAmount = value;
    animatedSwitch->LerpPosition(value);
    animatedSwitch->LerpColors(value, animatedSwitch->_highlightAmount, animatedSwitch->_disabledAmount);
}

void Utils::SetIncrementValue(BSML::IncrementSetting* increment, float value) {
    increment->currentValue = value;
    increment->UpdateState();
}

void Utils::SetDropdownValue(BSML::DropdownListSetting* dropdown, std::string value) {
    auto values = ListW<StringW>(dropdown->values);
    for (int i = 0; i < values.size(); i++) {
        if (values[i] == value) {
            dropdown->set_Value(dropdown->values[i]);
            break;
        }
    }
}

void Utils::SetIconButtonSprite(UnityEngine::UI::Button* button, UnityEngine::Sprite* sprite) {
    auto icon = button->transform->Find("QountersButtonImage");
    if (!icon)
        return;
    icon->GetComponent<HMUI::ImageView*>()->sprite = sprite;
}

UnityEngine::UI::Button* Utils::CreateIconButton(UnityEngine::GameObject* parent, UnityEngine::Sprite* sprite, std::function<void()> onClick) {
    auto button = BSML::Lite::CreateUIButton(parent, "", onClick);
    auto icon = BSML::Lite::CreateImage(button, sprite);
    icon->name = "QountersButtonImage";
    icon->preserveAspect = true;
    icon->transform->localScale = {0.8, 0.8, 0.8};
    SetLayoutSize(button, 8, 8);
    return button;
}

BSML::DropdownListSetting* Utils::CreateDropdown(
    UnityEngine::GameObject* parent,
    std::string name,
    std::string value,
    std::vector<std::string_view> values,
    std::function<void(std::string)> onChange
) {
    auto object = BSML::Lite::CreateDropdown(parent, name, value, values, [onChange](StringW value) { onChange(value); });
    object->transform->parent->GetComponent<UnityEngine::UI::LayoutElement*>()->preferredHeight = 7;
    return object;
}

BSML::DropdownListSetting* Utils::CreateDropdownEnum(
    UnityEngine::GameObject* parent, std::string name, int value, std::vector<std::string_view> values, std::function<void(int)> onChange
) {
    auto object = BSML::Lite::CreateDropdown(parent, name, values[value], values, [onChange, values](StringW value) {
        for (int i = 0; i < values.size(); i++) {
            if (value == values[i]) {
                onChange(i);
                break;
            }
        }
    });
    object->transform->parent->GetComponent<UnityEngine::UI::LayoutElement*>()->preferredHeight = 7;
    if (auto behindModal = parent->GetComponentInParent<HMUI::ModalView*>(true))
        AddModalAnimations(object->dropdown, behindModal);
    return object;
}

BSML::ColorSetting* Utils::CreateColorPicker(
    UnityEngine::GameObject* parent,
    std::string name,
    UnityEngine::Color value,
    std::function<void(UnityEngine::Color)> onChange,
    std::function<void()> onClose
) {
    auto ret = BSML::Lite::CreateColorPicker(parent, name, value);
    ret->modalColorPicker->onChange = [ret, onChange](UnityEngine::Color val) {
        ret->set_currentColor(val);
        onChange(val);
    };
    auto modal = ret->modalColorPicker->GetComponent<UnityEngine::RectTransform*>();
    modal->Find("BSMLHSVPanel/ColorPickerButtonPrimary")->gameObject->active = false;
    modal->Find("BSMLHorizontalLayoutGroup")->gameObject->active = false;
    modal->sizeDelta = {50, 70};
    auto rgb = ret->modalColorPicker->rgbPanel->GetComponent<UnityEngine::RectTransform*>();
    auto wheel = ret->modalColorPicker->hsvPanel->GetComponent<UnityEngine::RectTransform*>();
    auto preview = ret->modalColorPicker->colorImage->GetComponent<UnityEngine::RectTransform*>();
    rgb->localScale = {0.75, 0.75, 0.75};
    rgb->anchorMin = {0.5, 0.5};
    rgb->anchorMax = {0.5, 0.5};
    rgb->anchoredPosition = {-24, 10};
    wheel->localScale = {0.9, 0.9, 0.9};
    wheel->anchorMin = {0.5, 0.5};
    wheel->anchorMax = {0.5, 0.5};
    wheel->anchoredPosition = {0, -10};
    preview->anchoredPosition = {17, -27};
    auto modalView = ret->modalColorPicker->modalView;
    modalView->add_blockerClickedEvent(BSML::MakeSystemAction([ret, modalView, onClose]() {
        modalView->Hide();
        ret->currentColor = ret->modalColorPicker->currentColor;
        onClose();
    }));
    return ret;
}

CollapseController*
Utils::CreateCollapseArea(UnityEngine::GameObject* parent, std::string title, bool open, std::vector<UnityEngine::Component*> contents) {
    auto layout = BSML::Lite::CreateHorizontalLayoutGroup(parent);
    layout->spacing = 2;
    layout->childAlignment = UnityEngine::TextAnchor::MiddleCenter;
    layout->childForceExpandHeight = false;
    // makes the whole area clickable
    layout->gameObject->AddComponent<CanvasHighlight*>();
    auto ret = layout->gameObject->AddComponent<CollapseController*>();
    ret->open = open;
    ret->title = title;
    ret->text = BSML::Lite::CreateText(layout, "", TMPro::FontStyles::Normal, 3.5);
    ret->line = BSML::Lite::CreateImage(layout, BSML::Utilities::ImageResources::GetWhitePixel());
    auto dims = ret->line->GetComponent<UnityEngine::UI::LayoutElement*>();
    dims->preferredWidth = 0;
    dims->preferredHeight = 0.4;
    dims->flexibleWidth = 999;
    ret->contents = contents;
    ret->UpdateOpen();
    // update colors
    ret->OnPointerExit(nullptr);
    return ret;
}

MenuDragger* Utils::CreateMenuDragger(UnityEngine::GameObject* parent, bool isLeftMenu) {
    auto padding = BSML::Lite::CreateCanvas();
    padding->name = "QountersMenuDragger";
    padding->AddComponent<CanvasHighlight*>();
    auto rect = padding->GetComponent<UnityEngine::RectTransform*>();
    rect->SetParent(parent->transform, false);
    rect->localScale = {1, 1, 1};
    rect->anchorMin = {0.5, 1};
    rect->anchorMax = {0.5, 1};
    rect->sizeDelta = {42, 3};
    SetCanvasSorting(padding, 5);
    auto drag = BSML::Lite::CreateCanvas();
    drag->name = "QountersMenuDragCanvas";
    drag->AddComponent<CanvasHighlight*>();
    auto dragRect = drag->GetComponent<UnityEngine::RectTransform*>();
    dragRect->SetParent(rect, false);
    dragRect->localScale = {1, 1, 1};
    dragRect->sizeDelta = {1000, 1000};
    drag->active = false;
    auto ret = padding->AddComponent<MenuDragger*>();
    ret->dragCanvas = drag;
    ret->menu = parent->GetComponent<UnityEngine::RectTransform*>();
    ret->line = BSML::Lite::CreateImage(padding, BSML::Utilities::ImageResources::GetWhitePixel());
    auto img = ret->line->rectTransform;
    img->anchorMin = {0.5, 0.5};
    img->anchorMax = {0.5, 0.5};
    img->sizeDelta = {40, 1};
    ret->isLeftMenu = isLeftMenu;
    ret->OnEnable();
    return ret;
}

void AnimateModal(HMUI::ModalView* modal, bool out) {
    auto bg = modal->transform->Find("BG")->GetComponent<UnityEngine::UI::Image*>();
    auto canvas = modal->GetComponent<UnityEngine::CanvasGroup*>();

    if (out) {
        bg->color = {0.2, 0.2, 0.2, 1};
        canvas->alpha = 0.9;
    } else {
        bg->color = {1, 1, 1, 1};
        canvas->alpha = 1;
    }
}

void Utils::AddModalAnimations(HMUI::SimpleTextDropdown* dropdown, HMUI::ModalView* behindModal) {
    dropdown->_button->onClick->AddListener(BSML::MakeUnityAction([behindModal]() { AnimateModal(behindModal, true); }));
    dropdown->add_didSelectCellWithIdxEvent(BSML::MakeSystemAction(
        (std::function<void(UnityW<HMUI::DropdownWithTableView>, int)>) [behindModal](auto, int) { AnimateModal(behindModal, false); }
    ));
    dropdown->_modalView->add_blockerClickedEvent(BSML::MakeSystemAction([behindModal]() { AnimateModal(behindModal, false); }));
    dropdown->_modalView->_animateParentCanvas = false;
}

void Utils::AddSliderEndDrag(BSML::SliderSetting* slider, std::function<void(float)> onEndDrag) {
    std::function<void()> boundCallback = [slider = slider->slider, onEndDrag]() {
        onEndDrag(slider->value);
    };
    GetOrAddComponent<EndDragHandler*>(slider->slider)->callback = boundCallback;
    if (slider->showButtons && slider->incButton && slider->decButton) {
        slider->incButton->onClick->AddListener(BSML::MakeUnityAction(boundCallback));
        slider->decButton->onClick->AddListener(BSML::MakeUnityAction(boundCallback));
    }
}

void Utils::AddStringSettingOnClose(HMUI::InputFieldView* input, std::function<void(std::string)> onKeyboardClosed) {
    std::function<void()> boundCallback = [input, onKeyboardClosed]() {
        onKeyboardClosed(input->text);
    };
    GetOrAddComponent<KeyboardCloseHandler*>(input)->closeCallback = boundCallback;
    input->_buttonBinder->AddBinding(input->_clearSearchButton, BSML::MakeSystemAction(boundCallback));
}

void Utils::AddIncrementIncrement(BSML::IncrementSetting* setting, float increment) {
    auto transform = setting->transform->Find("ValuePicker").cast<UnityEngine::RectTransform>();
    transform->anchoredPosition = {-6, 0};

    auto leftButton = BSML::Lite::CreateUIButton(transform, "", "DecButton", {-20, 0}, {6, 8}, [setting, increment]() {
        setting->currentValue -= increment;
        setting->EitherPressed();
    });
    auto rightButton = BSML::Lite::CreateUIButton(transform, "", "IncButton", {7, 0}, {8, 8}, [setting, increment]() {
        setting->currentValue += increment;
        setting->EitherPressed();
    });
}

BSML::SliderSetting* Utils::ReparentSlider(BSML::SliderSetting* slider, BSML::Lite::TransformWrapper parent, float width) {
    auto newSlider = slider->slider->gameObject->AddComponent<BSML::SliderSetting*>();
    newSlider->slider = slider->slider;
    newSlider->onChange = std::move(slider->onChange);
    newSlider->formatter = std::move(slider->formatter);
    newSlider->isInt = slider->isInt;
    newSlider->increments = slider->increments;
    newSlider->slider->minValue = slider->slider->minValue;
    newSlider->slider->maxValue = slider->slider->maxValue;
    auto transform = newSlider->GetComponent<UnityEngine::RectTransform*>();
    transform->sizeDelta = {width, 0};
    transform->SetParent(parent->transform, false);
    newSlider->slider->valueSize = newSlider->slider->_containerRect->rect.width / 2;
    UnityEngine::Object::DestroyImmediate(slider->gameObject);
    // due to the weird way bsml does string formatting for sliders,
    // this needs to be called after destroying the old slider
    newSlider->Setup();
    return newSlider;
}

void Utils::RebuildWithScrollPosition(UnityEngine::GameObject* scrollView) {
    auto scrollComponent = GetScrollViewTop(scrollView)->GetComponent<HMUI::ScrollView*>();
    auto scroll = scrollComponent->position;
    // ew
    UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(scrollComponent->_contentRectTransform);
    UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(scrollComponent->_contentRectTransform);
    scrollComponent->UpdateContentSize();
    scrollComponent->ScrollTo(std::min(scroll, scrollComponent->scrollableSize), false);
}

UnityEngine::RectTransform* Utils::GetScrollViewTop(UnityEngine::GameObject* scrollView) {
    return scrollView->transform->parent->parent->parent->GetComponent<UnityEngine::RectTransform*>();
}

VRUIControls::VRInputModule* Utils::GetCurrentInputModule() {
    auto eventSystem = UnityEngine::EventSystems::EventSystem::get_current();
    if (!eventSystem) {
        auto eventSystems = UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::EventSystems::EventSystem*>();
        eventSystem = eventSystems->FirstOrDefault([](auto e) { return e->isActiveAndEnabled; });
        if (!eventSystem)
            eventSystem = eventSystems->FirstOrDefault();
    }
    if (!eventSystem)
        return nullptr;
    return eventSystem->GetComponent<VRUIControls::VRInputModule*>();
}
