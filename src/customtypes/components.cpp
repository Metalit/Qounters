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

#include <cmath>

#include "UnityEngine/Rect.hpp"

std::vector<UnityEngine::Vector3> GetCircumferencePoints(int sides, UnityEngine::Rect& bounds) {
    std::vector<UnityEngine::Vector3> ret = {};
    double circumferenceProgressPerStep = 1 / (double) sides;
    double radianProgressPerStep = circumferenceProgressPerStep * 2 * M_PI;

    for(int i = 0; i < sides; i++) {
        double currentRadian = radianProgressPerStep * i;
        ret.emplace_back(bounds.m_XMin + std::cos(currentRadian) * bounds.m_Width, bounds.m_YMin + std::sin(currentRadian) * bounds.m_Height, 0);
    }
    return ret;
}

std::vector<std::tuple<int, int, int>> GetFilledTriangles(std::vector<UnityEngine::Vector3> points) {
    std::vector<std::tuple<int, int, int>> ret;
    ret.reserve(points.size() - 2);

    for(int i = 0; i < points.size() - 2; i++)
        ret.emplace_back(0, i + 2, i + 1);

    return ret;
}

std::vector<std::tuple<int, int, int>> GetHollowTriangles(std::vector<UnityEngine::Vector3> points) {
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
void Shape::OnPopulateMesh(UnityEngine::UI::VertexHelper *vh) {
    // TODO: oh god I have to fill it myself oh no

    auto bounds = GetPixelAdjustedRect();
    bool hollowAndSpace = !filled && bounds.m_Width > border && bounds.m_Height > border;

    int sides = 0;

    switch ((ShapeType) shape) {
        case Shape::Circle:
            sides = 50;
            break;
        case Shape::Square:
            sides = 4;
            break;
        case Shape::Triangle:
            sides = 3;
            break;
    }

    std::vector<UnityEngine::Vector3> points;
    auto color = ToColor32(get_color());

    vh->Clear();
    for (auto& point : GetCircumferencePoints(sides, bounds)) {
        vh->AddVert(point, color, {});
        points.emplace_back(point);
    }
    if (hollowAndSpace) {
        // TODO: fancy math to get the border to refer to the width instead of the corner diagonals? or does it??
        auto innerBounds = bounds;
        innerBounds.m_XMin += border * 0.5;
        innerBounds.m_YMin += border * 0.5;
        innerBounds.m_Width -= border;
        innerBounds.m_Height -= border;
        for (auto& point : GetCircumferencePoints(sides, innerBounds)) {
            vh->AddVert(point, color, {});
            points.emplace_back(point);
        }
    }

    auto triangles = hollowAndSpace ? GetHollowTriangles(points) : GetFilledTriangles(points);
    for (auto& [p0, p1, p2] : triangles)
        vh->AddTriangle(p0, p1, p2);
}

// #include "questui/shared/BeatSaberUI.hpp"

// using namespace QuestUI;

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Material.hpp"

Shape* Shape::Create(UnityEngine::GameObject* object, int shape, bool filled, float borderWidth) {
    auto ret = object->AddComponent<Shape*>();

    auto mat = Resources::FindObjectsOfTypeAll<Material*>().First([](auto x) { return x->get_name() == "UINoGlow"; });
    ret->set_material(mat);

    ret->shape = shape;
    ret->filled = filled;
    ret->border = borderWidth;

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

std::array<UnityEngine::Transform*, BaseGameGraphic::cloneCount> BaseGameGraphic::clones = {};
std::array<std::map<std::string, float>, BaseGameGraphic::cloneCount> BaseGameGraphic::alphaIndex = {};

void BaseGameGraphic::Update() {
    if (updateChildren)
        SetChildColors();
    updateChildren = false;
}

void BaseGameGraphic::SetComponent(int comp) {
    component = comp;
    if (instance)
        Object::Destroy(instance->get_gameObject());

    getLogger().debug("set component %i", comp);

    instance = Object::Instantiate(clones[component]);

    CopyFields(clones[component], instance, component);

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
            get_rectTransform()->set_sizeDelta({52.5, 20});
            instance->set_localScale({1, 1, 1});
            break;
        case BaseGameOptions::Components::HealthBar:
            get_rectTransform()->set_sizeDelta({125, 10});
            instance->set_localScale({1, 1, 1});
            break;
    }

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
    switch ((BaseGameOptions::Components) component) {
        case BaseGameOptions::Components::Multiplier:
            return GetAnchor((int) Group::Anchors::Right)->get_parent()->Find("MultiplierCanvas");
        case BaseGameOptions::Components::ProgressBar:
            return GetAnchor((int) Group::Anchors::Right)->get_parent()->Find("SongProgressCanvas");
        case BaseGameOptions::Components::HealthBar:
            return GetAnchor((int) Group::Anchors::Bottom)->get_parent();
    }
}

void BaseGameGraphic::MakeClones() {
    for (int i = 0; i <= (int) BaseGameOptions::Components::ComponentsMax; i++) {
        auto base = GetBase(i);
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
    auto ret = QuestUI::BeatSaberUI::FileToSprite(IMAGE_DIRECTORY + name);
    inst->sprites->Add(ret);
    return ret;
}

int ImageSpriteCache::NumberOfSprites() {
    return GetInstance()->sprites->get_Count();
}

Sprite* ImageSpriteCache::GetSpriteIdx(int spriteIdx) {
    return GetInstance()->sprites->get_Item(spriteIdx);
}
