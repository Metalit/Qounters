#pragma once

#include "beatsaber-hook/shared/utils/logging.hpp"

static constexpr auto logger = Paper::ConstLoggerContext(MOD_ID);

#define IMAGE_DIRECTORY "/sdcard/ModData/com.beatgames.beatsaber/Mods/" MOD_ID "/"
#define SPEED_SAMPLES_PER_SEC 4
#define MAX_SECS_WITHOUT_DRAG 0.2

extern bool hasCJD;

#include "GlobalNamespace/IConnectedPlayer.hpp"

extern GlobalNamespace::IConnectedPlayer* localFakeConnectedPlayer;

#include "UnityEngine/Canvas.hpp"

extern bool blockOtherRaycasts;
extern std::unordered_set<UnityEngine::Canvas*> raycastCanvases;
