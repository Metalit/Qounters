#pragma once

#include "options.hpp"
#include "sources.hpp"

namespace Qounters::Shared {
    void RegisterTemplate(std::string title, TemplateUIFn function);

    void CloseTemplateModal();
    void AddGroup(Group group);

    int GetActionId();
    void SetSourceOptions(int actionId, UnparsedJSON options);
    void SetColorOptions(int actionId, UnparsedJSON options);
    void FinalizeAction();

    void AddSliderEndDrag(QuestUI::SliderSetting* slider, std::function<void ()> onEndDrag);
    void AddStringSettingOk(HMUI::InputFieldView* input, std::function<void ()> onOkPressed);
}

// see README
