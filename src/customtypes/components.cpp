#include "customtypes/components.hpp"
#include "config.hpp"
#include "main.hpp"
#include "qounters.hpp"
#include "utils.hpp"

#include <filesystem>

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

#include "UnityEngine/UI/Image_OriginHorizontal.hpp"
#include "UnityEngine/UI/Image_OriginVertical.hpp"
#include "UnityEngine/UI/Image_Origin360.hpp"

void Shape::SetMaskOptions(int type, bool inverse) {
    if (!mask)
        return;

    switch ((ShapeOptions::Fills) type) {
        case ShapeOptions::Fills::None:
            // could remove it from the source shapes map as well, but it's probably insignificant
            mask->set_type(UI::Image::Type::Simple);
            return;
        case ShapeOptions::Fills::Horizontal:
            mask->set_fillMethod(UI::Image::FillMethod::Horizontal);
            mask->set_fillOrigin(inverse
                ? UI::Image::OriginHorizontal::Right
                : UI::Image::OriginHorizontal::Left);
            break;
        case ShapeOptions::Fills::Vertical:
            mask->set_fillMethod(UI::Image::FillMethod::Vertical);
            mask->set_fillOrigin(inverse
                ? UI::Image::OriginVertical::Bottom
                : UI::Image::OriginVertical::Top);
            break;
        case ShapeOptions::Fills::Circle:
            mask->set_fillMethod(UI::Image::FillMethod::Radial360);
            mask->set_fillOrigin(UI::Image::Origin360::Top);
            mask->set_fillClockwise(!inverse);
            break;
    }
    mask->set_type(UI::Image::Type::Filled);
}

void Shape::SetMaskAmount(float value) {
    if (!mask)
        return;
    mask->set_fillAmount(value);
}

#include "UnityEngine/Rect.hpp"

// centering a triangle bc I'm too lazy to size the outline
// regular (unit equil.) triangle height is sqrt(3)/2 ~ 0.866
// circumcircle height is 2/sqrt(3) ~ 1.1547
// circumcircle centered in square has top at (2/sqrt(3) - 1)/2 + 1 ~ 1.07735
// average of centered circumcircle and regular triangle is ((2/sqrt(3) - 1)/2 + 1 + sqrt(3)/2)/2 ~ 0.97169
// height of base is that - sqrt(3)/2 ~ 0.10566

const std::map<int, std::vector<Vector2>> fixedPoints = {
    {3, {{0, 0.10566}, {0.5, 0.97169}, {1, 0.10566}}}, // doesn't work with outline T-T
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

    for(int i = 0; i < sides; i++) {
        double currentRadian = radianProgressPerStep * i;
        ret.emplace_back(bounds.m_XMin + (std::cos(currentRadian) / 2 + 0.5) * bounds.m_Width, bounds.m_YMin + (std::sin(currentRadian) / 2 + 0.5) * bounds.m_Height);
    }
    return ret;
}

std::vector<std::tuple<int, int, int>> GetFilledTriangles(std::vector<Vector2> points) {
    std::vector<std::tuple<int, int, int>> ret;
    ret.reserve(points.size() - 2);

    for(int i = 0; i < points.size() - 2; i++)
        ret.emplace_back(0, i + 2, i + 1);

    return ret;
}

std::vector<std::tuple<int, int, int>> GetHollowTriangles(std::vector<Vector2> points) {
    std::vector<std::tuple<int, int, int>> ret;
    ret.reserve(points.size());

    int half = points.size() / 2;
    for(int i = 0; i < half; i++) {
        int outerIndex = i;
        int innerIndex = i + half;

        ret.emplace_back(outerIndex, innerIndex, (i + 1) % half);
        ret.emplace_back(outerIndex, half + ((i + half - 1) % half), innerIndex);
    }
    return ret;
}

Color32 ToColor32(Color color) {
    return Color32(color.r * 255, color.g * 255, color.b * 255, color.a * 255);
}

#include "UnityEngine/RectTransform.hpp"

// curved ui can probably be done as simple as {curvedCanvasRadius, 0} in AddVert uv2
void Shape::OnPopulateMesh(UI::VertexHelper *vh) {
    auto bounds = GetPixelAdjustedRect();
    auto scale = get_transform()->get_lossyScale();

    bool hollowAndSpace = !filled && bounds.m_Width > border && bounds.m_Height > border;

    float borderX = border * 5 * 0.02 / scale.x;
    float borderY = border * 5 * 0.02 / scale.y;

    std::vector<Vector2> points;
    auto color = ToColor32(get_color());

    vh->Clear();
    for (auto& [x, y] : GetCircumferencePoints(sideCount, bounds)) {
        vh->AddVert({x, y, 0}, color, {});
        points.emplace_back(x, y);
    }
    if (hollowAndSpace) {
        auto innerBounds = bounds;
        innerBounds.m_XMin += borderX * 0.5;
        innerBounds.m_YMin += borderY * 0.5;
        innerBounds.m_Width -= borderX;
        innerBounds.m_Height -= borderY;
        for (auto& [x, y] : GetCircumferencePoints(sideCount, innerBounds)) {
            vh->AddVert({x, y, 0}, color, {});
            points.emplace_back(x, y);
        }
    }

    auto triangles = hollowAndSpace ? GetHollowTriangles(points) : GetFilledTriangles(points);
    for (auto& [p0, p1, p2] : triangles)
        vh->AddTriangle(p0, p1, p2);
}

#include "questui/shared/BeatSaberUI.hpp"

using namespace QuestUI;

#include "UnityEngine/UI/Mask.hpp"

Shape* Shape::Create(Transform* parent) {
    // TODO: cache and convert to asset
    auto sprite = QuestUI::BeatSaberUI::Base64ToSprite("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAIAAACQd1PeAAAACXBIWXMAAAABAAAAAQBPJcTWAAAADElEQVR4nGP4//8/AAX+Av4N70a4AAAAAElFTkSuQmCC");
    auto maskImage = BeatSaberUI::CreateImage(parent, sprite);

    auto mask = maskImage->get_gameObject()->AddComponent<UI::Mask*>();
    mask->set_showMaskGraphic(false);

    auto shapeObject = GameObject::New_ctor("QountersShape");
    auto ret = shapeObject->AddComponent<Shape*>();

    ret->mask = maskImage;
    ret->set_material(maskImage->get_material());

    auto transform = ret->get_rectTransform();
    transform->SetParent(maskImage->get_transform(), false);
    transform->set_anchorMin({0, 0});
    transform->set_anchorMax({1, 1});
    transform->set_sizeDelta({0, 0});

    return ret;
}

#include "GlobalNamespace/ScoreMultiplierUIController.hpp"
#include "GlobalNamespace/SongProgressUIController.hpp"
#include "GlobalNamespace/GameEnergyUIPanel.hpp"

void CopyFields(Transform* base, Transform* target, int component) {
    switch ((BaseGameOptions::Components) component) {
        case BaseGameOptions::Components::Multiplier: {
            auto baseComp = base->GetComponent<ScoreMultiplierUIController*>();
            auto targetComp = target->GetComponent<ScoreMultiplierUIController*>();
            targetComp->scoreController = baseComp->scoreController;
            break;
        }
        case BaseGameOptions::Components::ProgressBar: {
            auto baseComp = base->GetComponent<SongProgressUIController*>();
            auto targetComp = target->GetComponent<SongProgressUIController*>();
            targetComp->audioTimeSource = baseComp->audioTimeSource;
            break;
        }
        case BaseGameOptions::Components::HealthBar: {
            auto baseComp = base->GetComponent<GameEnergyUIPanel*>();
            auto targetComp = target->GetComponent<GameEnergyUIPanel*>();
            targetComp->gameEnergyCounter = baseComp->gameEnergyCounter;
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
    auto slider = (RectTransform*) songTimeInstance->Find("Slider");
    auto handle = (RectTransform*) slider->Find("Handle Slide Area/Handle");
    return slider->get_rect().m_Width + handle->get_rect().m_Width;
}

void BaseGameGraphic::SetComponent(int comp) {
    component = comp;
    if (instance)
        Object::Destroy(instance->get_gameObject());

    getLogger().debug("set component %i", comp);

    auto base = clones[component];
    if (!base || !base->m_CachedPtr)
        return;
    instance = Object::Instantiate(base);

    CopyFields(base, instance, component);

    instance->SetParent(get_transform(), false);
    instance->get_gameObject()->SetActive(true);

    auto rect = Utils::GetOrAddComponent<RectTransform*>(instance);
    rect->set_anchorMin({0.5, 0.5});
    rect->set_anchorMax({0.5, 0.5});
    rect->set_anchoredPosition({0, 0});

    switch ((BaseGameOptions::Components) component) {
        case BaseGameOptions::Components::Multiplier:
            get_rectTransform()->set_sizeDelta({50, 50});
            instance->set_localScale({0.5, 0.5, 0.5});
            break;
        case BaseGameOptions::Components::ProgressBar:
            get_rectTransform()->set_sizeDelta({GetTimerWidth(instance), 20});
            instance->set_localScale({1, 1, 1});
            break;
        case BaseGameOptions::Components::HealthBar:
            get_rectTransform()->set_sizeDelta({125, 10});
            instance->set_localScale({1, 1, 1});
            break;
    }
    instance->set_localEulerAngles({});

    graphics = instance->GetComponentsInChildren<UI::Graphic*>();
    SetChildColors();
}

void BaseGameGraphic::SetChildColors() {
    if (!graphics || !instance)
        return;

    auto color = get_color();
    float origAlhpa = color.a;

    for (auto graphic : graphics) {
        auto path = Utils::GetTransformPath(instance, graphic->get_transform());
        if (!alphaIndex[component].contains(path))
            continue;
        color.a = origAlhpa * alphaIndex[component][path];
        graphic->set_color(color);
    }
}

void BaseGameGraphic::OnPopulateMesh(UI::VertexHelper *vh) {
    vh->Clear();
    updateChildren = true;
}

BaseGameGraphic* BaseGameGraphic::Create(Transform* parent) {
    auto obj = GameObject::New_ctor("QountersBaseGameComponent");
    obj->get_transform()->SetParent(parent, false);
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
            getLogger().error("Failed to find base component %i", i);
            clones[i] = nullptr;
            continue;
        }
        clones[i] = Object::Instantiate(base);

        clones[i]->get_gameObject()->SetActive(false);
        clones[i]->set_name("QountersBaseGameClone" + std::to_string(i));

        if (auto qounters = clones[i]->Find("QountersCanvas"))
            Object::Destroy(qounters->get_gameObject());

        if (i == (int) BaseGameOptions::Components::Multiplier)
            clones[i]->Find("BGCircle")->get_gameObject()->SetActive(true);

        // non SerializeField fields don't get copied on instantiate
        CopyFields(base, clones[i], i);

        alphaIndex[i] = {};
        auto graphics = base->GetComponentsInChildren<UI::Graphic*>();
        for (auto graphic : graphics) {
            std::string path = Utils::GetTransformPath(base, graphic->get_transform());
            alphaIndex[i][path] = graphic->get_color().a;
            getLogger().debug("%.2f alpha for %i %s", graphic->get_color().a, i, path.c_str());
        }
    }
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
        instance->sprites = List<Sprite*>::New_ctor();
    }
    return instance;
}

void ImageSpriteCache::LoadAllSprites() {
    for (auto& file : std::filesystem::directory_iterator(IMAGE_DIRECTORY))
        GetSprite(file.path().filename().string());
}

Sprite* ImageSpriteCache::GetSprite(std::string name) {
    getLogger().debug("loading sprite %s", name.c_str());
    auto inst = GetInstance();
    for (int i = 0; i < inst->spritePaths.size(); i++) {
        if (inst->spritePaths[i] == name)
            return inst->sprites->get_Item(i);
    }
    inst->spritePaths.emplace_back(name);
    auto ret = BeatSaberUI::FileToSprite(IMAGE_DIRECTORY + name);
    inst->sprites->Add(ret);
    return ret;
}

int ImageSpriteCache::NumberOfSprites() {
    return GetInstance()->sprites->get_Count();
}

Sprite* ImageSpriteCache::GetSpriteIdx(int spriteIdx) {
    return GetInstance()->sprites->get_Item(spriteIdx);
}
