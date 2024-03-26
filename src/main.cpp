#include "main.hpp"
#include "environment.hpp"
#include "hooks.hpp"
#include "config.hpp"
#include "migration.hpp"

#include "scotland2/shared/modloader.h"

#include <filesystem>

bool hasCJD = false;
GlobalNamespace::IConnectedPlayer* localFakeConnectedPlayer = nullptr;
bool blockOtherRaycasts = false;
std::unordered_set<UnityEngine::Canvas*> raycastCanvases = {};

extern "C" void setup(CModInfo* info) {
    info->id = MOD_ID;
    info->version = VERSION;
    info->version_long = 0;

    getConfig().Init(modloader::ModInfo{MOD_ID, VERSION, 0});

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

#include "bsml/shared/BSML.hpp"

#include "scotland2/shared/loader.hpp"

extern "C" void late_load() {
    il2cpp_functions::Init();
    BSML::Init();

    BSML::Register::RegisterMenuButton("Qounters++", "Qounters++ Settings", Qounters::PresentSettingsEnvironment);

    /*auto loaded = modloader:modloader::get_loaded();
    if(std::find_if(loaded.begin(), loaded.end(), [](const modloader::ModResult &arg) {
        if(std::holds_alternative<modloader::ModData>(arg)) {
            auto modData = std::get<modloader::ModData>(arg);
            return modData.info.id == "CustomJSONData";
        }
        return false;
    }) != loaded.end()) {
        hasCJD = true;
    }*/

    QountersLogger::Logger.debug("Installing hooks");
    Qounters::InstallHooks();
}
