#pragma once

#include "config.hpp"
#include "sources.hpp"

namespace Qounters::HUD {
    enum class Type {
        Basic,
        Rotational,
        Multiplayer,
        Unsupported,
    };

    void UpdateComponentOptions(int componentType, UnityEngine::Component* component, Options::Component::OptionsTypes newOptions);
    void UpdateComponentColor(UnityEngine::UI::Graphic* component, std::string newSource, UnparsedJSON newOptions, Options::Gradient gradientOptions);
    void UpdateComponentEnabled(UnityEngine::GameObject* component, std::string newSource, UnparsedJSON newOptions, bool invert);

    void UpdateComponentPosition(UnityEngine::RectTransform* component, Options::Component const& qounterComponent);
    void UpdateGroupPosition(UnityEngine::RectTransform* group, Options::Group const& qounterGroup);

    void RemoveComponent(int componentType, UnityEngine::Component* component);

    void SetSourceOptions(Options::Component& component, UnparsedJSON newOptions);
    void SetDefaultOptions(Options::Component& component);

    std::pair<UnityEngine::Transform*, Type> GetHUD();
    UnityEngine::Transform* GetAnchor(int anchor);

    void CreateQounterComponent(Options::Component const& qounterComponent, int componentIdx, UnityEngine::Transform* parent, bool editing);
    void CreateQounterGroup(Options::Group const& qounterGroup, int groupIdx, bool editing);
    void CreateQounters();
    void Reset(bool sceneEnd = true);
    void SetupObjects();

    void UpdateSource(Types::Sources sourceType, std::string source);
    void UpdateAllEnables();
    void UpdateAllSources();
}
