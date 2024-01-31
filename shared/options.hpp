#pragma once

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
        VALUE_DEFAULT(std::string, TextSource, TextSource::StaticName)
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
        VALUE_DEFAULT(std::string, FillSource, TextSource::StaticName)
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
        VALUE_DEFAULT(std::string, ColorSource, ColorSource::StaticName)
        VALUE_DEFAULT(UnparsedJSON, ColorOptions, ColorSource::Static())
        VALUE_DEFAULT(std::string, EnableSource, EnableSource::StaticName)
        VALUE_DEFAULT(bool, InvertEnable, false);
        VALUE_DEFAULT(UnparsedJSON, EnableOptions, EnableSource::Static())
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

    extern const std::vector<std::string> TypeStrings;
    extern const std::vector<std::string> AnchorStrings;
    extern const std::vector<std::string> AlignStrings;
    extern const std::vector<std::string> ShapeStrings;
    extern const std::vector<std::string> FillStrings;
    extern const std::vector<std::string> ComponentStrings;
    extern const std::vector<std::string> SaberStrings;
}
