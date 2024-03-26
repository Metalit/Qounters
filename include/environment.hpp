#pragma once

#include "main.hpp"

#include "GlobalNamespace/EnvironmentInfoSO.hpp"
namespace Qounters {
    void PresentSettingsEnvironment();
    void DismissSettingsEnvironment();
    void RefreshSettingsEnvironment();
    bool InSettingsEnvironment();
    std::string CurrentSettingsEnvironment();

    void OnSceneStart(GlobalNamespace::EnvironmentInfoSO* environment);
    void OnSceneEnd();
}
