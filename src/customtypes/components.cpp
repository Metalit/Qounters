#include "customtypes/components.hpp"

#include "GlobalNamespace/GameEnergyUIPanel.hpp"
#include "GlobalNamespace/ScoreMultiplierUIController.hpp"
#include "GlobalNamespace/SongProgressUIController.hpp"
#include "TMPro/TMP_TextInfo.hpp"
#include "TMPro/TMP_VertexDataUpdateFlags.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/UI/Mask.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/Helpers/delegates.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "config.hpp"
#include "internals.hpp"
#include "main.hpp"
#include "options.hpp"
#include "qounters.hpp"
#include "utils.hpp"

DEFINE_TYPE(Qounters, Shape);
DEFINE_TYPE(Qounters, TextGradient);
DEFINE_TYPE(Qounters, BaseGameGraphic);
DEFINE_TYPE(Qounters, PremadeParent);
DEFINE_TYPE(Qounters, SongTimeSource);
DEFINE_TYPE(Qounters, ImageSpriteCache);
DEFINE_TYPE(Qounters, ObjectSignal);

using namespace Qounters;
using namespace GlobalNamespace;
using namespace UnityEngine;

void Shape::SetFilled(bool value) {
    filled = value;
    SetVerticesDirty();
}

void Shape::SetSideCount(int value) {
    sideCount = value;
    SetVerticesDirty();
}

void Shape::SetBorder(float value) {
    border = value;
    SetVerticesDirty();
}

void Shape::SetMaskOptions(int type, bool inverse) {
    if (!mask)
        return;

    switch ((Options::Shape::Fills) type) {
        case Options::Shape::Fills::None:
            // could remove it from the source shapes map as well, but it's probably insignificant
            mask->type = UI::Image::Type::Simple;
            return;
        case Options::Shape::Fills::Horizontal:
            mask->fillMethod = UI::Image::FillMethod::Horizontal;
            mask->fillOrigin = (int) (inverse ? UI::Image::OriginHorizontal::Right : UI::Image::OriginHorizontal::Left);
            break;
        case Options::Shape::Fills::Vertical:
            mask->fillMethod = UI::Image::FillMethod::Vertical;
            mask->fillOrigin = (int) (inverse ? UI::Image::OriginVertical::Bottom : UI::Image::OriginVertical::Top);
            break;
        case Options::Shape::Fills::Circle:
            mask->fillMethod = UI::Image::FillMethod::Radial360;
            mask->fillOrigin = (int) UI::Image::Origin360::Top;
            mask->fillClockwise = !inverse;
            break;
    }
    mask->type = UI::Image::Type::Filled;
}

void Shape::SetMaskAmount(float value) {
    if (!mask)
        return;
    mask->fillAmount = value;
}

// centering a triangle bc I'm too lazy to size the outline
// regular (unit equil.) triangle height is sqrt(3)/2 ~ 0.866
// circumcircle height is 2/sqrt(3) ~ 1.1547
// circumcircle centered in square has top at (2/sqrt(3) - 1)/2 + 1 ~ 1.07735
// average of centered circumcircle and regular triangle is ((2/sqrt(3) - 1)/2 + 1 + sqrt(3)/2)/2 ~ 0.97169
// height of base is that - sqrt(3)/2 ~ 0.10566

static std::map<int, std::vector<Vector2>> const FixedPoints = {
    {3, {{0, 0.10566}, {0.5, 0.97169}, {1, 0.10566}}},  // doesn't work with outline T-T
    {4, {{0, 0}, {0, 1}, {1, 1}, {1, 0}}},
};

std::vector<Vector2> GetCircumferencePoints(int sides, Rect& bounds) {
    std::vector<Vector2> ret = {};

    if (FixedPoints.contains(sides)) {
        auto& base = FixedPoints.at(sides);
        for (auto& [x, y] : base)
            ret.emplace_back(bounds.m_XMin + x * bounds.m_Width, bounds.m_YMin + y * bounds.m_Height);
        return ret;
    }

    double circumferenceProgressPerStep = 1 / (double) sides;
    double radianProgressPerStep = circumferenceProgressPerStep * 2 * M_PI;

    for (int i = 0; i < sides; i++) {
        double currentRadian = radianProgressPerStep * i;
        ret.emplace_back(
            bounds.m_XMin + (std::cos(currentRadian) / 2 + 0.5) * bounds.m_Width,
            bounds.m_YMin + (std::sin(currentRadian) / 2 + 0.5) * bounds.m_Height
        );
    }
    return ret;
}

std::vector<std::tuple<int, int, int>> GetFilledTriangles(std::vector<Vector2> points) {
    std::vector<std::tuple<int, int, int>> ret;
    ret.reserve(points.size() - 2);

    for (int i = 0; i < points.size() - 2; i++)
        ret.emplace_back(0, i + 2, i + 1);

    return ret;
}

std::vector<std::tuple<int, int, int>> GetHollowTriangles(std::vector<Vector2> points) {
    std::vector<std::tuple<int, int, int>> ret;
    ret.reserve(points.size());

    int half = points.size() / 2;
    for (int i = 0; i < half; i++) {
        int outerIndex = i;
        int innerIndex = i + half;

        ret.emplace_back(outerIndex, innerIndex, (i + 1) % half);
        ret.emplace_back(outerIndex, half + ((i + half - 1) % half), innerIndex);
    }
    return ret;
}

// curved ui can probably be done as simple as {curvedCanvasRadius, 0} in AddVert uv2
void Shape::AddColoredVertex(UI::VertexHelper* vh, Vector3 pos, Rect bounds) {
    Color32 color32;
    if (gradient) {
        // start should be left/top imo
        float lerpValue = gradientDirection == (int) Options::Gradient::Directions::Horizontal ? (pos.x - bounds.m_XMin) / bounds.m_Width
                                                                                               : 1 - (pos.y - bounds.m_YMin) / bounds.m_Height;
        color32 = Color32::op_Implicit___UnityEngine__Color32(Color::Lerp(startColor, endColor, lerpValue));
    } else
        color32 = Color32::op_Implicit___UnityEngine__Color32(color);
    vh->AddVert(pos, color32, {0, 0, 0, 0});
}

void Shape::OnPopulateMesh(UI::VertexHelper* vh) {
    auto bounds = GetPixelAdjustedRect();
    auto scale = transform->lossyScale;

    bool hollowAndSpace = !filled && bounds.m_Width > border && bounds.m_Height > border;

    float borderX = border * 5 * 0.02 / scale.x;
    float borderY = border * 5 * 0.02 / scale.y;

    std::vector<Vector2> points;
    auto color32 = Color32::op_Implicit___UnityEngine__Color32(color);

    vh->Clear();
    for (auto& [x, y] : GetCircumferencePoints(sideCount, bounds)) {
        AddColoredVertex(vh, {x, y, 0}, bounds);
        points.emplace_back(x, y);
    }
    if (hollowAndSpace) {
        auto innerBounds = bounds;
        innerBounds.m_XMin += borderX * 0.5;
        innerBounds.m_YMin += borderY * 0.5;
        innerBounds.m_Width -= borderX;
        innerBounds.m_Height -= borderY;
        for (auto& [x, y] : GetCircumferencePoints(sideCount, innerBounds)) {
            AddColoredVertex(vh, {x, y, 0}, bounds);
            points.emplace_back(x, y);
        }
    }

    auto triangles = hollowAndSpace ? GetHollowTriangles(points) : GetFilledTriangles(points);
    for (auto& [p0, p1, p2] : triangles)
        vh->AddTriangle(p0, p1, p2);
}

Shape* Shape::Create(Transform* parent) {
    auto maskImage = BSML::Lite::CreateImage(parent, BSML::Utilities::ImageResources::GetWhitePixel());

    auto mask = maskImage->gameObject->AddComponent<UI::Mask*>();
    mask->showMaskGraphic = false;

    auto shapeObject = GameObject::New_ctor("QountersShape");
    auto ret = shapeObject->AddComponent<Shape*>();

    ret->mask = maskImage;
    ret->material = maskImage->material;

    auto transform = ret->rectTransform;
    transform->SetParent(maskImage->transform, false);
    transform->anchorMin = {0, 0};
    transform->anchorMax = {1, 1};
    transform->sizeDelta = {0, 0};

    return ret;
}

void TextGradient::OnEnable() {
    if (!delegate) {
        delegate = BSML::MakeSystemAction<TMPro::TMP_TextInfo*>((std::function<void(TMPro::TMP_TextInfo*)>) [this](TMPro::TMP_TextInfo*) {
            UpdateGradient();
        });
    }
    text = GetComponent<TMPro::TextMeshProUGUI*>();
    if (text) {
        text->remove_OnPreRenderText(delegate);
        text->add_OnPreRenderText(delegate);
        UpdateGradient();
    }
}

void TextGradient::OnDisable() {
    if (text) {
        if (delegate)
            text->remove_OnPreRenderText(delegate);
        text->SetVerticesDirty();
    }
}

Color32 TextGradient::GetColor(Bounds bounds, Vector3 vertex) {
    float lerpAmount = gradientDirection == (int) Options::Gradient::Directions::Horizontal ? (vertex.x - bounds.m_Center.x) / bounds.m_Extents.x
                                                                                            : (vertex.y - bounds.m_Center.y) / -bounds.m_Extents.y;
    // (-1, 1) -> (0, 1)
    lerpAmount = (lerpAmount + 1) * 0.5;
    return Color32::op_Implicit___UnityEngine__Color32(Color::Lerp(startColor, endColor, lerpAmount));
}

void TextGradient::UpdateGradient() {
    if (!text || text->text == "" || !text->textInfo)
        return;
    auto bounds = text->bounds;
    auto info = text->textInfo;
    int chars = info->characterCount;
    for (auto& charInfo : info->characterInfo) {
        if (!charInfo.isVisible)
            continue;
        auto& colors = info->meshInfo[charInfo.materialReferenceIndex].colors32;
        int startVertex = charInfo.vertexIndex;
        // idk if this is the right order but it works
        colors[startVertex] = GetColor(bounds, charInfo.bottomLeft);
        colors[startVertex + 1] = GetColor(bounds, charInfo.topLeft);
        colors[startVertex + 2] = GetColor(bounds, charInfo.topRight);
        colors[startVertex + 3] = GetColor(bounds, charInfo.bottomRight);
    }
    text->UpdateVertexData(TMPro::TMP_VertexDataUpdateFlags::Colors32);
}

static void CopyFields(Transform* base, Transform* target, int component) {
    switch ((BaseGameGraphic::Objects) component) {
        case BaseGameGraphic::Objects::Multiplier: {
            auto baseComp = base->GetComponent<ScoreMultiplierUIController*>();
            auto targetComp = target->GetComponent<ScoreMultiplierUIController*>();
            targetComp->_scoreController = baseComp->_scoreController;
            break;
        }
        case BaseGameGraphic::Objects::ProgressBar: {
            auto baseComp = base->GetComponent<SongProgressUIController*>();
            auto targetComp = target->GetComponent<SongProgressUIController*>();
            targetComp->_audioTimeSource = (IAudioTimeSource*) CRASH_UNLESS(il2cpp_utils::New<SongTimeSource*>());
            break;
        }
        case BaseGameGraphic::Objects::HealthBar: {
            auto baseComp = base->GetComponent<GameEnergyUIPanel*>();
            auto targetComp = target->GetComponent<GameEnergyUIPanel*>();
            targetComp->_gameEnergyCounter = baseComp->_gameEnergyCounter;
            break;
        }
    }
}

std::array<Transform*, BaseGameGraphic::cloneCount> BaseGameGraphic::clones = {};
std::array<std::map<std::string, float>, BaseGameGraphic::cloneCount> BaseGameGraphic::alphaIndex = {};

void BaseGameGraphic::Update() {
    if (updateChildren)
        SetChildColors();
    updateChildren = false;
}

static float GetTimerWidth(Transform* songTimeInstance) {
    auto slider = songTimeInstance->Find("Slider").cast<RectTransform>();
    auto handle = slider->Find("Handle Slide Area/Handle").cast<RectTransform>();
    return slider->rect.m_Width + handle->rect.m_Width;
}

void BaseGameGraphic::SetComponent(int comp) {
    component = comp;
    if (instance)
        Object::Destroy(instance->gameObject);

    logger.debug("set component {}", comp);

    auto base = clones[component];
    if (!base || !base->m_CachedPtr.m_value)
        return;
    instance = Object::Instantiate(base);

    CopyFields(base, instance, component);

    instance->SetParent(transform, false);
    instance->gameObject->active = true;

    auto rect = Utils::GetOrAddComponent<RectTransform*>(instance);
    rect->anchorMin = {0.5, 0.5};
    rect->anchorMax = {0.5, 0.5};
    rect->anchoredPosition = {0, 0};

    switch ((Objects) component) {
        case Objects::Multiplier:
            rectTransform->sizeDelta = {50, 50};
            instance->localScale = {0.5, 0.5, 0.5};
            break;
        case Objects::ProgressBar:
            rectTransform->sizeDelta = {GetTimerWidth(instance), 20};
            instance->localScale = {1, 1, 1};
            break;
        case Objects::HealthBar:
            rectTransform->sizeDelta = {125, 10};
            instance->localScale = {1, 1, 1};
            break;
    }
    instance->localEulerAngles = {0, 0, 0};

    graphics = instance->GetComponentsInChildren<UI::Graphic*>();
    SetChildColors();
}

void BaseGameGraphic::SetChildColors() {
    if (!graphics || !instance)
        return;

    auto modColor = color;
    float origAlhpa = modColor.a;

    for (auto graphic : graphics) {
        auto path = Utils::GetTransformPath(instance, graphic->transform);
        if (!alphaIndex[component].contains(path))
            continue;
        modColor.a = origAlhpa * alphaIndex[component][path];
        graphic->color = modColor;
    }
}

void BaseGameGraphic::OnPopulateMesh(UI::VertexHelper* vh) {
    vh->Clear();
    updateChildren = true;
}

BaseGameGraphic* BaseGameGraphic::Create(Transform* parent) {
    auto obj = GameObject::New_ctor("QountersBaseGameComponent");
    obj->transform->SetParent(parent, false);
    return obj->AddComponent<BaseGameGraphic*>();
}

static Transform* GetBase(int component) {
    auto hud = HUD::GetHUD().first;
    if (!hud)
        return nullptr;
    switch ((BaseGameGraphic::Objects) component) {
        case BaseGameGraphic::Objects::Multiplier:
            return Utils::FindRecursive(hud, "MultiplierCanvas");
        case BaseGameGraphic::Objects::ProgressBar:
            return Utils::FindRecursive(hud, "SongProgressCanvas");
        case BaseGameGraphic::Objects::HealthBar:
            return Utils::FindRecursive(hud, "EnergyPanel");
    }
}

void BaseGameGraphic::MakeClones() {
    for (int i = 0; i <= (int) Objects::ComponentsMax; i++) {
        auto base = GetBase(i);
        if (!base) {
            logger.error("Failed to find base component {}", i);
            clones[i] = nullptr;
            continue;
        }
        clones[i] = Object::Instantiate(base);

        clones[i]->gameObject->active = false;
        clones[i]->name = "QountersBaseGameClone" + std::to_string(i);

        if (auto qounters = clones[i]->Find("QountersCanvas"))
            Object::Destroy(qounters->gameObject);

        if (i == (int) Objects::Multiplier)
            clones[i]->Find("BGCircle")->gameObject->active = true;

        // non SerializeField fields don't get copied on instantiate
        CopyFields(base, clones[i], i);

        alphaIndex[i] = {};
        auto graphics = base->GetComponentsInChildren<UI::Graphic*>();
        for (auto graphic : graphics) {
            std::string path = Utils::GetTransformPath(base, graphic->transform);
            alphaIndex[i][path] = graphic->color.a;
            logger.debug("{:.2f} alpha for {} {}", graphic->color.a, i, path.c_str());
        }
    }
}

void BaseGameGraphic::Reset() {
    for (int i = 0; i <= (int) Objects::ComponentsMax; i++)
        clones[i] = nullptr;
}

void PremadeParent::Update() {
    if (!GetGraphic())
        return;
    if (!rectTransform)
        rectTransform = Utils::GetOrAddComponent<RectTransform*>(this);
    rectTransform->sizeDelta = graphic->rectTransform->sizeDelta;
    if (updateColor)
        graphic->color = color;
}

void PremadeParent::OnPopulateMesh(UI::VertexHelper* vh) {
    vh->Clear();
    updateColor = true;
}

UI::Graphic* PremadeParent::GetGraphic() {
    if (!graphic && transform->GetChildCount() > 0)
        graphic = transform->GetChild(0)->GetComponent<UI::Graphic*>();
    return graphic;
}

float SongTimeSource::get_songTime() {
    return Internals::songTime;
}

float SongTimeSource::get_lastFrameDeltaSongTime() {
    return Time::get_deltaTime();
}

float SongTimeSource::get_songEndTime() {
    return Internals::songLength;
}

float SongTimeSource::get_songLength() {
    return Internals::songLength;
}

bool SongTimeSource::get_isReady() {
    return true;
}

ImageSpriteCache* ImageSpriteCache::instance;

void ImageSpriteCache::OnDestroy() {
    instance = nullptr;
}

ImageSpriteCache* ImageSpriteCache::GetInstance() {
    if (!instance) {
        auto go = GameObject::New_ctor("QountersImageSpriteCache");
        Object::DontDestroyOnLoad(go);
        instance = go->AddComponent<ImageSpriteCache*>();
        instance->sprites = ListW<Sprite*>::New();
    }
    return instance;
}

void ImageSpriteCache::LoadAllSprites() {
    for (auto& file : std::filesystem::directory_iterator(IMAGE_DIRECTORY))
        GetSprite(file.path().filename().string());
}

Sprite* ImageSpriteCache::GetSprite(std::string name) {
    logger.debug("loading sprite {}", name);
    auto inst = GetInstance();
    for (int i = 0; i < inst->spritePaths.size(); i++) {
        if (inst->spritePaths[i] == name)
            return inst->sprites[i];
    }
    std::string path = IMAGE_DIRECTORY + name;
    if (!fileexists(path)) {
        logger.error("sprite {} not in image directory! (" IMAGE_DIRECTORY ")", name);
        return nullptr;
    }
    inst->spritePaths.emplace_back(name);
    auto ret = BSML::Lite::FileToSprite(path);
    inst->sprites->Add(ret);
    return ret;
}

int ImageSpriteCache::NumberOfSprites() {
    return GetInstance()->sprites.size();
}

Sprite* ImageSpriteCache::GetSpriteIdx(int spriteIdx) {
    return GetInstance()->sprites[spriteIdx];
}

void ObjectSignal::OnEnable() {
    if (onEnable)
        onEnable();
}

void ObjectSignal::OnDisable() {
    if (onDisable)
        onDisable();
}

void ObjectSignal::OnDestroy() {
    if (onDestroy)
        onDestroy();
}

ObjectSignal* ObjectSignal::CreateDestroySignal(std::function<void()> onDestroy) {
    auto ret = GameObject::New_ctor("QountersDestroySignal")->AddComponent<ObjectSignal*>();
    ret->onDestroy = onDestroy;
    return ret;
}
