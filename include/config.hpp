#pragma once

#include "config-utils/shared/config-utils.hpp"
#include "options.hpp"
#include "sources.hpp"

// not in shared
namespace Qounters::Options {
    void CreateTextUI(UnityEngine::GameObject* parent, Text const& options);
    void CreateShapeUI(UnityEngine::GameObject* parent, Shape const& options);
    void CreateImageUI(UnityEngine::GameObject* parent, Image const& options);
    void CreatePremadeUI(UnityEngine::GameObject* parent, Premade const& options);
    void CreateTypeUI(UnityEngine::Transform* parent, int type, Component::OptionsTypes const& options);
}

DECLARE_CONFIG(Config) {
    DECLARE_JSON_STRUCT(PresetOverride) {
        VALUE_DEFAULT(bool, Enabled, false);
        VALUE_DEFAULT(std::string, Preset, "");
    };
    CONFIG_VALUE(Enabled, bool, "Enable Qounters++", true, "Turn this off to disable all ingame GUI modifications");
    CONFIG_VALUE(Noodle, bool, "Disable On Chroma/Noodle", false, "Enable or disable the mod for noodle maps specifically");
    CONFIG_VALUE(Migrated, bool, "Migrated", false);
    CONFIG_VALUE(Presets, StringKeyedMap<Qounters::Options::Preset>, "Presets", {});
    CONFIG_VALUE(SettingsPreset, std::string, "Settings Preset", "Default");
    CONFIG_VALUE(Preset, std::string, "Preset", "Default");
    CONFIG_VALUE(TypePresets, StringKeyedMap<PresetOverride>, "Type Presets", {});
    CONFIG_VALUE(SpecificPresets, StringKeyedMap<PresetOverride>, "Environment Specific Presets", {});
    CONFIG_VALUE(EnvironmentType, int, "Settings Environment Type", 0);
    CONFIG_VALUE(Environment, std::string, "Settings Environment", "The First");
    CONFIG_VALUE(ColorScheme, std::string, "Color Scheme", "User Override / Environment");
    CONFIG_VALUE(Snap, bool, "Snap Enabled", true);
    CONFIG_VALUE(SnapStep, float, "Snap To Grid", 2);
    CONFIG_VALUE(LeftOffset, float, "Left Menu Position", 0);
    CONFIG_VALUE(RightOffset, float, "Right Menu Position", 0);
};
