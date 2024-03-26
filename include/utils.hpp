#pragma once

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "HMUI/SimpleTextDropdown.hpp"
#include "HMUI/InputFieldView.hpp"
#include "bsml/shared/BSML/Components/Settings/ColorSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/IncrementSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/SliderSetting.hpp"

#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"

namespace Qounters::Utils {
    std::string FormatDecimals(double num, int decimals);
    std::string SecondsToString(int seconds);
    std::tuple<std::string, std::string, int> GetBeatmapDetails( GlobalNamespace::BeatmapKey key);
    std::string GetBeatmapIdentifier(GlobalNamespace::BeatmapKey key);
    void DisableAllBut(UnityEngine::Transform* parent, std::set<std::string> enabled, std::set<std::string> disabled = {});
    UnityEngine::Transform* FindRecursive(UnityEngine::Transform* parent, std::string name);
    std::string GetTransformPath(UnityEngine::Transform* parent, UnityEngine::Transform* child);

    template<class T>
    T GetOrAddComponent(UnityEngine::Component* self) {
        if (T existing = self->GetComponent<T>())
            return existing;
        return self->get_gameObject()->AddComponent<T>();
    }

    template<class T>
    std::vector<std::string> GetKeys(std::vector<std::pair<std::string, T>> vec) {
        std::vector<std::string> keys;
        for (auto& [key, _] : vec)
            keys.emplace_back(key);
        return keys;
    }

    void InstantSetToggle(UnityEngine::UI::Toggle* toggle, bool value);
    void SetDropdownValue(HMUI::SimpleTextDropdown* dropdown, std::string value);
    HMUI::SimpleTextDropdown* CreateDropdown(UnityEngine::GameObject* parent, std::string name, std::string value, std::vector<std::string_view> values, std::function<void (std::string)> onChange);
    HMUI::SimpleTextDropdown* CreateDropdownEnum(UnityEngine::GameObject* parent, std::string name, int value, std::vector<std::string_view> values, std::function<void (int)> onChange);
    BSML::ColorSetting* CreateColorPicker(UnityEngine::GameObject* parent, std::string name, UnityEngine::Color value, std::function<void (UnityEngine::Color)> onChange, std::function<void ()> onClose);
    void AddSliderEndDrag(BSML::SliderSetting* slider, std::function<void (float)> onEndDrag);
    void AddStringSettingOnClose(HMUI::InputFieldView* input, std::function<void (std::string)> onKeyboardClosed);
    void AddIncrementIncrement(BSML::IncrementSetting* setting, float increment);
    void SetChildrenWidth(UnityEngine::Transform* parent, float width);
    void FixScrollView(UnityEngine::GameObject* scrollView, float width);
    void SetScrollViewActive(UnityEngine::GameObject* scrollView, bool active);
    void RebuildWithScrollPosition(UnityEngine::GameObject* scrollView);
}
