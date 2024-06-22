#include "main.hpp"

#include "bsml/shared/BSML.hpp"
#include "config.hpp"
#include "environment.hpp"
#include "hooks.hpp"
#include "migration.hpp"
#include "scotland2/shared/modloader.h"

static modloader::ModInfo modInfo = {MOD_ID, VERSION, 0};

bool hasCJD = false;
GlobalNamespace::IConnectedPlayer* localFakeConnectedPlayer = nullptr;
bool blockOtherRaycasts = false;
std::unordered_set<UnityEngine::Canvas*> raycastCanvases = {};

extern "C" void setup(CModInfo* info) {
    Paper::Logger::RegisterFileContextId(MOD_ID);

    *info = modInfo.to_c();

    getConfig().Init(modInfo);

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

extern "C" void late_load() {
    il2cpp_functions::Init();
    BSML::Init();

    BSML::Register::RegisterMenuButton("Qounters++", "Qounters++ Settings", Qounters::PresentSettingsEnvironment);

    // hasCJD = Modloader::requireMod("CustomJSONData");

    logger.info("Installing hooks");
    Qounters::InstallHooks();
}
