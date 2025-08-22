#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "HMUI/InputFieldView.hpp"
#include "HMUI/SimpleTextDropdown.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "bsml/shared/BSML/Components/Settings/ColorSetting.hpp"
#include "customtypes/settings.hpp"

namespace Qounters::Utils {
    UnityEngine::Vector3 GetFixedEuler(UnityEngine::Quaternion rotation);
    UnityEngine::Color GetClampedColor(std::tuple<float, float, float> hsv);

    std::tuple<std::string, std::string, int> GetBeatmapDetails(GlobalNamespace::BeatmapKey beatmap);
    std::string GetBeatmapIdentifier(GlobalNamespace::BeatmapKey beatmap);
    std::vector<std::string> GetSimplifiedRequirements(GlobalNamespace::BeatmapKey beatmap);

    template <class U, class T>
    U* ptr_cast(T* inst) {
        return il2cpp_utils::try_cast<U>(inst).value_or(nullptr);
    }

    template <class T>
    std::vector<std::string_view> GetKeys(std::vector<std::pair<std::string, T>> vec) {
        std::vector<std::string_view> keys;
        for (auto& [key, _] : vec)
            keys.emplace_back(key);
        return keys;
    }

    BSML::ColorSetting* CreateColorPicker(
        UnityEngine::GameObject* parent,
        std::string name,
        UnityEngine::Color value,
        std::function<void(UnityEngine::Color)> onChange,
        std::function<void()> onClose
    );
    Qounters::HSVController* CreateHSVModifierPicker(
        UnityEngine::GameObject* parent, std::string name, std::function<void(UnityEngine::Vector3)> onChange, std::function<void()> onClose
    );
    Qounters::CollapseController* CreateCollapseArea(UnityEngine::GameObject* parent, std::string title, bool open, int copyId = -1);
    Qounters::MenuDragger* CreateMenuDragger(UnityEngine::GameObject* parent, bool isLeftMenu);

    void RebuildWithScrollPosition(UnityEngine::GameObject* scrollView);

    UnityEngine::RectTransform* GetScrollViewTop(UnityEngine::GameObject* scrollView);
}