#include "api.hpp"

#include "editor.hpp"
#include "environment.hpp"
#include "playtest.hpp"
#include "sources.hpp"
#include "templates.hpp"
#include "utils.hpp"

using namespace Qounters;

void API::RegisterTemplate(std::string section, std::string title, Types::TemplateUIFn function) {
    Templates::registration.emplace_back(title, function);
}
void API::RegisterPremade(std::string mod, std::string name, Types::PremadeFn creation, Types::PremadeUIFn uiFunction, Types::PremadeUpdateFn update) {
    if (!Sources::premades.contains(mod))
        Sources::premades[mod] = {};
    Sources::premades[mod].emplace_back(name, creation, uiFunction, update);
}

void API::CloseTemplateModal() {
    Templates::CloseModal();
}
void API::AddGroup(Options::Group group) {
    Editor::AddGroup(group);
}

int API::GetActionId() {
    return Editor::GetActionId();
}
void API::SetSourceOptions(int actionId, UnparsedJSON options) {
    Editor::SetSourceOptions(actionId, options);
}
void API::SetColorOptions(int actionId, UnparsedJSON options) {
    Editor::SetColorOptions(actionId, options);
}
void API::SetEnableOptions(int actionId, UnparsedJSON options) {
    Editor::SetEnableOptions(actionId, options);
}
void API::FinalizeAction() {
    Editor::FinalizeAction();
}

BSML::ColorSetting* API::CreateColorPicker(
    UnityEngine::GameObject* parent,
    std::string name,
    UnityEngine::Color value,
    std::function<void(UnityEngine::Color)> onChange,
    std::function<void()> onClose
) {
    return Utils::CreateColorPicker(parent, name, value, onChange, onClose);
}

float API::GetPlaytestPB() {
    return Playtest::GetOverridePBRatio();
}

bool API::InSettings() {
    return Environment::InSettings();
}

bool API::IsInstalled() {
    return true;
}
