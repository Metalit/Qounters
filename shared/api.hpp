#pragma once

#include "HMUI/InputFieldView.hpp"
#include "bsml/shared/BSML/Components/Settings/ColorSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/SliderSetting.hpp"
#include "options.hpp"
#include "sources.hpp"

namespace Qounters::API {
    void RegisterTemplate(std::string section, std::string title, Types::TemplateUIFn function);
    void RegisterPremade(
        std::string mod, std::string name, Types::PremadeFn creation, Types::PremadeUIFn uiFunction = nullptr, Types::PremadeUpdateFn update = nullptr
    );

    void CloseTemplateModal();
    void AddGroup(Options::Group group);

    int GetActionId();
    void SetSourceOptions(int actionId, UnparsedJSON options);
    void SetColorOptions(int actionId, UnparsedJSON options);
    void SetEnableOptions(int actionId, UnparsedJSON options);
    void FinalizeAction();

    BSML::ColorSetting* CreateColorPicker(
        UnityEngine::GameObject* parent,
        std::string name,
        UnityEngine::Color value,
        std::function<void(UnityEngine::Color)> onChange,
        std::function<void()> onClose
    );

    float GetPlaytestPB();

    bool InSettings();
}

// see README
