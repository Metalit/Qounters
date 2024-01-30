#pragma once

#include "main.hpp"

namespace Qounters {
    void PresentSettingsEnvironment();
    void DismissSettingsEnvironment();
    void RefreshSettingsEnvironment();
    bool InSettingsEnvironment();
    std::string CurrentSettingsEnvironment();

    void OnSceneStart();
    void OnSceneEnd();
}
