#include "main.hpp"

#include "assets.hpp"
#include "bsml/shared/BSML.hpp"
#include "config.hpp"
#include "environment.hpp"
#include "gameplay.hpp"
#include "hooks.hpp"
#include "migration.hpp"
#include "scotland2/shared/modloader.h"

using namespace Qounters;

static modloader::ModInfo modInfo = {MOD_ID, VERSION, 0};

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
        presets[defaultPreset] = Options::GetDefaultHUDPreset();
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
    custom_types::Register::AutoRegister();
    BSML::Init();

    BSML::Register::RegisterMenuButton("Qounters++", "Qounters++ Settings", Environment::PresentSettings);
    BSML::Register::RegisterGameplaySetupTab("Qounters++", Gameplay::GameplaySetupMenu, BSML::MenuType::All);

    logger.info("Installing hooks");
    Hooks::Install();

    logger.info("Writing default images");
    writefile(IMAGE_DIRECTORY "Beatleader.png", IncludedAssets::Beatleader_png);
    writefile(IMAGE_DIRECTORY "Default.png", IncludedAssets::Default_png);
    writefile(IMAGE_DIRECTORY "Scoresaber.png", IncludedAssets::Scoresaber_png);
}
