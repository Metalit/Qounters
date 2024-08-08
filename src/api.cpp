#include "api.hpp"

#include "editor.hpp"
#include "environment.hpp"
#include "main.hpp"
#include "qounters.hpp"
#include "templates.hpp"
#include "utils.hpp"

namespace Qounters::Shared {
    void RegisterTemplate(std::string title, TemplateUIFn function) {
        templates.emplace_back(title, function);
    }
    void RegisterPremade(std::string mod, std::string name, PremadeFn creation, PremadeUIFn uiFunction, PremadeUpdateFn update) {
        if (!premadeRegistry.contains(mod))
            premadeRegistry[mod] = {};
        premadeRegistry[mod].emplace_back(name, creation, uiFunction, update);
    }

    void CloseTemplateModal() {
        Templates::CloseModal();
    }
    void AddGroup(Group group) {
        Editor::AddGroup(group);
    }

    int GetActionId() {
        return Editor::GetActionId();
    }
    void SetSourceOptions(int actionId, UnparsedJSON options) {
        Editor::SetSourceOptions(actionId, options);
    }
    void SetColorOptions(int actionId, UnparsedJSON options) {
        Editor::SetColorOptions(actionId, options);
    }
    void SetEnableOptions(int actionId, UnparsedJSON options) {
        Editor::SetEnableOptions(actionId, options);
    }
    void FinalizeAction() {
        Editor::FinalizeAction();
    }

    void AddSliderEndDrag(BSML::SliderSetting* slider, std::function<void(float)> onEndDrag) {
        Utils::AddSliderEndDrag(slider, onEndDrag);
    }
    void AddStringSettingOnClose(HMUI::InputFieldView* input, std::function<void(std::string)> onKeyboardClosed) {
        Utils::AddStringSettingOnClose(input, onKeyboardClosed);
    }

    bool InSettingsEnvironment() {
        return Qounters::InSettingsEnvironment();
    }
}
