#pragma once

#include "sources.hpp"

namespace Qounters {
    extern std::vector<std::string_view> const TypeStrings;
    extern std::vector<std::string_view> const AnchorStrings;
    extern std::vector<std::string_view> const AlignStrings;
    extern std::vector<std::string_view> const ShapeStrings;
    extern std::vector<std::string_view> const FillStrings;
    extern std::vector<std::string_view> const BaseGameObjectStrings;
    extern std::vector<std::string_view> const SaberStrings;

    DECLARE_JSON_STRUCT(TextOptions) {
        enum class Aligns {
            Left,
            Right,
            Center,
        };
        VALUE_DEFAULT(int, Align, (int) Aligns::Center);
        VALUE_DEFAULT(float, Size, 15);
        VALUE_DEFAULT(bool, Italic, false);
        VALUE_DEFAULT(std::string, TextSource, TextSource::StaticName);
        VALUE_DEFAULT(UnparsedJSON, SourceOptions, TextSource::Static());
    };

    DECLARE_JSON_STRUCT(ShapeOptions) {
        enum class Shapes {
            Square,
            SquareOutline,
            Circle,
            CircleOutline,
            Triangle,
            TriangleOutline,
        };
        static inline bool IsOutline(int shape) {
            return std::set{Shapes::SquareOutline, Shapes::CircleOutline, Shapes::TriangleOutline}.contains((Shapes) shape);
        };
        enum class Fills {
            None,
            Horizontal,
            Vertical,
            Circle,
        };
        VALUE_DEFAULT(int, Shape, (int) Shapes::Square);
        VALUE_DEFAULT(float, OutlineWidth, 1);
        VALUE_DEFAULT(int, Fill, (int) Fills::None);
        VALUE_DEFAULT(std::string, FillSource, TextSource::StaticName);
        VALUE_DEFAULT(UnparsedJSON, SourceOptions, ShapeSource::Static());
        VALUE_DEFAULT(bool, Inverse, false);
    };

    DECLARE_JSON_STRUCT(ImageOptions) {
        VALUE_DEFAULT(std::string, Path, "Default.png");
    };

    DECLARE_JSON_STRUCT(PremadeOptions) {
        VALUE_DEFAULT(std::string, SourceMod, "");
        VALUE_DEFAULT(std::string, Name, std::string(BaseGameObjectStrings[0]));
        VALUE_DEFAULT(UnparsedJSON, Options, {});
    };

    DECLARE_JSON_STRUCT(Component) {
        enum class Types {
            Text,
            Shape,
            Image,
            Premade,
        };
        using OptionsTypes = TypeOptions<TextOptions, ShapeOptions, ImageOptions, PremadeOptions>;
        VALUE_DEFAULT(ConfigUtils::Vector2, Position, ConfigUtils::Vector2(0, 0));
        VALUE_DEFAULT(float, Rotation, 0);
        VALUE_DEFAULT(ConfigUtils::Vector2, Scale, ConfigUtils::Vector2(1, 1));
        VALUE(int, Type);
        VALUE(OptionsTypes, Options);
        VALUE_DEFAULT(std::string, ColorSource, ColorSource::StaticName);
        VALUE_DEFAULT(UnparsedJSON, ColorOptions, ColorSource::Static());
        VALUE_DEFAULT(std::string, EnableSource, EnableSource::StaticName);
        VALUE_DEFAULT(bool, InvertEnable, false);
        VALUE_DEFAULT(UnparsedJSON, EnableOptions, EnableSource::Static());
    };

    DECLARE_JSON_STRUCT(Group) {
        enum class Anchors {
            Left,
            Right,
            Top,
            Bottom,
            Center,
            AnchorMax = Center,
        };
        VALUE(ConfigUtils::Vector2, Position);
        VALUE_DEFAULT(float, Rotation, 0);
        VALUE(int, Anchor);
        VECTOR_DEFAULT(Component, Components, {});
        VALUE_DEFAULT(bool, Detached, false);
        VALUE_DEFAULT(ConfigUtils::Vector3, DetachedPosition, ConfigUtils::Vector3(0, 0, 0));
        VALUE_DEFAULT(bool, LockPosX, false);
        VALUE_DEFAULT(bool, LockPosY, false);
        VALUE_DEFAULT(bool, LockPosZ, false);
        VALUE_DEFAULT(ConfigUtils::Vector3, DetachedRotation, ConfigUtils::Vector3(0, 0, 0));
        VALUE_DEFAULT(bool, LockRotX, false);
        VALUE_DEFAULT(bool, LockRotY, false);
        VALUE_DEFAULT(bool, LockRotZ, false);
    };

    DECLARE_JSON_STRUCT(Preset) {
        VECTOR(Group, Qounters);
        MAP(Preset, Layouts);
    };

    Preset GetDefaultHUDPreset();
}
