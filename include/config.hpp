#pragma once

#include "config-utils/shared/config-utils.hpp"
#include "sources.hpp"

namespace Qounters {
    DECLARE_JSON_CLASS(TextOptions,
        enum class Aligns {
            Left,
            Right,
            Center,
        };
        VALUE_DEFAULT(int, Align, (int) Aligns::Center)
        VALUE_DEFAULT(float, Size, 15)
        VALUE_DEFAULT(bool, Italic, false)
        VALUE_DEFAULT(std::string, TextSource, "Static")
        VALUE_DEFAULT(UnparsedJSON, SourceOptions, TextSource::Static())
    )

    DECLARE_JSON_CLASS(ShapeOptions,
        enum class Shapes {
            Square,
            SquareOutline,
            Circle,
            CircleOutline,
            Triangle,
            TriangleOutline,
        };
        enum class Fills {
            None,
            Horizontal,
            Vertical,
            Circle,
        };
        VALUE_DEFAULT(int, Shape, (int) Shapes::Square)
        VALUE_DEFAULT(float, OutlineWidth, 1)
        VALUE_DEFAULT(int, Fill, (int) Fills::None)
        VALUE_DEFAULT(std::string, FillSource, "Static")
        VALUE_DEFAULT(UnparsedJSON, SourceOptions, ShapeSource::Static())
        VALUE_DEFAULT(bool, Inverse, false)
    )

    DECLARE_JSON_CLASS(ImageOptions,
        VALUE_DEFAULT(std::string, Path, "Default.png")
    )

    DECLARE_JSON_CLASS(BaseGameOptions,
        enum class Components {
            Multiplier,
            ProgressBar,
            HealthBar,
            ComponentsMax = HealthBar,
        };
        VALUE(int, Component)
    )

    DECLARE_JSON_CLASS(Component,
        enum class Types {
            Text,
            Shape,
            Image,
            BaseGame,
        };
        using OptionsTypes = TypeOptions<TextOptions, ShapeOptions, ImageOptions, BaseGameOptions>;
        VALUE(ConfigUtils::Vector2, Position)
        VALUE(float, Rotation)
        VALUE_DEFAULT(ConfigUtils::Vector2, Scale, ConfigUtils::Vector2(1, 1))
        VALUE(int, Type)
        VALUE(OptionsTypes, Options)
        VALUE_DEFAULT(std::string, ColorSource, "Static")
        VALUE_DEFAULT(UnparsedJSON, ColorOptions, ColorSource::Static())
    )

    DECLARE_JSON_CLASS(Group,
        enum class Anchors {
            Left,
            Right,
            Top,
            Bottom,
            AnchorMax = Bottom,
        };
        VALUE(ConfigUtils::Vector2, Position)
        VALUE(float, Rotation)
        VALUE(int, Anchor)
        VECTOR(Component, Components)
    )

    DECLARE_JSON_CLASS(Preset,
        VECTOR(Group, Qounters)
        MAP(Preset, Layouts)
    )

    Preset GetDefaultHUDPreset();

    extern std::vector<std::string> TypeStrings;
    extern std::vector<std::string> AnchorStrings;
    extern std::vector<std::string> AlignStrings;
    extern std::vector<std::string> ShapeStrings;
    extern std::vector<std::string> FillStrings;
    extern std::vector<std::string> ComponentStrings;

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
