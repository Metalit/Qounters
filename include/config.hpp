#pragma once

#include "config-utils/shared/config-utils.hpp"
#include "options.hpp"
#include "sources.hpp"

namespace Qounters {
    void CreateTextOptionsUI(UnityEngine::GameObject* parent, TextOptions const& options);
    void CreateShapeOptionsUI(UnityEngine::GameObject* parent, ShapeOptions const& options);
    void CreateImageOptionsUI(UnityEngine::GameObject* parent, ImageOptions const& options);
    void CreateBaseGameOptionsUI(UnityEngine::GameObject* parent, BaseGameOptions const& options);
    void CreateTypeOptionsUI(UnityEngine::Transform* parent, int type, Component::OptionsTypes const& options);
}

DECLARE_CONFIG(Config,
    CONFIG_VALUE(Enabled, bool, "Enabled", true)
    CONFIG_VALUE(Migrated, bool, "Migrated", false)
    CONFIG_VALUE(Outlines, bool, "Outlines", true)
    CONFIG_VALUE(Presets, StringKeyedMap<Qounters::Preset>, "Presets", {})
    CONFIG_VALUE(Preset, std::string, "Preset", "Default")
    CONFIG_VALUE(Environment, std::string, "Settings Environment", "The First")
)
