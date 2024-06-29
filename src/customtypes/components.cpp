#include "customtypes/components.hpp"

#include "GlobalNamespace/GameEnergyUIPanel.hpp"
#include "GlobalNamespace/ScoreMultiplierUIController.hpp"
#include "GlobalNamespace/SongProgressUIController.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/UI/Mask.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "config.hpp"
#include "main.hpp"
#include "qounters.hpp"
#include "utils.hpp"

DEFINE_TYPE(Qounters, Shape);
DEFINE_TYPE(Qounters, BaseGameGraphic);
DEFINE_TYPE(Qounters, ImageSpriteCache);

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace Qounters;

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

    switch ((ShapeOptions::Fills) type) {
        case ShapeOptions::Fills::None:
            // could remove it from the source shapes map as well, but it's probably insignificant
            mask->type = UI::Image::Type::Simple;
            return;
        case ShapeOptions::Fills::Horizontal:
            mask->fillMethod = UI::Image::FillMethod::Horizontal;
            mask->fillOrigin = (int) (inverse ? UI::Image::OriginHorizontal::Right : UI::Image::OriginHorizontal::Left);
            break;
        case ShapeOptions::Fills::Vertical:
            mask->fillMethod = UI::Image::FillMethod::Vertical;
            mask->fillOrigin = (int) (inverse ? UI::Image::OriginVertical::Bottom : UI::Image::OriginVertical::Top);
            break;
        case ShapeOptions::Fills::Circle:
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

std::map<int, std::vector<Vector2>> const fixedPoints = {
    {3, {{0, 0.10566}, {0.5, 0.97169}, {1, 0.10566}}},  // doesn't work with outline T-T
    {4, {{0, 0}, {0, 1}, {1, 1}, {1, 0}}},
};

std::vector<Vector2> GetCircumferencePoints(int sides, Rect& bounds) {
    std::vector<Vector2> ret = {};

    if (fixedPoints.contains(sides)) {
        auto& base = fixedPoints.at(sides);
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

Color32 ToColor32(Color color) {
    return Color32::op_Implicit___UnityEngine__Color32(color);
}

// curved ui can probably be done as simple as {curvedCanvasRadius, 0} in AddVert uv2
void Shape::OnPopulateMesh(UI::VertexHelper* vh) {
    auto bounds = GetPixelAdjustedRect();
    auto scale = transform->lossyScale;

    bool hollowAndSpace = !filled && bounds.m_Width > border && bounds.m_Height > border;

    float borderX = border * 5 * 0.02 / scale.x;
    float borderY = border * 5 * 0.02 / scale.y;

    std::vector<Vector2> points;
    auto color32 = ToColor32(color);

    vh->Clear();
    for (auto& [x, y] : GetCircumferencePoints(sideCount, bounds)) {
        vh->AddVert({x, y, 0}, color32, {0, 0, 0, 0});
        points.emplace_back(x, y);
    }
    if (hollowAndSpace) {
        auto innerBounds = bounds;
        innerBounds.m_XMin += borderX * 0.5;
        innerBounds.m_YMin += borderY * 0.5;
        innerBounds.m_Width -= borderX;
        innerBounds.m_Height -= borderY;
        for (auto& [x, y] : GetCircumferencePoints(sideCount, innerBounds)) {
            vh->AddVert({x, y, 0}, color32, {0, 0, 0, 0});
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

void CopyFields(Transform* base, Transform* target, int component) {
    switch ((BaseGameOptions::Components) component) {
        case BaseGameOptions::Components::Multiplier: {
            auto baseComp = base->GetComponent<ScoreMultiplierUIController*>();
            auto targetComp = target->GetComponent<ScoreMultiplierUIController*>();
            targetComp->_scoreController = baseComp->_scoreController;
            break;
        }
        case BaseGameOptions::Components::ProgressBar: {
            auto baseComp = base->GetComponent<SongProgressUIController*>();
            auto targetComp = target->GetComponent<SongProgressUIController*>();
            targetComp->_audioTimeSource = baseComp->_audioTimeSource;
            break;
        }
        case BaseGameOptions::Components::HealthBar: {
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

float GetTimerWidth(Transform* songTimeInstance) {
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

    switch ((BaseGameOptions::Components) component) {
        case BaseGameOptions::Components::Multiplier:
            rectTransform->sizeDelta = {50, 50};
            instance->localScale = {0.5, 0.5, 0.5};
            break;
        case BaseGameOptions::Components::ProgressBar:
            rectTransform->sizeDelta = {GetTimerWidth(instance), 20};
            instance->localScale = {1, 1, 1};
            break;
        case BaseGameOptions::Components::HealthBar:
            rectTransform->sizeDelta = {125, 10};
            instance->localScale = {1, 1, 1};
            break;
    }
    instance->localEulerAngles = {};

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

Transform* GetBase(int component) {
    auto hud = GetHUD().first;
    if (!hud)
        return nullptr;
    switch ((BaseGameOptions::Components) component) {
        case BaseGameOptions::Components::Multiplier:
            return Utils::FindRecursive(hud, "MultiplierCanvas");
        case BaseGameOptions::Components::ProgressBar:
            return Utils::FindRecursive(hud, "SongProgressCanvas");
        case BaseGameOptions::Components::HealthBar:
            return Utils::FindRecursive(hud, "EnergyPanel");
    }
}

void BaseGameGraphic::MakeClones() {
    for (int i = 0; i <= (int) BaseGameOptions::Components::ComponentsMax; i++) {
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

        if (i == (int) BaseGameOptions::Components::Multiplier)
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
    for (int i = 0; i <= (int) BaseGameOptions::Components::ComponentsMax; i++)
        clones[i] = nullptr;
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
