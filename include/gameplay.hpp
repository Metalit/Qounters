#pragma once

#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "UnityEngine/GameObject.hpp"

namespace Qounters {
    void GameplaySetupMenu(UnityEngine::GameObject* parent, bool firstActivation);
    void SetLevel(GlobalNamespace::BeatmapLevel* level, GlobalNamespace::BeatmapKey key, bool enableOverride);
    void ClearLevel();
    void UpdateEnvironment();
    void UpdateUI();
}
