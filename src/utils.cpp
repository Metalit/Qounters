#include "utils.hpp"

#include <iomanip>
#include <sstream>

#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "HMUI/AnimatedSwitchView.hpp"
#include "HMUI/ButtonBinder.hpp"
#include "HMUI/ScrollView.hpp"
#include "System/Action.hpp"
#include "UnityEngine/EventSystems/EventSystem.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/UI/LayoutRebuilder.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/Components/ScrollViewContent.hpp"
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

void Utils::InstantSetToggle(BSML::ToggleSetting* toggle, bool value) {
    if (toggle->toggle->m_IsOn == value)
        return;
    toggle->toggle->m_IsOn = value;
    auto animatedSwitch = toggle->GetComponent<HMUI::AnimatedSwitchView*>();
    animatedSwitch->HandleOnValueChanged(value);
    animatedSwitch->_switchAmount = value;
    animatedSwitch->LerpPosition(value);
    animatedSwitch->LerpColors(value, animatedSwitch->_highlightAmount, animatedSwitch->_disabledAmount);
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

void Utils::AddSliderEndDrag(BSML::SliderSetting* slider, std::function<void(float)> onEndDrag) {
    std::function<void()> boundCallback = [slider, onEndDrag]() {
        onEndDrag(slider->slider->value);
    };
    GetOrAddComponent<EndDragHandler*>(slider->slider)->callback = boundCallback;
}

void Utils::AddStringSettingOnClose(HMUI::InputFieldView* input, std::function<void(std::string)> onKeyboardClosed) {
    std::function<void()> boundCallback = [input, onKeyboardClosed]() {
        onKeyboardClosed(input->text);
    };
    GetOrAddComponent<KeyboardCloseHandler*>(input)->closeCallback = boundCallback;
    input->_buttonBinder->AddBinding(input->_clearSearchButton, BSML::MakeSystemAction(boundCallback));
}

void Utils::AddIncrementIncrement(BSML::IncrementSetting* setting, float increment) {
    auto transform = setting->get_transform()->Find("ValuePicker").cast<UnityEngine::RectTransform>();
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

UnityEngine::RectTransform* GetScrollViewTop(UnityEngine::GameObject* scrollView) {
    return scrollView->transform->parent->parent->parent->GetComponent<UnityEngine::RectTransform*>();
}

void Utils::FixScrollView(UnityEngine::GameObject* scrollView, float width) {
    UnityEngine::Object::Destroy(scrollView->GetComponentInParent<BSML::ScrollViewContent*>(true));
    scrollView->GetComponent<UnityEngine::UI::VerticalLayoutGroup*>()->spacing = 0;
    GetScrollViewTop(scrollView)->sizeDelta = {width - 100, 0};
    auto transform = scrollView->GetComponent<UnityEngine::RectTransform*>();
    transform->sizeDelta = {width, 74};
    SetChildrenWidth(transform, width);
    auto content = transform->parent.cast<UnityEngine::RectTransform>();
    content->sizeDelta = {0, 74};
}

void Utils::SetScrollViewActive(UnityEngine::GameObject* scrollView, bool active) {
    GetScrollViewTop(scrollView)->gameObject->active = active;
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
