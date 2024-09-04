#pragma once

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/Graphic.hpp"
#include "rapidjson-macros/shared/macros.hpp"

namespace Qounters::Types {
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
    using PremadeFn = std::function<UnityEngine::UI::Graphic*(UnityEngine::GameObject*, UnparsedJSON)>;
    using PremadeUIFn = std::function<void(UnityEngine::GameObject*, UnparsedJSON)>;
    using PremadeUpdateFn = std::function<void(UnityEngine::UI::Graphic*, UnparsedJSON)>;
    using TemplateUIFn = std::function<void(UnityEngine::GameObject*)>;
}
