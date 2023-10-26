#include "customtypes/settings.hpp"
#include "main.hpp"
#include "utils.hpp"

using namespace GlobalNamespace;
using namespace Qounters;

#include <sstream>
#include <iomanip>

std::string Qounters::Utils::FormatDecimals(double num, int decimals) {
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

std::tuple<std::string, std::string, int> Qounters::Utils::GetBeatmapDetails(IDifficultyBeatmap* beatmap) {
    std::string id = beatmap->get_level()->i_IPreviewBeatmapLevel()->get_levelID();
    std::string characteristic = beatmap->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName;
    int difficulty = beatmap->get_difficulty();
    return {id, characteristic, difficulty};
}

std::string Qounters::Utils::GetBeatmapIdentifier(IDifficultyBeatmap* beatmap) {
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
            while (loopback != original) {
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
    animatedSwitch->switchAmount = value;
    animatedSwitch->LerpPosition(value);
    animatedSwitch->LerpColors(value, animatedSwitch->highlightAmount, animatedSwitch->disabledAmount);
}

void Qounters::Utils::SetDropdownValue(HMUI::SimpleTextDropdown* dropdown, std::string value) {
    auto values = ListW<StringW>(dropdown->texts);
    for (int i = 0; i < values.size(); i++) {
        if (values[i] == value) {
            dropdown->SelectCellWithIdx(i);
            break;
        }
    }
}

#include "questui/shared/BeatSaberUI.hpp"

HMUI::SimpleTextDropdown* Qounters::Utils::CreateDropdown(UnityEngine::GameObject* parent, std::string name, std::string value, std::vector<std::string> values, std::function<void (std::string)> onChange) {
    std::vector<StringW> dropdownStringWs(values.begin(), values.end());
    auto object = QuestUI::BeatSaberUI::CreateDropdown(parent, name, value, dropdownStringWs, [onChange](StringW value) {
        onChange(value);
    });
    object->get_transform()->GetParent()->GetComponent<UnityEngine::UI::LayoutElement*>()->set_preferredHeight(7);
    return object;
}

HMUI::SimpleTextDropdown* Qounters::Utils::CreateDropdownEnum(UnityEngine::GameObject* parent, std::string name, int value, std::vector<std::string> values, std::function<void (int)> onChange) {
    std::vector<StringW> dropdownStringWs(values.begin(), values.end());
    auto object = QuestUI::BeatSaberUI::CreateDropdown(parent, name, dropdownStringWs[value], dropdownStringWs, [onChange, values](StringW value) {
        for(int i = 0; i < values.size(); i++) {
            if(value == values[i]) {
                onChange(i);
                break;
            }
        }
    });
    object->get_transform()->GetParent()->GetComponent<UnityEngine::UI::LayoutElement*>()->set_preferredHeight(7);
    return object;
}

#include "custom-types/shared/delegate.hpp"

#include "System/Action.hpp"

QuestUI::ColorSetting* Qounters::Utils::CreateColorPicker(UnityEngine::GameObject* parent, std::string name, UnityEngine::Color value, std::function<void (UnityEngine::Color)> onChange, std::function<void ()> onClose) {
    auto ret = QuestUI::BeatSaberUI::CreateColorPicker(parent->get_transform(), name, value);
    ret->colorPickerModal->onChange = [ret, onChange](UnityEngine::Color val) {
        ret->set_currentColor(ret->colorPickerModal->get_color());
        onChange(val);
    };
    auto modal = (UnityEngine::RectTransform*) ret->colorPickerModal->get_transform();
    modal->Find("QuestUIHSVPanel/ColorPickerButtonPrimary")->get_gameObject()->SetActive(false);
    modal->Find("QuestUIHorizontalLayoutGroup")->get_gameObject()->SetActive(false);
    modal->set_sizeDelta({50, 70});
    auto rgb = (UnityEngine::RectTransform*) ret->colorPickerModal->rgbPanel->get_transform();
    auto wheel = (UnityEngine::RectTransform*) ret->colorPickerModal->hsvPanel->get_transform();
    auto preview = (UnityEngine::RectTransform*) ret->colorPickerModal->colorImage->get_transform();
    rgb->set_localScale({0.75, 0.75, 0.75});
    rgb->set_anchorMin({0.5, 0.5});
    rgb->set_anchorMax({0.5, 0.5});
    rgb->set_anchoredPosition({-24, 10});
    wheel->set_localScale({0.9, 0.9, 0.9});
    wheel->set_anchorMin({0.5, 0.5});
    wheel->set_anchorMax({0.5, 0.5});
    wheel->set_anchoredPosition({0, -10});
    preview->set_anchoredPosition({17, -27});
    auto modalView = ret->colorPickerModal->modalView;
    modalView->add_blockerClickedEvent(custom_types::MakeDelegate<System::Action*>((std::function<void ()>) [ret, modalView, onClose]() {
        modalView->Hide(true, nullptr);
        ret->set_currentColor(ret->colorPickerModal->get_color());
        onClose();
    }));
    return ret;
}

void Qounters::Utils::AddSliderEndDrag(QuestUI::SliderSetting* slider, std::function<void ()> onEndDrag) {
    slider->slider->get_gameObject()->AddComponent<Qounters::EndDragHandler*>()->callback = onEndDrag;
}

void Qounters::Utils::AddIncrementIncrement(QuestUI::IncrementSetting* setting, float increment) {
    auto transform = (UnityEngine::RectTransform*) setting->get_transform()->Find("ValuePicker");
    transform->set_anchoredPosition({-6, 0});

    auto leftButton = QuestUI::BeatSaberUI::CreateUIButton(transform, "", "DecButton", {-22, 0}, {6, 8}, [setting, increment](){
        setting->CurrentValue -= increment;
        setting->UpdateValue();
    });
    auto rightButton = QuestUI::BeatSaberUI::CreateUIButton(transform, "", "IncButton", {22, 0}, {8, 8}, [setting, increment](){
        setting->CurrentValue += increment;
        setting->UpdateValue();
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

#include "questui/shared/CustomTypes/Components/ExternalComponents.hpp"

UnityEngine::Transform* GetScrollViewTop(UnityEngine::GameObject* scrollView) {
    return scrollView->get_transform()->GetParent()->GetParent()->GetParent();
}

#include "UnityEngine/UI/LayoutRebuilder.hpp"

void Qounters::Utils::FixScrollView(UnityEngine::GameObject* scrollView, float width) {
    UnityEngine::Object::Destroy(scrollView->GetComponentInParent(il2cpp_utils::GetSystemType(il2cpp_utils::GetClassFromName("QuestUI", "ScrollViewContent"))));
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
    UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(scrollComponent->contentRectTransform);
    UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(scrollComponent->contentRectTransform);
    scrollComponent->UpdateContentSize();
    scrollComponent->ScrollTo(std::min(scroll, scrollComponent->get_scrollableSize()), false);
}
