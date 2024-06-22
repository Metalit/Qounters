#pragma once

#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "main.hpp"

namespace Qounters {
    void PresentSettingsEnvironment();
    void DismissSettingsEnvironment();
    void RefreshSettingsEnvironment();
    bool InSettingsEnvironment();
    std::string CurrentSettingsEnvironment();
    std::string CurrentColorScheme();

    void OnSceneStart(GlobalNamespace::EnvironmentInfoSO* environment);
    void OnSceneEnd();
}
