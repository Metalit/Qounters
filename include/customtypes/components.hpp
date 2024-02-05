#pragma once

#include "config.hpp"
#include "main.hpp"

#include "custom-types/shared/macros.hpp"

#include "UnityEngine/UI/MaskableGraphic.hpp"
#include "UnityEngine/UI/VertexHelper.hpp"
#include "UnityEngine/Transform.hpp"

#define UUI UnityEngine::UI

#define METHOD(...) il2cpp_utils::il2cpp_type_check::MetadataGetter<__VA_ARGS__>::get()
#define CAST_METHOD(c, m, ...) METHOD(static_cast<void (c::*)(__VA_ARGS__)>(&c::m))

DECLARE_CLASS_CODEGEN(Qounters, Shape, UUI::MaskableGraphic,
    enum ShapeType {
        Square,
        Circle,
        Triangle,
    };

    DECLARE_INSTANCE_FIELD(bool, filled);
    DECLARE_INSTANCE_FIELD(int, sideCount);
    DECLARE_INSTANCE_FIELD(float, border);

    DECLARE_INSTANCE_METHOD(void, SetFilled, bool value);
    DECLARE_INSTANCE_METHOD(void, SetSideCount, int value);
    DECLARE_INSTANCE_METHOD(void, SetBorder, float value);

    DECLARE_INSTANCE_METHOD(void, SetMaskOptions, int type, bool inverse);
    DECLARE_INSTANCE_METHOD(void, SetMaskAmount, float value);

    DECLARE_OVERRIDE_METHOD(void, OnPopulateMesh, CAST_METHOD(UUI::Graphic, OnPopulateMesh, UUI::VertexHelper*), UUI::VertexHelper* vh);

    DECLARE_STATIC_METHOD(Shape*, Create, UnityEngine::Transform* parent);

    private:
    HMUI::ImageView* mask = nullptr;
)

DECLARE_CLASS_CODEGEN(Qounters, BaseGameGraphic, UUI::Graphic,
    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::Transform*, instance, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(ArrayW<UUI::Graphic*>, graphics, nullptr);

    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(int, component);

    DECLARE_INSTANCE_METHOD(void, Update);

    DECLARE_INSTANCE_METHOD(void, SetComponent, int component);
    DECLARE_INSTANCE_METHOD(void, SetChildColors);
    DECLARE_OVERRIDE_METHOD(void, OnPopulateMesh, CAST_METHOD(UUI::Graphic, OnPopulateMesh, UUI::VertexHelper*), UUI::VertexHelper* vh);

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

#undef UUI
