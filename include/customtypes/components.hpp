#pragma once

#include "config.hpp"
#include "main.hpp"

#include "custom-types/shared/macros.hpp"

#include "HMUI/ImageView.hpp"
#include "UnityEngine/UI/VertexHelper.hpp"
#include "UnityEngine/GameObject.hpp"
#include <array>

#define METHOD(...) il2cpp_utils::il2cpp_type_check::MetadataGetter<__VA_ARGS__>::get()
#define CAST_METHOD(c, m, ...) METHOD(static_cast<void (c::*)(__VA_ARGS__)>(&c::m))

DECLARE_CLASS_CODEGEN(Qounters, Shape, HMUI::ImageView,
    enum ShapeType {
        Square,
        Circle,
        Triangle,
    };

    DECLARE_INSTANCE_FIELD(bool, filled);
    DECLARE_INSTANCE_FIELD(int, shape);
    DECLARE_INSTANCE_FIELD(float, border);

    DECLARE_OVERRIDE_METHOD(void, OnPopulateMesh, CAST_METHOD(UnityEngine::UI::Graphic, OnPopulateMesh, UnityEngine::UI::VertexHelper*), UnityEngine::UI::VertexHelper* vh);

    DECLARE_STATIC_METHOD(Shape*, Create, UnityEngine::GameObject* object, int shape, bool filled, float borderWidth = 0);
)

#include "UnityEngine/Transform.hpp"

DECLARE_CLASS_CODEGEN(Qounters, BaseGameGraphic, UnityEngine::UI::Graphic,
    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::Transform*, instance, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(ArrayW<UnityEngine::UI::Graphic*>, graphics, nullptr);

    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(int, component);

    DECLARE_INSTANCE_METHOD(void, Update);

    DECLARE_INSTANCE_METHOD(void, SetComponent, int component);
    DECLARE_INSTANCE_METHOD(void, SetChildColors);
    DECLARE_OVERRIDE_METHOD(void, OnPopulateMesh, CAST_METHOD(UnityEngine::UI::Graphic, OnPopulateMesh, UnityEngine::UI::VertexHelper*), UnityEngine::UI::VertexHelper* vh);

    DECLARE_STATIC_METHOD(BaseGameGraphic*, Create, UnityEngine::Transform* parent);
    DECLARE_STATIC_METHOD(void, MakeClones);

    private:
    static constexpr int cloneCount = (int) BaseGameOptions::Components::ComponentsMax + 1;
    static std::array<UnityEngine::Transform*, cloneCount> clones;
    static std::array<std::map<std::string, float>, cloneCount> alphaIndex;

    bool updateChildren = false;
)

DECLARE_CLASS_CODEGEN(Qounters, ImageSpriteCache, UnityEngine::MonoBehaviour,
    DECLARE_DEFAULT_CTOR();
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

    DECLARE_STATIC_METHOD(ImageSpriteCache*, GetInstance);
    DECLARE_STATIC_METHOD(void, LoadAllSprites);
    DECLARE_STATIC_METHOD(int, NumberOfSprites);
    DECLARE_STATIC_METHOD(UnityEngine::Sprite*, GetSpriteIdx, int spriteIdx);

    DECLARE_INSTANCE_FIELD(List<UnityEngine::Sprite*>*, sprites);

    public:
    std::vector<std::string> spritePaths;
    static UnityEngine::Sprite* GetSprite(std::string name);

    private:
    static ImageSpriteCache* instance;
)
