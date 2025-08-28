#pragma once

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/Graphic.hpp"
#include "metacore/shared/stats.hpp"
#include "rapidjson-macros/shared/macros.hpp"

namespace Qounters::Types {
    enum class Sabers {
        Left = MetaCore::Stats::LeftSaber,
        Right = MetaCore::Stats::RightSaber,
        Both = MetaCore::Stats::BothSabers,
    };

    enum class Sources {
        Text,
        Shape,
        Color,
        Enable,
    };

    enum class Separators {
        None,
        Gap,
        Comma,
        Period,
    };

    template <class T>
    using SourceFn = std::function<T(UnparsedJSON)>;
    using SourceUIFn = std::function<void(UnityEngine::GameObject*, UnparsedJSON)>;
    using PremadeFn = std::function<UnityEngine::UI::Graphic*(UnityEngine::GameObject*, UnparsedJSON)>;
    using PremadeUIFn = std::function<void(UnityEngine::GameObject*, UnparsedJSON)>;
    using PremadeUpdateFn = std::function<void(UnityEngine::UI::Graphic*, UnparsedJSON)>;
    using TemplateUIFn = std::function<void(UnityEngine::GameObject*)>;
}
