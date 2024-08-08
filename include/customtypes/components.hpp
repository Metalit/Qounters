#pragma once

#include "GlobalNamespace/IAudioTimeSource.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/MaskableGraphic.hpp"
#include "UnityEngine/UI/VertexHelper.hpp"
#include "config.hpp"
#include "custom-types/shared/macros.hpp"
#include "main.hpp"
#include "options.hpp"

#define UUI UnityEngine::UI

#define CAST_METHOD(c, m, ...) static_cast<void (c::*)(__VA_ARGS__)>(&c::m)

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

    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPopulateMesh, CAST_METHOD(UUI::Graphic, OnPopulateMesh, UUI::VertexHelper*), UUI::VertexHelper* vh);

    DECLARE_STATIC_METHOD(Shape*, Create, UnityEngine::Transform* parent);

   private:
    HMUI::ImageView* mask = nullptr;
)

DECLARE_CLASS_CODEGEN(Qounters, BaseGameGraphic, UUI::Graphic,
   public:
    enum class Objects {
        Multiplier,
        ProgressBar,
        HealthBar,
        ComponentsMax = HealthBar,
    };

    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::Transform*, instance, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(ArrayW<UUI::Graphic*>, graphics, nullptr);

    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(int, component);

    DECLARE_INSTANCE_METHOD(void, Update);

    DECLARE_INSTANCE_METHOD(void, SetComponent, int component);
    DECLARE_INSTANCE_METHOD(void, SetChildColors);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPopulateMesh, CAST_METHOD(UUI::Graphic, OnPopulateMesh, UUI::VertexHelper*), UUI::VertexHelper*);

    DECLARE_STATIC_METHOD(BaseGameGraphic*, Create, UnityEngine::Transform* parent);
    DECLARE_STATIC_METHOD(void, MakeClones);
    DECLARE_STATIC_METHOD(void, Reset);

   private:
    static constexpr int cloneCount = (int) Objects::ComponentsMax + 1;
    static std::array<UnityEngine::Transform*, cloneCount> clones;
    static std::array<std::map<std::string, float>, cloneCount> alphaIndex;

    bool updateChildren = false;
)

DECLARE_CLASS_CODEGEN(Qounters, PremadeParent, UUI::Graphic,
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPopulateMesh, CAST_METHOD(UUI::Graphic, OnPopulateMesh, UUI::VertexHelper*), UUI::VertexHelper*);

    DECLARE_INSTANCE_METHOD(UUI::Graphic*, GetGraphic);

    DECLARE_INSTANCE_FIELD(UUI::Graphic*, graphic);
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, rectTransform);
    DECLARE_INSTANCE_FIELD(bool, updateColor);

   public:
    PremadeOptions options;
)

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, SongTimeSource, Il2CppObject, classof(GlobalNamespace::IAudioTimeSource*),
    DECLARE_OVERRIDE_METHOD_MATCH(float, get_songTime, &GlobalNamespace::IAudioTimeSource::get_songTime);
    DECLARE_OVERRIDE_METHOD_MATCH(float, get_lastFrameDeltaSongTime, &GlobalNamespace::IAudioTimeSource::get_lastFrameDeltaSongTime);
    DECLARE_OVERRIDE_METHOD_MATCH(float, get_songEndTime, &GlobalNamespace::IAudioTimeSource::get_songEndTime);
    DECLARE_OVERRIDE_METHOD_MATCH(float, get_songLength, &GlobalNamespace::IAudioTimeSource::get_songLength);
    DECLARE_OVERRIDE_METHOD_MATCH(bool, get_isReady, &GlobalNamespace::IAudioTimeSource::get_isReady);
)

DECLARE_CLASS_CODEGEN(Qounters, ImageSpriteCache, UnityEngine::MonoBehaviour,
    DECLARE_DEFAULT_CTOR();
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

    DECLARE_STATIC_METHOD(ImageSpriteCache*, GetInstance);
    DECLARE_STATIC_METHOD(void, LoadAllSprites);
    DECLARE_STATIC_METHOD(int, NumberOfSprites);
    DECLARE_STATIC_METHOD(UnityEngine::Sprite*, GetSpriteIdx, int spriteIdx);

    DECLARE_INSTANCE_FIELD(ListW<UnityEngine::Sprite*>, sprites);

   public:
    std::vector<std::string> spritePaths;
    static UnityEngine::Sprite* GetSprite(std::string name);

   private:
    static ImageSpriteCache* instance;
)

DECLARE_CLASS_CODEGEN(Qounters, DestroySignal, UnityEngine::MonoBehaviour,
    DECLARE_DEFAULT_CTOR();
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

   public:
    static void Create(std::function<void()> callback);

   private:
    std::function<void()> callback = nullptr;
)

#undef UUI
#undef CAST_METHOD
