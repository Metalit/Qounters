#pragma once

#include "metacore/shared/assets.hpp"

#define DECLARE_ASSET(name, binary)       \
    const IncludedAsset name {            \
        Externs::_binary_##binary##_start, \
        Externs::_binary_##binary##_end    \
    };

#define DECLARE_ASSET_NS(namespaze, name, binary) \
    namespace namespaze { DECLARE_ASSET(name, binary) }

namespace IncludedAssets {
    namespace Externs {
        extern "C" uint8_t _binary_Arrow_png_start[];
        extern "C" uint8_t _binary_Arrow_png_end[];
        extern "C" uint8_t _binary_Beatleader_png_start[];
        extern "C" uint8_t _binary_Beatleader_png_end[];
        extern "C" uint8_t _binary_Bloq_png_start[];
        extern "C" uint8_t _binary_Bloq_png_end[];
        extern "C" uint8_t _binary_Bomb_png_start[];
        extern "C" uint8_t _binary_Bomb_png_end[];
        extern "C" uint8_t _binary_Chain_png_start[];
        extern "C" uint8_t _binary_Chain_png_end[];
        extern "C" uint8_t _binary_ChainArrow_png_start[];
        extern "C" uint8_t _binary_ChainArrow_png_end[];
        extern "C" uint8_t _binary_Copy_png_start[];
        extern "C" uint8_t _binary_Copy_png_end[];
        extern "C" uint8_t _binary_Default_png_start[];
        extern "C" uint8_t _binary_Default_png_end[];
        extern "C" uint8_t _binary_Frame_png_start[];
        extern "C" uint8_t _binary_Frame_png_end[];
        extern "C" uint8_t _binary_Lock_png_start[];
        extern "C" uint8_t _binary_Lock_png_end[];
        extern "C" uint8_t _binary_Scoresaber_png_start[];
        extern "C" uint8_t _binary_Scoresaber_png_end[];
        extern "C" uint8_t _binary_Unlock_png_start[];
        extern "C" uint8_t _binary_Unlock_png_end[];
        extern "C" uint8_t _binary_Wall_png_start[];
        extern "C" uint8_t _binary_Wall_png_end[];
    }

    // Arrow.png
    DECLARE_ASSET(Arrow_png, Arrow_png);
    // Beatleader.png
    DECLARE_ASSET(Beatleader_png, Beatleader_png);
    // Bloq.png
    DECLARE_ASSET(Bloq_png, Bloq_png);
    // Bomb.png
    DECLARE_ASSET(Bomb_png, Bomb_png);
    // Chain.png
    DECLARE_ASSET(Chain_png, Chain_png);
    // ChainArrow.png
    DECLARE_ASSET(ChainArrow_png, ChainArrow_png);
    // Copy.png
    DECLARE_ASSET(Copy_png, Copy_png);
    // Default.png
    DECLARE_ASSET(Default_png, Default_png);
    // Frame.png
    DECLARE_ASSET(Frame_png, Frame_png);
    // Lock.png
    DECLARE_ASSET(Lock_png, Lock_png);
    // Scoresaber.png
    DECLARE_ASSET(Scoresaber_png, Scoresaber_png);
    // Unlock.png
    DECLARE_ASSET(Unlock_png, Unlock_png);
    // Wall.png
    DECLARE_ASSET(Wall_png, Wall_png);
}
