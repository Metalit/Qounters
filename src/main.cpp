#include "main.hpp"
#include "environment.hpp"
#include "hooks.hpp"
#include "config.hpp"
#include "migration.hpp"

#include <filesystem>

bool hasCJD = false;

Logger& getLogger() {
    static auto logger = new Logger(ModInfo{MOD_ID, VERSION}, LoggerOptions{false, true});
    return *logger;
}

extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;

    getConfig().Init(info);

    auto presets = getConfig().Presets.GetValue();
    auto defaultPreset = getConfig().Preset.GetDefaultValue();
    if (!presets.contains(defaultPreset)) {
        presets[defaultPreset] = Qounters::GetDefaultHUDPreset();
        getConfig().Presets.SetValue(presets);
    }

    if (!getConfig().Migrated.GetValue()) {
        getConfig().Migrated.SetValue(true);
        Qounters::Migration::Migrate();
    }

    std::filesystem::create_directories(IMAGE_DIRECTORY);
}

#include "questui/shared/QuestUI.hpp"
#include "bsml/shared/BSML.hpp"

extern "C" void load() {
    il2cpp_functions::Init();
    QuestUI::Init();
    BSML::Init();

    BSML::Register::RegisterMenuButton("Qounters++", "Qounters++ Settings", Qounters::PresentSettingsEnvironment);

    hasCJD = Modloader::requireMod("CustomJSONData");

    getLogger().debug("Installing hooks");
    Qounters::InstallHooks();
}
