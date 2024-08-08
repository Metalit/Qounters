#pragma once

#include "options.hpp"
#include "sources.hpp"

namespace Qounters::Shared {
    void RegisterTemplate(std::string title, TemplateUIFn function);
    void RegisterPremade(std::string mod, std::string name, PremadeFn creation, PremadeUIFn uiFunction = nullptr, PremadeUpdateFn update = nullptr);

    void CloseTemplateModal();
    void AddGroup(Group group);

    int GetActionId();
    void SetSourceOptions(int actionId, UnparsedJSON options);
    void SetColorOptions(int actionId, UnparsedJSON options);
    void SetEnableOptions(int actionId, UnparsedJSON options);
    void FinalizeAction();

    void AddSliderEndDrag(BSML::SliderSetting* slider, std::function<void(float)> onEndDrag);
    void AddStringSettingOnClose(HMUI::InputFieldView* input, std::function<void(std::string)> onKeyboardClosed);

    bool InSettingsEnvironment();
}

// see README
