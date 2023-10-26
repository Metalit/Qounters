#pragma once

#include "main.hpp"

namespace Qounters {
    void PresentSettingsEnvironment();
    void DismissSettingsEnvironment();
    void RefreshSettingsEnvironment();
    bool InSettingsEnvironment();

    void OnSceneStart();
    void OnSceneEnd();
    void OnSceneRefresh();
}
