#pragma once

#include "beatsaber-hook/shared/utils/hooking.hpp"

namespace Qounters {
    enum class Sabers {
        Left,
        Right,
        Both,
    };
    enum class ColorSettings {
        LeftSaber,
        RightSaber,
        Lights1,
        Lights2,
        Walls,
        Boost1,
        Boost2,
    };
}

Logger& getLogger();

#define IMAGE_DIRECTORY "/sdcard/ModData/com.beatgames.beatsaber/Mods/" MOD_ID "/"
#define SPEED_SAMPLES_PER_SEC 4
#define MAX_SECS_WITHOUT_DRAG 0.2

extern bool hasCJD;
