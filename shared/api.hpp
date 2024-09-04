#pragma once

#include "options.hpp"
#include "sources.hpp"

namespace Qounters::API {
    void RegisterTemplate(std::string title, Types::TemplateUIFn function);
    void RegisterPremade(
        std::string mod, std::string name, Types::PremadeFn creation, Types::PremadeUIFn uiFunction = nullptr, Types::PremadeUpdateFn update = nullptr
    );

    void CloseTemplateModal();
    void AddGroup(Options::Group group);

    int GetActionId();
    void SetSourceOptions(int actionId, UnparsedJSON options);
    void SetColorOptions(int actionId, UnparsedJSON options);
    void SetEnableOptions(int actionId, UnparsedJSON options);
    void FinalizeAction();

    void AddSliderEndDrag(BSML::SliderSetting* slider, std::function<void(float)> onEndDrag);
    void AddStringSettingOnClose(HMUI::InputFieldView* input, std::function<void(std::string)> onKeyboardClosed);

    bool InSettings();
}

// see README
