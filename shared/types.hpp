#pragma once

#include "UnityEngine/GameObject.hpp"
#include "rapidjson-macros/shared/macros.hpp"

namespace Qounters {
    enum class Sabers {
        Left,
        Right,
        Both,
    };

    enum class Sources {
        Text,
        Shape,
        Color,
        Enable,
    };

    template <class T>
    using SourceFn = std::function<T(UnparsedJSON)>;
    using SourceUIFn = std::function<void(UnityEngine::GameObject*, UnparsedJSON)>;
    using TemplateUIFn = std::function<void(UnityEngine::GameObject*)>;
}
