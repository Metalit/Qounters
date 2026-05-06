#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "HMUI/InputFieldView.hpp"
#include "bsml/shared/BSML/Components/Settings/ColorSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/SliderSetting.hpp"
#include "export.h"
#include "options.hpp"
#include "sources.hpp"

namespace Qounters::API {
    QOUNTERS_EXPORT void RegisterTemplate(std::string section, std::string title, Types::TemplateUIFn function);
    QOUNTERS_EXPORT void RegisterPremade(
        std::string mod, std::string name, Types::PremadeFn creation, Types::PremadeUIFn uiFunction = nullptr, Types::PremadeUpdateFn update = nullptr
    );

    QOUNTERS_EXPORT void CloseTemplateModal();
    QOUNTERS_EXPORT void AddGroup(Options::Group group);

    QOUNTERS_EXPORT int GetActionId();
    QOUNTERS_EXPORT void SetSourceOptions(int actionId, UnparsedJSON options);
    QOUNTERS_EXPORT void SetColorOptions(int actionId, UnparsedJSON options);
    QOUNTERS_EXPORT void SetEnableOptions(int actionId, UnparsedJSON options);
    QOUNTERS_EXPORT void FinalizeAction();

    QOUNTERS_EXPORT BSML::ColorSetting* CreateColorPicker(
        UnityEngine::GameObject* parent,
        std::string name,
        UnityEngine::Color value,
        std::function<void(UnityEngine::Color)> onChange,
        std::function<void()> onClose
    );

    QOUNTERS_EXPORT float GetPlaytestPB();

    QOUNTERS_EXPORT bool InSettings();
    QOUNTERS_EXPORT bool IsInstalled();
    QOUNTERS_EXPORT bool IsEnabled();
    QOUNTERS_EXPORT bool IsEnabledForSelection(GlobalNamespace::BeatmapKey beatmap);
}

// see README
