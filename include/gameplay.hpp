#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "UnityEngine/GameObject.hpp"

namespace Qounters::Gameplay {
    void GameplaySetupMenu(UnityEngine::GameObject* parent, bool firstActivation);
    void SetLevel(GlobalNamespace::BeatmapLevel* level, GlobalNamespace::BeatmapKey key, bool enableOverride);
    void ClearLevel();
    void UpdateEnvironment();
    void UpdateUI();
}
