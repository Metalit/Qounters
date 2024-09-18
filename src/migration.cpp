#include "migration.hpp"

#include "config.hpp"
#include "editor.hpp"
#include "main.hpp"
#include "sources.hpp"
#include "templates.hpp"

using namespace Qounters;

static std::pair<int, UnityEngine::Vector2> EstimatePosition(std::string position, float distance, Migration::OldConfig const& config) {
    distance /= 2;  // correct for scale difference
    if (position == "BelowCombo") {
        float offset = -12 * (1 - 6 * config.ComboOffset) / 2;  // ComboOffset
        offset += 34;  // difference between my canvas and the combo panel
        offset -= 25;  // correct for pivot location
        return {(int) Options::Group::Anchors::Left, {0, -distance + offset}};
    } else if (position == "AboveCombo") {
        float offset = 40 * (1 - 6 * config.ComboOffset) / 2;
        offset += 34;
        offset += 25;
        return {(int) Options::Group::Anchors::Left, {0, distance + offset}};
    } else if (position == "BelowMultiplier") {
        float offset = -12 * (1 - 6 * config.MultiplierOffset) / 2;
        offset += 27;
        offset -= 25;
        return {(int) Options::Group::Anchors::Right, {0, -distance + offset}};
    } else if (position == "AboveMultiplier") {
        float offset = 40 * (1 - 6 * config.MultiplierOffset) / 2;
        offset += 27;
        offset += 25;
        return {(int) Options::Group::Anchors::Right, {0, distance + offset}};
    } else if (position == "BelowEnergy") {
        float offset = -13;
        offset += 25;
        return {(int) Options::Group::Anchors::Bottom, {0, -distance + offset}};
    } else if (position == "AboveHighway") {
        float offset = 7;
        offset += 25;
        return {(int) Options::Group::Anchors::Top, {0, distance + offset}};
    }
    return {(int) Options::Group::Anchors::Left, {0, 0}};
}

void Migration::Migrate() {
    static std::string const oldConfigPath = "/sdcard/ModData/com.beatgames.beatsaber/Configs/Qounters-.json";

    if (!fileexists(oldConfigPath)) {
        logger.info("No qounters- config found");
        return;
    }

    auto presets = getConfig().Presets.GetValue();
    std::string newPresetName = "Migrated";
    if (presets.contains(newPresetName)) {
        logger.info("Migrated preset found, skipping migration");
        return;
    }

    logger.info("Migrating qounters- config");

    OldConfig config;
    try {
        ReadFromFile(oldConfigPath, config);
    } catch (std::exception const& exc) {
        logger.error("Failed to read old config: {}", exc.what());
        return;
    }

    auto newPreset = Options::GetDefaultHUDPreset();

    if (config.HideMultiplier)
        newPreset.Qounters.erase(newPreset.Qounters.begin() + 2);
    if (config.HideCombo)
        newPreset.Qounters.erase(newPreset.Qounters.begin() + 1);
    if (config.ScoreConfig.Enabled)
        newPreset.Qounters.erase(newPreset.Qounters.begin());

    Editor::SetPresetForMigrating(newPreset);

    if (config.ScoreConfig.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.ScoreConfig.Position, config.ScoreConfig.Distance, config);
        bool score = config.ScoreConfig.Mode != "RemoveScore";
        bool percent = config.ScoreConfig.Mode != "RankOnly";
        bool rank = config.ScoreConfig.Mode != "ScoreOnly";
        Templates::AddScore(anchor, pos, score, percent, rank, config.ScoreConfig.DecimalPrecision, config.ScoreConfig.CustomRankColors);
    }

    if (config.PBConfig.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.PBConfig.Position, config.PBConfig.Distance, config);
        if (config.PBConfig.UnderScore) {
            auto [anchor2, pos2] = config.ScoreConfig.Enabled ? EstimatePosition(config.ScoreConfig.Position, config.ScoreConfig.Distance, config)
                                                              : std::make_pair((int) Options::Group::Anchors::Left, UnityEngine::Vector2(0, -35));
            anchor = anchor2;
            pos = {pos2.x, pos2.y - 15};
        }
        Templates::AddPersonalBest(anchor, pos, config.PBConfig.Mode == "Absolute", config.PBConfig.HideFirstScore, config.PBConfig.DecimalPrecision);
    }

    if (config.CutConfig.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.CutConfig.Position, config.CutConfig.Distance, config);
        Templates::AddAverageCut(
            anchor, pos, config.CutConfig.SeparateSaberCounts, config.CutConfig.SeparateCutValues, config.CutConfig.AveragePrecision
        );
    }

    if (config.NotesLeftConfig.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.NotesLeftConfig.Position, config.NotesLeftConfig.Distance, config);
        Templates::AddNotes(anchor, pos, (int) Sources::Text::Notes::Displays::Remaining, 0);
    }

    if (config.NoteConfig.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.NoteConfig.Position, config.NoteConfig.Distance, config);
        int display = (int) (config.NoteConfig.ShowPercentage ? Sources::Text::Notes::Displays::Percent : Sources::Text::Notes::Displays::Ratio);
        Templates::AddNotes(anchor, pos, display, config.NoteConfig.DecimalPrecision);
    }

    if (config.MissedConfig.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.MissedConfig.Position, config.MissedConfig.Distance, config);
        Templates::AddMistakes(anchor, pos, config.MissedConfig.CountBadCuts, false, false);
    }

    if (config.FailConfig.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.FailConfig.Position, config.FailConfig.Distance, config);
        Templates::AddFails(anchor, pos, config.FailConfig.ShowRestartsInstead);
    }

    if (config.ProgressConfig.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.ProgressConfig.Position, config.ProgressConfig.Distance, config);
        int display = 1;
        if (config.ProgressConfig.Mode == "BaseGame")
            display = 0;
        else if (config.ProgressConfig.Mode == "Percent")
            display = 2;
        Templates::AddSongTime(anchor, pos, display, config.ProgressConfig.ProgressTimeLeft);
    }

    if (config.PPConfig.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.PPConfig.Position, config.PPConfig.Distance, config);
        Templates::AddPP(anchor, pos, false, true, config.PPConfig.HideWhenUnranked, 2);
    }

    if (config.SpeedConfig.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.SpeedConfig.Position, config.SpeedConfig.Distance, config);
        bool split = config.SpeedConfig.Mode == "SplitAverage" || config.SpeedConfig.Mode == "SplitBoth";
        bool last5Secs = config.SpeedConfig.Mode == "Top5Sec";
        Templates::AddSaberSpeed(anchor, pos, split, last5Secs, config.SpeedConfig.DecimalPrecision);
    }

    if (config.Spinometer.Enabled) {
        auto [anchor, pos] = EstimatePosition(config.Spinometer.Position, config.Spinometer.Distance, config);
        bool split = config.Spinometer.Mode == "SplitAverage";
        bool highest = config.Spinometer.Mode == "Highest";
        Templates::AddSpinometer(anchor, pos, split, highest);
    }

    newPreset = Editor::GetPreset();

    if (config.ItalicText) {
        for (auto& group : newPreset.Qounters) {
            for (auto& comp : group.Components) {
                if (comp.Type == (int) Options::Component::Types::Text) {
                    auto opts = *comp.Options.GetValue<Options::Text>();
                    opts.Italic = true;
                    comp.Options = opts;
                }
            }
        }
    }

    presets[newPresetName] = newPreset;

    getConfig().Presets.SetValue(presets);
    if (config.Enabled)
        getConfig().Preset.SetValue(newPresetName);

    logger.info("Done migrating");
}
