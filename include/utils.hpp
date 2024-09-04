#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "HMUI/InputFieldView.hpp"
#include "HMUI/SimpleTextDropdown.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "VRUIControls/VRInputModule.hpp"
#include "bsml/shared/BSML/Components/Settings/ColorSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/DropdownListSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/IncrementSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/SliderSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/ToggleSetting.hpp"
#include "customtypes/settings.hpp"

namespace Qounters::Utils {
    std::string FormatDecimals(double num, int decimals);
    std::string SecondsToString(int seconds);

    UnityEngine::Color GetClampedColor(std::tuple<float, float, float> hsv);

    std::tuple<std::string, std::string, int> GetBeatmapDetails(GlobalNamespace::BeatmapKey beatmap);
    std::string GetBeatmapIdentifier(GlobalNamespace::BeatmapKey beatmap);
    std::vector<std::string> GetSimplifiedRequirements(GlobalNamespace::BeatmapKey beatmap);

    void DisableAllBut(UnityEngine::Transform* parent, std::set<std::string> enabled, std::set<std::string> disabled = {});
    UnityEngine::Transform* FindRecursive(UnityEngine::Transform* parent, std::string name);
    std::string GetTransformPath(UnityEngine::Transform* parent, UnityEngine::Transform* child);
    void SetRelativeSiblingIndex(UnityEngine::Transform* child, UnityEngine::Transform* ref, int amount);

    template <class T>
    T GetOrAddComponent(UnityEngine::Component* self) {
        if (T existing = self->GetComponent<T>())
            return existing;
        return self->gameObject->AddComponent<T>();
    }

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

    void SetLayoutSize(UnityEngine::Component* object, float width, float height, float flex = -1);
    void SetChildrenWidth(UnityEngine::Transform* parent, float width);
    void SetCanvasSorting(UnityEngine::GameObject* canvas, int value);
    void InstantSetToggle(BSML::ToggleSetting* toggle, bool value);
    void SetIncrementValue(BSML::IncrementSetting* increment, float value);
    void SetDropdownValue(BSML::DropdownListSetting* dropdown, std::string value);
    void SetDropdownValues(
        BSML::DropdownListSetting* dropdown, std::vector<std::string> values, std::string selected, std::function<void()> notPresent = nullptr
    );
    void SetIconButtonSprite(UnityEngine::UI::Button* button, UnityEngine::Sprite* sprite);

    UnityEngine::UI::Button* CreateIconButton(UnityEngine::GameObject* parent, UnityEngine::Sprite* sprite, std::function<void()> onClick);
    BSML::DropdownListSetting* CreateDropdown(
        UnityEngine::GameObject* parent,
        std::string name,
        std::string value,
        std::vector<std::string_view> values,
        std::function<void(std::string)> onChange
    );
    BSML::DropdownListSetting* CreateDropdownEnum(
        UnityEngine::GameObject* parent, std::string name, int value, std::vector<std::string_view> values, std::function<void(int)> onChange
    );
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
    Qounters::CollapseController*
    CreateCollapseArea(UnityEngine::GameObject* parent, std::string title, bool open, std::set<UnityEngine::Component*> contents = {});
    Qounters::MenuDragger* CreateMenuDragger(UnityEngine::GameObject* parent, bool isLeftMenu);

    void AddModalAnimations(HMUI::SimpleTextDropdown* dropdown, HMUI::ModalView* behindModal);
    void AddSliderEndDrag(BSML::SliderSetting* slider, std::function<void(float)> onEndDrag);
    void AddStringSettingOnClose(HMUI::InputFieldView* input, std::function<void(std::string)> onKeyboardClosed);
    void AddIncrementIncrement(BSML::IncrementSetting* setting, float increment);

    BSML::SliderSetting* ReparentSlider(BSML::SliderSetting* slider, BSML::Lite::TransformWrapper parent, float width);
    void RebuildWithScrollPosition(UnityEngine::GameObject* scrollView);

    UnityEngine::RectTransform* GetScrollViewTop(UnityEngine::GameObject* scrollView);
    VRUIControls::VRInputModule* GetCurrentInputModule();
}
