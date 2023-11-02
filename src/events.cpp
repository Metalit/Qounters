#include "events.hpp"
#include "config.hpp"
#include "qounters.hpp"
#include "sources.hpp"

namespace Qounters {
    std::map<int, std::vector<std::pair<Sources, std::string>>> eventSourceRegistry = {
        {(int) Events::ScoreChanged, {
            {Sources::Text, TextSource::ScoreName},
            {Sources::Text, TextSource::RankName},
            {Sources::Text, TextSource::PPName},
            {Sources::Shape, ShapeSource::ScoreName},
            {Sources::Color, ColorSource::RankName},
            {Sources::Color, ColorSource::PersonalBestName},
        }},
        {(int) Events::NoteCut, {
            {Sources::Text, TextSource::AverageCutName},
            {Sources::Text, TextSource::MistakesName},
            {Sources::Text, TextSource::NotesName},
            {Sources::Shape, ShapeSource::AverageCutName},
            {Sources::Shape, ShapeSource::NotesName},
        }},
        {(int) Events::NoteMissed, {
            {Sources::Text, TextSource::MistakesName},
            {Sources::Text, TextSource::NotesName},
            {Sources::Shape, ShapeSource::NotesName},
        }},
        {(int) Events::BombCut, {
            {Sources::Text, TextSource::MistakesName},
            {Sources::Text, TextSource::NotesName},
            {Sources::Shape, ShapeSource::NotesName},
        }},
        {(int) Events::WallHit, {
            {Sources::Text, TextSource::MistakesName},
        }},
        {(int) Events::ComboChanged, {
            {Sources::Text, TextSource::ComboName},
            {Sources::Text, TextSource::MultiplierName},
            {Sources::Shape, ShapeSource::MultiplierName},
            {Sources::Color, ColorSource::ComboName},
            {Sources::Color, ColorSource::MultiplierName},
        }},
        {(int) Events::HealthChanged, {
            {Sources::Text, TextSource::HealthName},
            {Sources::Text, TextSource::FailsName},
            {Sources::Shape, ShapeSource::HealthName},
            {Sources::Color, ColorSource::HealthName},
        }},
        {(int) Events::Update, {
            {Sources::Text, TextSource::TimeName},
            {Sources::Shape, ShapeSource::TimeName},
        }},
        {(int) Events::SlowUpdate, {
            {Sources::Text, TextSource::SaberSpeedName},
        }},
        {(int) Events::PPInfo, {
            {Sources::Text, TextSource::PPName},
        }},
    };

    std::map<std::string, std::map<int, int>> customEvents = {};

    int RegisterCustomEvent(std::string mod, int event) {
        int maxEvent = (int) Events::EventMax;
        for (auto& [_, idMap] : customEvents) {
            for (auto& [_, realId] : idMap)
                maxEvent = std::max(realId, maxEvent);
        }
        if (!customEvents.contains(mod))
            customEvents[mod] = {};
        customEvents[mod][event] = maxEvent + 1;
        return maxEvent + 1;
    }

    void RegisterToEvent(Sources sourceType, std::string source, int event) {
        if (!eventSourceRegistry.contains(event))
            eventSourceRegistry[event] = {};
        eventSourceRegistry[event].emplace_back(sourceType, source);
    }

    void BroadcastEvent(int event) {
        if (!eventSourceRegistry.contains(event))
            return;
        for (auto& [sourceType, source] : eventSourceRegistry[event])
            UpdateSource(sourceType, source);
    }

    void BroadcastEvent(std::string mod, int event) {
        if (!customEvents.contains(mod))
            return;
        if (!customEvents[mod].contains(event))
            return;
        BroadcastEvent(customEvents[mod][event]);
    }
}
