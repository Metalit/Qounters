#pragma once

#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "main.hpp"

namespace Qounters {
    enum class EnvironmentHUDType {
        Wide,
        Narrow,
        Close,
        Sunken,
        Circle,
    };
    extern std::vector<std::string_view> EnvironmentHUDTypeStrings;
    EnvironmentHUDType GetHUDType(std::string serializedName);

    void PresentSettingsEnvironment();
    void DismissSettingsEnvironment();
    void RefreshSettingsEnvironment();
    bool InSettingsEnvironment();
    std::string CurrentSettingsEnvironment();
    std::string CurrentColorScheme();

    void OnSceneStart(GlobalNamespace::EnvironmentInfoSO* environment);
    void OnSceneEnd();
}
