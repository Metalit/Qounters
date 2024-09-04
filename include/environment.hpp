#pragma once

#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "main.hpp"

namespace Qounters::Environment {
    enum class HUDType {
        Wide,
        Narrow,
        Close,
        Sunken,
        Circle,
        Max = Circle,
    };
    extern std::vector<std::string_view> HUDTypeStrings;
    HUDType GetHUDType(std::string serializedName);

    void PresentSettings();
    void DismissSettings();
    void RefreshSettings();
    bool InSettings();
    std::string CurrentSettingsEnvironment();
    std::string CurrentColorScheme();

    void SetPlayerActive(bool active);

    void OnSceneStart(GlobalNamespace::EnvironmentInfoSO* environment);
    void OnSceneEnd();
}
