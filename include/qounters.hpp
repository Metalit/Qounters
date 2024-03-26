#pragma once

#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/UI/Graphic.hpp"

#include "sources.hpp"
#include "config.hpp"

namespace Qounters {
    enum class HUDType {
        Basic,
        Narrow,
        Lattice,
        Rock,
        Rotational,
        Multiplayer,
        Unsupported,
    };

    void UpdateComponentOptions(int componentType, UnityEngine::Component* component, Component::OptionsTypes newOptions);
    void UpdateComponentColor(UnityEngine::UI::Graphic* component, std::string newSource, UnparsedJSON newOptions);
    void UpdateComponentEnabled(UnityEngine::GameObject* component, std::string newSource, UnparsedJSON newOptions, bool invert);

    void UpdateComponentPosition(UnityEngine::RectTransform* component, Component const& qounterComponent);
    void UpdateGroupPosition(UnityEngine::RectTransform* group, Group const& qounterGroup);

    void RemoveComponent(int componentType, UnityEngine::Component* component);

    void SetSourceOptions(Component& component, UnparsedJSON newOptions);
    void SetDefaultOptions(Component& component);

    std::pair<UnityEngine::Transform*, HUDType> GetHUD();
    UnityEngine::Transform* GetAnchor(int anchor);

    void CreateQounterComponent(Component const& qounterComponent, int componentIdx, UnityEngine::Transform* parent, bool editing);
    void CreateQounterGroup(Group const& qounterGroup, int groupIdx, bool editing);
    void CreateQounters();
    void Reset();
    void SetupObjects();

    void UpdateSource(Sources sourceType, std::string source);
    void UpdateAllEnables();
}
