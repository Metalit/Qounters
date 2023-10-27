#pragma once

#include "rapidjson-macros/shared/macros.hpp"

class UnparsedJSON : public JSONClass {
    public:
        void Deserialize(rapidjson::Value const& jsonValue) {
            storedValue = jsonValue;
        }
        rapidjson::Value Serialize(rapidjson::Document::AllocatorType& allocator) const {
            rapidjson::Value ret;
            ret.CopyFrom(storedValue.document, allocator);
            return ret;
        }
        template<JSONClassDerived T>
        T Parse() const {
            T ret;
            ret.Deserialize(storedValue.document);
            return ret;
        }
        template<JSONClassDerived T>
        void Set(T value) {
            value.Serialize(storedValue.document.GetAllocator()).Swap(storedValue.document);
        }
        template<JSONClassDerived T>
        UnparsedJSON& operator=(T&& other) {
            Set(other);
            return *this;
        }
        template<JSONClassDerived T>
        UnparsedJSON(T value) {
            Set(value);
        }
        UnparsedJSON() = default;
        UnparsedJSON(UnparsedJSON const& value) = default;
    private:
        rapidjson_macros_types::CopyableValue storedValue;
};

#include "UnityEngine/GameObject.hpp"

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
    };

    template<class T>
    using SourceFn = std::function<T (UnparsedJSON)>;
    using SourceUIFn = std::function<void (UnityEngine::GameObject*, UnparsedJSON)>;
    using TemplateUIFn = std::function<void (UnityEngine::GameObject*)>;
}
