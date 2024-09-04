#pragma once

#include "GlobalNamespace/IAudioTimeSource.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/MaskableGraphic.hpp"
#include "UnityEngine/UI/VertexHelper.hpp"
#include "System/Action_1.hpp"
#include "TMPro/TMP_TextInfo.hpp"
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

    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(bool, filled);
    DECLARE_INSTANCE_FIELD(int, sideCount);
    DECLARE_INSTANCE_FIELD(float, border);
    DECLARE_INSTANCE_FIELD(bool, gradient);
    DECLARE_INSTANCE_FIELD(int, gradientDirection);
    DECLARE_INSTANCE_FIELD(UnityEngine::Color, startColor);
    DECLARE_INSTANCE_FIELD(UnityEngine::Color, endColor);

    DECLARE_INSTANCE_METHOD(void, SetFilled, bool value);
    DECLARE_INSTANCE_METHOD(void, SetSideCount, int value);
    DECLARE_INSTANCE_METHOD(void, SetBorder, float value);

    DECLARE_INSTANCE_METHOD(void, SetMaskOptions, int type, bool inverse);
    DECLARE_INSTANCE_METHOD(void, SetMaskAmount, float value);

    DECLARE_INSTANCE_METHOD(void, AddColoredVertex, UUI::VertexHelper* vh, UnityEngine::Vector3 pos, UnityEngine::Rect bounds);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPopulateMesh, CAST_METHOD(UUI::Graphic, OnPopulateMesh, UUI::VertexHelper*), UUI::VertexHelper* vh);

    DECLARE_STATIC_METHOD(Shape*, Create, UnityEngine::Transform* parent);

   private:
    HMUI::ImageView* mask = nullptr;
)

DECLARE_CLASS_CODEGEN(Qounters, TextGradient, UnityEngine::MonoBehaviour,
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(int, gradientDirection);
    DECLARE_INSTANCE_FIELD(UnityEngine::Color, startColor);
    DECLARE_INSTANCE_FIELD(UnityEngine::Color, endColor);
    DECLARE_INSTANCE_FIELD_DEFAULT(TMPro::TextMeshProUGUI*, text, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(System::Action_1<TMPro::TMP_TextInfo*>*, delegate, nullptr);

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, OnDisable);

    DECLARE_INSTANCE_METHOD(UnityEngine::Color32, GetColor, UnityEngine::Bounds bounds, UnityEngine::Vector3 vertex);
    DECLARE_INSTANCE_METHOD(void, UpdateGradient);
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

    DECLARE_INSTANCE_FIELD_DEFAULT(UUI::Graphic*, graphic, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::RectTransform*, rectTransform, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(bool, updateColor, false);

   public:
    Options::Premade options;
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

DECLARE_CLASS_CODEGEN(Qounters, ObjectSignal, UnityEngine::MonoBehaviour,
    DECLARE_DEFAULT_CTOR();
    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, OnDisable);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

   public:
    static ObjectSignal* CreateDestroySignal(std::function<void()> onDestroy);

    std::function<void()> onEnable = nullptr;
    std::function<void()> onDisable = nullptr;
    std::function<void()> onDestroy = nullptr;
)

#undef UUI
#undef CAST_METHOD
