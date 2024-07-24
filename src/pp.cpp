#include "pp.hpp"

#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapDifficultySerializedMethods.hpp"
#include "System/Action_1.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "custom-types/shared/delegate.hpp"
#include "environment.hpp"
#include "events.hpp"
#include "main.hpp"
#include "song-details/shared/SongDetails.hpp"
#include "web-utils/shared/WebUtils.hpp"

using namespace GlobalNamespace;
using namespace Qounters;

std::vector<std::pair<double, double>> const PP::BeatLeaderCurve = {
    {1.0, 7.424},    {0.999, 6.241}, {0.9975, 5.158}, {0.995, 4.010}, {0.9925, 3.241}, {0.99, 2.700}, {0.9875, 2.303}, {0.985, 2.007},
    {0.9825, 1.786}, {0.98, 1.618},  {0.9775, 1.490}, {0.975, 1.392}, {0.9725, 1.315}, {0.97, 1.256}, {0.965, 1.167},  {0.96, 1.094},
    {0.955, 1.039},  {0.95, 1.000},  {0.94, 0.931},   {0.93, 0.867},  {0.92, 0.813},   {0.91, 0.768}, {0.9, 0.729},    {0.875, 0.650},
    {0.85, 0.581},   {0.825, 0.522}, {0.8, 0.473},    {0.75, 0.404},  {0.7, 0.345},    {0.65, 0.296}, {0.6, 0.256},    {0.0, 0.000},
};

std::vector<std::pair<double, double>> const PP::ScoreSaberCurve = {
    {1.0, 5.367394282890631},
    {0.9995, 5.019543595874787},
    {0.999, 4.715470646416203},
    {0.99825, 4.325027383589547},
    {0.9975, 3.996793606763322},
    {0.99625, 3.5526145337555373},
    {0.995, 3.2022017597337955},
    {0.99375, 2.9190155639254955},
    {0.9925, 2.685667856592722},
    {0.99125, 2.4902905794106913},
    {0.99, 2.324506282149922},
    {0.9875, 2.058947159052738},
    {0.985, 1.8563887693647105},
    {0.9825, 1.697536248647543},
    {0.98, 1.5702410055532239},
    {0.9775, 1.4664726399289512},
    {0.975, 1.3807102743105126},
    {0.9725, 1.3090333065057616},
    {0.97, 1.2485807759957321},
    {0.965, 1.1552120359501035},
    {0.96, 1.0871883573850478},
    {0.955, 1.0388633331418984},
    {0.95, 1.0},
    {0.94, 0.9417362980580238},
    {0.93, 0.9039994071865736},
    {0.92, 0.8728710341448851},
    {0.91, 0.8488375988124467},
    {0.9, 0.825756123560842},
    {0.875, 0.7816934560296046},
    {0.85, 0.7462290664143185},
    {0.825, 0.7150465663454271},
    {0.8, 0.6872268862950283},
    {0.75, 0.6451808210101443},
    {0.7, 0.6125565959114954},
    {0.65, 0.5866010012767576},
    {0.6, 0.18223233667439062},
    {0.0, 0.},
};

double scoresaberMult = 42.117208413;

BeatmapKey latestRequest = {nullptr, 0, nullptr};
bool PP::blSongValid = false;
PP::BLSongDiff PP::latestBeatleaderSong;
bool PP::ssSongValid = false;
float PP::latestScoresaberSong;

std::vector<std::string> PP::GetModStringsBL(GameplayModifiers* modifiers, bool speeds, bool failed) {
    std::vector<std::string> ret;
    if (modifiers->_disappearingArrows)
        ret.emplace_back("da");
    if (speeds && modifiers->_songSpeed == GameplayModifiers::SongSpeed::Faster)
        ret.emplace_back("fs");
    if (speeds && modifiers->_songSpeed == GameplayModifiers::SongSpeed::Slower)
        ret.emplace_back("ss");
    if (speeds && modifiers->_songSpeed == GameplayModifiers::SongSpeed::SuperFast)
        ret.emplace_back("sf");
    if (modifiers->_ghostNotes)
        ret.emplace_back("gn");
    if (modifiers->_noArrows)
        ret.emplace_back("na");
    if (modifiers->_noBombs)
        ret.emplace_back("nb");
    if (failed && modifiers->_noFailOn0Energy)
        ret.emplace_back("nf");
    if (modifiers->_enabledObstacleType == GameplayModifiers::EnabledObstacleType::NoObstacles)
        ret.emplace_back("no");
    if (modifiers->_proMode)
        ret.emplace_back("pm");
    if (modifiers->_smallCubes)
        ret.emplace_back("sc");
    if (modifiers->_instaFail)
        ret.emplace_back("if");
    if (modifiers->_energyType == GameplayModifiers::EnergyType::Battery)
        ret.emplace_back("be");
    if (modifiers->_strictAngles)
        ret.emplace_back("sa");
    if (modifiers->_zenMode)
        ret.emplace_back("zm");
    return ret;
}

float PP::AccCurve(float acc, std::vector<std::pair<double, double>> const& curve) {
    int i = 1;
    for (; i < curve.size(); i++) {
        if (curve[i].first <= acc)
            break;
    }

    double middle_dis = (acc - curve[i - 1].first) / (curve[i].first - curve[i - 1].first);
    return curve[i - 1].second + middle_dis * (curve[i].second - curve[i - 1].second);
}

std::tuple<float, float, float> CalculatePP(float accuracy, float accRating, float passRating, float techRating) {
    float passPP = passRating > 0 ? 15.2 * std::exp(std::pow(passRating, 1 / 2.62)) - 30 : 0;
    if (passPP < 0)
        passPP = 0;
    float accPP = PP::AccCurve(accuracy, PP::BeatLeaderCurve) * accRating * 34;
    float techPP = std::exp(1.9 * accuracy) * 1.08 * techRating;

    return {passPP, accPP, techPP};
}

float Inflate(float pp) {
    return 650 * std::pow(pp, 1.3) / std::pow(650, 1.3);
}

bool PP::IsRankedBL() {
    if (InSettingsEnvironment())
        return blSongValid;
    // https://github.com/BeatLeader/beatleader-qmod/blob/b5b7dc811f6b39f52451d2dad9ebb70f3ad4ad57/src/UI/LevelInfoUI.cpp#L78
    return blSongValid && latestBeatleaderSong.Stars > 0 && latestBeatleaderSong.RankedStatus == 3;
}

bool PP::IsRankedSS() {
    if (InSettingsEnvironment())
        return ssSongValid;
    return ssSongValid && latestScoresaberSong > 0;
}

float PP::CalculateBL(float percentage, GameplayModifiers* modifiers, bool failed) {
    if (failed || !blSongValid)
        return 0;
    float passRating = latestBeatleaderSong.Pass;
    float accRating = latestBeatleaderSong.Acc;
    float techRating = latestBeatleaderSong.Tech;

    if (InSettingsEnvironment()) {
        // arbitrary but good enough
        passRating = 0.8 * settingsStarsBL;
        accRating = settingsStarsBL;
        techRating = 0.5 * settingsStarsBL;
    } else {
        bool precalculatedSpeeds = latestBeatleaderSong.ModifierRatings.has_value();
        if (precalculatedSpeeds) {
            switch (modifiers->_songSpeed) {
                case GameplayModifiers::SongSpeed::Slower:
                    passRating = latestBeatleaderSong.ModifierRatings->ssPassRating;
                    accRating = latestBeatleaderSong.ModifierRatings->ssAccRating;
                    techRating = latestBeatleaderSong.ModifierRatings->ssTechRating;
                    break;
                case GameplayModifiers::SongSpeed::Faster:
                    passRating = latestBeatleaderSong.ModifierRatings->fsPassRating;
                    accRating = latestBeatleaderSong.ModifierRatings->fsAccRating;
                    techRating = latestBeatleaderSong.ModifierRatings->fsTechRating;
                    break;
                case GameplayModifiers::SongSpeed::SuperFast:
                    passRating = latestBeatleaderSong.ModifierRatings->sfPassRating;
                    accRating = latestBeatleaderSong.ModifierRatings->sfAccRating;
                    techRating = latestBeatleaderSong.ModifierRatings->sfTechRating;
                    break;
                default:
                    break;
            }
        }

        float multiplier = 1;
        auto mods = GetModStringsBL(modifiers, !precalculatedSpeeds, failed);
        for (auto& mod : mods) {
            auto value = latestBeatleaderSong.ModifierValues.find(mod);
            if (value != latestBeatleaderSong.ModifierValues.end())
                multiplier += value->second;
        }
        passRating *= multiplier;
        accRating *= multiplier;
        techRating *= multiplier;
    }

    logger.debug("calculating pp with ratings {} {} {}", passRating, accRating, techRating);

    auto [passPP, accPP, techPP] = CalculatePP(percentage, accRating, passRating, techRating);
    auto rawPP = Inflate(passPP + accPP + techPP);
    return rawPP;
}

float PP::CalculateSS(float percentage, GameplayModifiers* modifiers, bool failed) {
    if (!ssSongValid)
        return 0;
    if (failed)
        percentage /= 2;
    float multiplier = AccCurve(percentage, ScoreSaberCurve);
    float stars = InSettingsEnvironment() ? settingsStarsSS : latestScoresaberSong;
    return multiplier * stars * scoresaberMult;
}

void ProcessResponseBL(PP::BLSong song, BeatmapKey map) {
    logger.debug("processing bl respose");
    std::string characteristic = map.beatmapCharacteristic->serializedName;
    std::string difficulty = BeatmapDifficultySerializedMethods::SerializedName(map.difficulty);

    for (auto& diff : song.Difficulties) {
        if (diff.Characteristic == characteristic && diff.Difficulty == difficulty) {
            logger.debug("found correct difficulty, {:.2f} stars", diff.Stars);
            PP::latestBeatleaderSong = diff;
            PP::blSongValid = true;
            BroadcastEvent((int) Events::MapInfo);
            break;
        }
    }
}

void GetMapInfoBL(BeatmapKey map, std::string hash) {
    std::string url = "https://api.beatleader.xyz/map/hash/" + hash;
    WebUtils::GetAsync<WebUtils::StringResponse>({url, std::string(MOD_ID " " VERSION)}, [map](WebUtils::StringResponse response) {
        if (!response.IsSuccessful() || !response.responseData) {
            logger.error("bl pp request failed {} {}", response.httpCode, response.curlStatus);
            return;
        }
        logger.debug("got bl respose");
        if (!latestRequest.Equals(map))
            return;
        PP::BLSong song;
        try {
            ReadFromString(*response.responseData, song);
            BSML::MainThreadScheduler::Schedule([song = std::move(song), map]() { ProcessResponseBL(song, map); });
        } catch (std::exception const& e) {
            logger.error("failed to parse beatleader response: {}", e.what());
            logger.debug("{}", *response.responseData);
        }
    });
}

SongDetailsCache::SongDetails* songDetailsInstance = nullptr;

void GetSongDetails(auto&& callback) {
    if (songDetailsInstance) {
        callback(songDetailsInstance);
        return;
    }

    logger.debug("initializing song details");

    std::thread([callback]() mutable {
        songDetailsInstance = SongDetailsCache::SongDetails::Init().get();
        callback(songDetailsInstance);
    }).detach();
}

void GetMapInfoSS(BeatmapKey map, std::string hash) {
    std::string characteristic = map.beatmapCharacteristic->serializedName;
    int difficulty = (int) map.difficulty;

    GetSongDetails([hash, characteristic, difficulty](auto details) {
        logger.debug("got song details");
        auto& song = details->songs.FindByHash(hash);
        if (song == SongDetailsCache::Song::none)
            return;
        logger.debug("processing song details");
        auto& diff = song.GetDifficulty((SongDetailsCache::MapDifficulty) difficulty, characteristic);
        if (diff == SongDetailsCache::SongDifficulty::none)
            return;
        logger.debug("found correct difficulty, {:.2f} stars", diff.starsSS);

        BSML::MainThreadScheduler::Schedule([&diff]() {
            PP::latestScoresaberSong = diff.starsSS;
            PP::ssSongValid = true;
            BroadcastEvent((int) Events::MapInfo);
        });
    });
}

void PP::GetMapInfo(BeatmapKey map) {
    blSongValid = false;
    ssSongValid = false;
    latestRequest = map;
    BroadcastEvent((int) Events::MapInfo);

    std::string id = map.levelId;
    static std::string const prefix = "custom_level_";
    if (id.ends_with(" WIP") || !id.starts_with(prefix))
        return;
    std::string hash = id.substr(prefix.size());

    logger.info("Requesting PP info for {}", hash);

    GetMapInfoBL(map, hash);
    GetMapInfoSS(map, hash);
}

void PP::Reset() {
    blSongValid = false;
    ssSongValid = false;
    latestRequest = {nullptr, 0, nullptr};
    BroadcastEvent((int) Events::MapInfo);
}
