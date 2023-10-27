#pragma once

#include "options.hpp"
#include "sources.hpp"
#include "utils.hpp"

namespace Qounters::Shared {
    void RegisterTemplate(std::string title, TemplateUIFn function);

    void CloseTemplateModal();
    void AddGroup(Group group);

    int GetActionId();
    void SetSourceOptions(int actionId, UnparsedJSON options);
    void SetColorOptions(int actionId, UnparsedJSON options);
    void FinalizeAction();

    void AddSliderEndDrag(QuestUI::SliderSetting *slider, std::function<void ()> onEndDrag);
}

// see README
