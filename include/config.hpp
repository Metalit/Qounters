#pragma once

#include "config-utils/shared/config-utils.hpp"
#include "options.hpp"
#include "sources.hpp"

namespace Qounters {
    void CreateTextOptionsUI(UnityEngine::GameObject* parent, TextOptions const& options);
    void CreateShapeOptionsUI(UnityEngine::GameObject* parent, ShapeOptions const& options);
    void CreateImageOptionsUI(UnityEngine::GameObject* parent, ImageOptions const& options);
    void CreatePremadeOptionsUI(UnityEngine::GameObject* parent, PremadeOptions const& options);
    void CreateTypeOptionsUI(UnityEngine::Transform* parent, int type, Component::OptionsTypes const& options);
}

DECLARE_CONFIG(Config) {
    CONFIG_VALUE(Enabled, bool, "Enabled", true);
    CONFIG_VALUE(Migrated, bool, "Migrated", false);
    CONFIG_VALUE(Presets, StringKeyedMap<Qounters::Preset>, "Presets", {});
    CONFIG_VALUE(Preset, std::string, "Preset", "Default");
    CONFIG_VALUE(EnvironmentType, int, "Settings Environment Type", 0);
    CONFIG_VALUE(Environment, std::string, "Settings Environment", "The First");
    CONFIG_VALUE(ColorScheme, std::string, "Color Scheme", "User Override / Environment");
    CONFIG_VALUE(Snap, bool, "Snap Enabled", true);
    CONFIG_VALUE(SnapStep, float, "Snap To Grid", 2);
    CONFIG_VALUE(LeftOffset, float, "Left Menu Position", 0);
    CONFIG_VALUE(RightOffset, float, "Right Menu Position", 0);
};
