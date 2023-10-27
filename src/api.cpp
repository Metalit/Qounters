#include "api.hpp"
#include "editor.hpp"
#include "main.hpp"
#include "templates.hpp"
#include "utils.hpp"

namespace Qounters::Shared {
    void RegisterTemplate(std::string title, TemplateUIFn function) {
        templates.emplace_back(title, function);
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
    void FinalizeAction() {
        Editor::FinalizeAction();
    }

    void AddSliderEndDrag(QuestUI::SliderSetting *slider, std::function<void ()> onEndDrag) {
        Utils::AddSliderEndDrag(slider, onEndDrag);
    }
}
