#include "customtypes/settings.hpp"
#include "main.hpp"
#include "utils.hpp"

#include "bsml/shared/BSML/Components/ScrollViewContent.hpp"

using namespace GlobalNamespace;
using namespace Qounters;

#include <sstream>
#include <iomanip>

std::string Qounters::Utils::FormatDecimals(double num, int decimals) {
    if (decimals < 0) decimals = 0;
    std::stringstream stream;
    stream << std::fixed << std::setprecision(decimals) << num;
    return stream.str();
}

std::string Qounters::Utils::SecondsToString(int value) {
    int minutes = value / 60;
    int seconds = value - minutes * 60;

    std::string minutesString = std::to_string(minutes);
    std::string secondsString = std::to_string(seconds);
    if (seconds < 10)
        secondsString = "0" + secondsString;

    return minutesString + ":" + secondsString;
}

#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IDifficultyBeatmapSet.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"

std::tuple<std::string, std::string, int> Qounters::Utils::GetBeatmapDetails(IDifficultyBeatmap* beatmap) {
    std::string id = beatmap->get_level()->i___GlobalNamespace__IPreviewBeatmapLevel()->get_levelID();
    std::string characteristic = beatmap->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName;
    int difficulty = beatmap->get_difficulty().value__;
    return {id, characteristic, difficulty};
}

std::string Qounters::Utils::GetBeatmapIdentifier(IDifficultyBeatmap* beatmap) {
    if (!beatmap)
        return "Unknown";
    auto [id, characteristic, difficulty] = GetBeatmapDetails(beatmap);
    return string_format("%s_%s_%i", id.c_str(), characteristic.c_str(), difficulty);
}

#include "UnityEngine/GameObject.hpp"

void DisableAllBut(UnityEngine::Transform* original, UnityEngine::Transform* source, std::set<std::string> enabled, std::set<std::string> disabled) {
    for (int i = 0; i < source->GetChildCount(); i++) {
        auto child = source->GetChild(i);
        std::string name = child->get_name();
        if (enabled.contains(name)) {
            auto loopback = child;
            while (loopback.ptr() != original) {
                loopback->get_gameObject()->SetActive(true);
                loopback = loopback->get_parent();
            }
        } else {
            child->get_gameObject()->SetActive(false);
            if (!disabled.contains(name))
                DisableAllBut(original, child, enabled, disabled);
        }
    }
}

void Qounters::Utils::DisableAllBut(UnityEngine::Transform* parent, std::set<std::string> enabled, std::set<std::string> disabled) {
    if (!enabled.contains(parent->get_name()))
        DisableAllBut(parent, parent, enabled, disabled);
}

UnityEngine::Transform* Qounters::Utils::FindRecursive(UnityEngine::Transform* parent, std::string name) {
    for (int i = 0; i < parent->GetChildCount(); i++) {
        auto child = parent->GetChild(i);
        if (child->get_name() == name)
            return child;
    }
    // breadth first
    for (int i = 0; i < parent->GetChildCount(); i++) {
        if (auto ret = FindRecursive(parent->GetChild(i), name))
            return ret;
    }
    return nullptr;
}

std::string Qounters::Utils::GetTransformPath(UnityEngine::Transform* parent, UnityEngine::Transform* child) {
    if (parent == child || !child->IsChildOf(parent))
        return "";
    return GetTransformPath(parent, child->get_parent()) + "/" + static_cast<std::string>(child->get_name());
}

#include "HMUI/AnimatedSwitchView.hpp"

void Qounters::Utils::InstantSetToggle(UnityEngine::UI::Toggle* toggle, bool value) {
    if(toggle->m_IsOn == value)
        return;
    toggle->m_IsOn = value;
    auto animatedSwitch = toggle->GetComponent<HMUI::AnimatedSwitchView*>();
    animatedSwitch->HandleOnValueChanged(value);
    animatedSwitch->_switchAmount = value;
    animatedSwitch->LerpPosition(value);
    animatedSwitch->LerpColors(value, animatedSwitch->_highlightAmount, animatedSwitch->_disabledAmount);
}

void Qounters::Utils::SetDropdownValue(HMUI::SimpleTextDropdown* dropdown, std::string value) {
    auto values = ListW<StringW>(dropdown->_texts);
    for (int i = 0; i < values.size(); i++) {
        if (values[i] == value) {
            dropdown->SelectCellWithIdx(i);
            break;
        }
    }
}

#include "bsml/shared/BSML-Lite.hpp"

HMUI::SimpleTextDropdown* Qounters::Utils::CreateDropdown(UnityEngine::GameObject* parent, std::string name, std::string value, std::vector<std::string_view> values, std::function<void (std::string)> onChange) {
    auto object = BSML::Lite::CreateDropdown(parent, name, value, values, [onChange](StringW value) {
        onChange(value);
    });
    object->get_transform()->GetParent()->GetComponent<UnityEngine::UI::LayoutElement*>()->set_preferredHeight(7);
    return object->dropdown;
}

HMUI::SimpleTextDropdown* Qounters::Utils::CreateDropdownEnum(UnityEngine::GameObject* parent, std::string name, int value, std::vector<std::string_view> values, std::function<void (int)> onChange) {
    std::vector<StringW> dropdownStringWs(values.begin(), values.end());
    auto object = BSML::Lite::CreateDropdown(parent, name, dropdownStringWs[value], values, [onChange, values](StringW value) {
        for(int i = 0; i < values.size(); i++) {
            if(value == values[i]) {
                onChange(i);
                break;
            }
        }
    });
    object->get_transform()->GetParent()->GetComponent<UnityEngine::UI::LayoutElement*>()->set_preferredHeight(7);
    return object->dropdown;
}

#include "custom-types/shared/delegate.hpp"

#include "System/Action.hpp"

BSML::ColorSetting* Qounters::Utils::CreateColorPicker(UnityEngine::GameObject* parent, std::string name, UnityEngine::Color value, std::function<void (UnityEngine::Color)> onChange, std::function<void ()> onClose) {
    auto ret = BSML::Lite::CreateColorPicker(parent->get_transform(), name, value);
    ret->modalColorPicker->onChange = [ret, onChange](UnityEngine::Color val) {
        ret->set_currentColor(ret->modalColorPicker->currentColor);
        onChange(val);
    };
    auto modal = ret->modalColorPicker->get_transform().try_cast<UnityEngine::RectTransform>().value_or(nullptr);
    modal->Find("BSMLHSVPanel/ColorPickerButtonPrimary")->get_gameObject()->SetActive(false);
    modal->Find("BSMLHorizontalLayoutGroup")->get_gameObject()->SetActive(false);
    modal->set_sizeDelta({50, 70});
    auto rgb = ret->modalColorPicker->rgbPanel->get_transform().try_cast<UnityEngine::RectTransform>().value_or(nullptr);
    auto wheel = ret->modalColorPicker->hsvPanel->get_transform().try_cast<UnityEngine::RectTransform>().value_or(nullptr);
    auto preview = ret->modalColorPicker->colorImage->get_transform().try_cast<UnityEngine::RectTransform>().value_or(nullptr);
    rgb->set_localScale({0.75, 0.75, 0.75});
    rgb->set_anchorMin({0.5, 0.5});
    rgb->set_anchorMax({0.5, 0.5});
    rgb->set_anchoredPosition({-24, 10});
    wheel->set_localScale({0.9, 0.9, 0.9});
    wheel->set_anchorMin({0.5, 0.5});
    wheel->set_anchorMax({0.5, 0.5});
    wheel->set_anchoredPosition({0, -10});
    preview->set_anchoredPosition({17, -27});
    auto modalView = ret->modalColorPicker->modalView;
    modalView->add_blockerClickedEvent(custom_types::MakeDelegate<System::Action*>((std::function<void ()>) [ret, modalView, onClose]() {
        modalView->Hide();
        ret->set_currentColor(ret->modalColorPicker->currentColor);
        onClose();
    }));
    return ret;
}

void Qounters::Utils::AddSliderEndDrag(BSML::SliderSetting* slider, std::function<void (float)> onEndDrag) {
    std::function<void ()> boundCallback = [slider, onEndDrag]() { onEndDrag(slider->slider->get_value()); };
    GetOrAddComponent<EndDragHandler*>(slider->slider)->callback = boundCallback;
}

#include "HMUI/ButtonBinder.hpp"

void Qounters::Utils::AddStringSettingOnClose(HMUI::InputFieldView* input, std::function<void (std::string)> onKeyboardClosed) {
    std::function<void ()> boundCallback = [input, onKeyboardClosed]() { onKeyboardClosed(input->get_text()); };
    GetOrAddComponent<KeyboardCloseHandler*>(input)->closeCallback = boundCallback;
    input->_buttonBinder->AddBinding(input->_clearSearchButton, custom_types::MakeDelegate<System::Action*>(boundCallback));
}

void Qounters::Utils::AddIncrementIncrement(BSML::IncrementSetting* setting, float increment) {
    auto transform = setting->get_transform()->Find("ValuePicker").try_cast<UnityEngine::RectTransform>().value_or(nullptr);
    transform->set_anchoredPosition({-6, 0});

    auto leftButton = BSML::Lite::CreateUIButton(transform, "", "DecButton", {-22, 0}, {6, 8}, [setting, increment](){
        setting->currentValue -= increment;
        setting->UpdateState();
    });
    auto rightButton = BSML::Lite::CreateUIButton(transform, "", "IncButton", {22, 0}, {8, 8}, [setting, increment](){
        setting->currentValue += increment;
        setting->UpdateState();
    });
}

void Qounters::Utils::SetChildrenWidth(UnityEngine::Transform* parent, float width) {
    for (int i = 0; i < parent->GetChildCount(); i++) {
        bool first = true;
        for (auto layout : parent->GetChild(i)->GetComponents<UnityEngine::UI::LayoutElement*>()) {
            if (first)
                layout->set_preferredWidth(width);
            else
                UnityEngine::Object::Destroy(layout);
            first = false;
        }
    }
}

UnityEngine::Transform* GetScrollViewTop(UnityEngine::GameObject* scrollView) {
    return scrollView->get_transform()->GetParent()->GetParent()->GetParent();
}

#include "UnityEngine/UI/LayoutRebuilder.hpp"

void Qounters::Utils::FixScrollView(UnityEngine::GameObject* scrollView, float width) {
    UnityEngine::Object::Destroy(scrollView->GetComponentsInParent(csTypeOf(BSML::ScrollViewContent*), true)->First());
    scrollView->GetComponent<UnityEngine::UI::VerticalLayoutGroup*>()->set_spacing(0);
    GetScrollViewTop(scrollView)->GetComponent<UnityEngine::RectTransform*>()->set_sizeDelta({width - 100, 0});
    auto transform = scrollView->GetComponent<UnityEngine::RectTransform*>();
    transform->set_sizeDelta({width, 74});
    SetChildrenWidth(transform, width);
    auto content = transform->GetParent()->GetComponent<UnityEngine::RectTransform*>();
    content->set_sizeDelta({0, 74});
}

void Qounters::Utils::SetScrollViewActive(UnityEngine::GameObject* scrollView, bool active) {
    GetScrollViewTop(scrollView)->get_gameObject()->SetActive(active);
}

#include "HMUI/ScrollView.hpp"

void Qounters::Utils::RebuildWithScrollPosition(UnityEngine::GameObject* scrollView) {
    auto scrollComponent = GetScrollViewTop(scrollView)->GetComponent<HMUI::ScrollView*>();
    auto scroll = scrollComponent->get_position();
    // ew
    UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(scrollComponent->_contentRectTransform);
    UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(scrollComponent->_contentRectTransform);
    scrollComponent->UpdateContentSize();
    scrollComponent->ScrollTo(std::min(scroll, scrollComponent->get_scrollableSize()), false);
}