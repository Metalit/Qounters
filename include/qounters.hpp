#pragma once

#include "config.hpp"
#include "sources.hpp"

namespace Qounters {
    enum class HUDType {
        Basic,
        Rotational,
        Multiplayer,
        Unsupported,
    };

    struct PremadeInfo {
        std::string name;
        PremadeFn creation;
        PremadeUIFn uiFunction;
        PremadeUpdateFn update;
        PremadeInfo(std::string_view name, PremadeFn creation, PremadeUIFn uiFunction = nullptr, PremadeUpdateFn update = nullptr) :
            name(name),
            creation(creation),
            uiFunction(uiFunction),
            update(update) {}
    };
    // map with mod name for ordering purposes
    extern std::map<std::string, std::vector<PremadeInfo>> premadeRegistry;
    PremadeInfo* GetPremadeInfo(std::string const& mod, std::string const& name);

    void UpdateComponentOptions(int componentType, UnityEngine::Component* component, Component::OptionsTypes newOptions);
    void UpdateComponentColor(UnityEngine::UI::Graphic* component, std::string newSource, UnparsedJSON newOptions, GradientOptions gradientOptions);
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
    void Reset(bool sceneEnd = true);
    void SetupObjects();

    void UpdateSource(Sources sourceType, std::string source);
    void UpdateAllEnables();
    void UpdateAllSources();
}
