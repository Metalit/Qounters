#pragma once

#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "UnityEngine/GameObject.hpp"

namespace Qounters::Gameplay {
    void GameplaySetupMenu(UnityEngine::GameObject* parent, bool firstActivation);
    void UpdateEnvironment();
    void UpdateUI();
}
