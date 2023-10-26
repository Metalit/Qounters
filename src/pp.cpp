#include "pp.hpp"
#include "events.hpp"

#include "main.hpp"
#include "rapidjson-macros/shared/macros.hpp"
#include <exception>

DECLARE_JSON_CLASS(BLModifiers,
    VALUE(float, da)
    VALUE(float, fs)
    VALUE(float, sf)
    VALUE(float, ss)
    VALUE(float, gn)
    VALUE(float, na)
    VALUE(float, nb)
    VALUE(float, nf)
    VALUE(float, no)
    VALUE(float, pm)
    VALUE(float, sc)
    VALUE(float, sa)
    VALUE(float, op)
)

DECLARE_JSON_CLASS(BLSongDiff,
    NAMED_VALUE(std::string, Difficulty, "difficultyName")
    NAMED_VALUE(std::string, Characteristic, "modeName")
    NAMED_VALUE_DEFAULT(int, RankedStatus, 0, "status")
    NAMED_VALUE_DEFAULT(float, Stars, 0, "stars")
    NAMED_VALUE_DEFAULT(float, Predicted, 0, "predictedAcc")
    NAMED_VALUE_DEFAULT(float, Pass, 0, "passRating")
    NAMED_VALUE_DEFAULT(float, Acc, 0, "accRating")
    NAMED_VALUE_DEFAULT(float, Tech, 0, "techRating")
)

DECLARE_JSON_CLASS(BLSong,
    NAMED_VECTOR(BLSongDiff, Difficulties, "difficulties")
)

std::vector<std::pair<double, double>> beatleader = {
    {1.0, 7.424},
    {0.999, 6.241},
    {0.9975, 5.158},
    {0.995, 4.010},
    {0.9925, 3.241},
    {0.99, 2.700},
    {0.9875, 2.303},
    {0.985, 2.007},
    {0.9825, 1.786},
    {0.98, 1.618},
    {0.9775, 1.490},
    {0.975, 1.392},
    {0.9725, 1.315},
    {0.97, 1.256},
    {0.965, 1.167},
    {0.96, 1.094},
    {0.955, 1.039},
    {0.95, 1.000},
    {0.94, 0.931},
    {0.93, 0.867},
    {0.92, 0.813},
    {0.91, 0.768},
    {0.9, 0.729},
    {0.875, 0.650},
    {0.85, 0.581},
    {0.825, 0.522},
    {0.8, 0.473},
    {0.75, 0.404},
    {0.7, 0.345},
    {0.65, 0.296},
    {0.6, 0.256},
    {0.0, 0.000},
};

std::vector<std::pair<double, double>> scoresaber = {
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

using namespace GlobalNamespace;

IDifficultyBeatmap* latestRequest;
bool blSongValid = false;
BLSongDiff latestBeatleaderSong;
bool ssSongValid = false;
float latestScoresaberSong;

float AccCurve(float acc, std::vector<std::pair<double, double>>& curve) {
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
    float accPP = AccCurve(accuracy, beatleader) * accRating * 34;
    float techPP = std::exp(1.9 * accuracy) * 1.08 * techRating;

    return {passPP, accPP, techPP};
}

float Inflate(float pp) {
    return 650 * std::pow(pp, 1.3) / std::pow(650, 1.3);
}

bool Qounters::PP::IsRankedBL() {
    // https://github.com/BeatLeader/beatleader-qmod/blob/b5b7dc811f6b39f52451d2dad9ebb70f3ad4ad57/src/UI/LevelInfoUI.cpp#L78
    return blSongValid && latestBeatleaderSong.Stars > 0 && latestBeatleaderSong.RankedStatus == 3;
}

bool Qounters::PP::IsRankedSS() {
    return ssSongValid && latestScoresaberSong > 0;
}

float Qounters::PP::CalculateBL(float percentage, bool failed) {
    if (failed || !blSongValid)
        return 0;
    // TODO: speed modifier custom ratings
    float accRating = latestBeatleaderSong.Acc;
    float passRating = latestBeatleaderSong.Pass;
    float techRating = latestBeatleaderSong.Tech;
    // TODO: custom modifier values
    auto [passPP, accPP, techPP] = CalculatePP(percentage, accRating, passRating, techRating);
    auto rawPP = Inflate(passPP + accPP + techPP);
    return rawPP;
}

float Qounters::PP::CalculateSS(float percentage, bool failed) {
    if (!ssSongValid)
        return 0;
    if (failed)
        percentage /= 2;
    float multiplier = AccCurve(percentage, scoresaber);
    return multiplier * latestScoresaberSong * scoresaberMult;
}

#include "GlobalNamespace/IDifficultyBeatmapSet.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapDifficultySerializedMethods.hpp"

void ProcessResponseBL(BLSong song, IDifficultyBeatmap* map) {
    getLogger().debug("processing bl respose");
    std::string characteristic = map->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName;
    std::string difficulty = BeatmapDifficultySerializedMethods::SerializedName(map->get_difficulty());

    for (auto& diff : song.Difficulties) {
        if (diff.Characteristic == characteristic && diff.Difficulty == difficulty) {
            getLogger().debug("found correct difficulty, %.2f stars", diff.Stars);
            latestBeatleaderSong = diff;
            blSongValid = true;
            Qounters::BroadcastEvent((int) Qounters::Events::PPInfo);
            break;
        }
    }
}

#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "UnityEngine/Networking/UnityWebRequest.hpp"
#include "UnityEngine/Networking/UnityWebRequestAsyncOperation.hpp"
#include "UnityEngine/Networking/DownloadHandler.hpp"
#include "System/Action_1.hpp"

#include "custom-types/shared/delegate.hpp"

using DownloadCompleteDelegate = System::Action_1<UnityEngine::AsyncOperation*>*;

void GetMapInfoBL(IDifficultyBeatmap* map, std::string hash) {
    auto request = UnityEngine::Networking::UnityWebRequest::Get("api.beatleader.xyz/map/hash/" + hash);
    getLogger().debug("bl url: api.beatleader.xyz/map/hash/%s", hash.c_str());
    request->SetRequestHeader("User-Agent", MOD_ID " " VERSION);
    auto completed = custom_types::MakeDelegate<DownloadCompleteDelegate>((std::function<void (UnityEngine::AsyncOperation*)>) [map, request](UnityEngine::AsyncOperation* op) {
        getLogger().debug("got bl respose");
        if (map != latestRequest)
            return;
        std::string responseString = request->get_downloadHandler()->GetText();
        BLSong response;
        try {
            ReadFromString(responseString, response);
            ProcessResponseBL(response, map);
        } catch (std::exception const& e) {
            getLogger().error("failed to parse beatleader response: %s", e.what());
            getLogger().error("%s", responseString.c_str());
        }
    });
    request->SendWebRequest()->add_completed(completed);
}

#include "song-details/shared/SongDetails.hpp"

SongDetailsCache::SongDetails* songDetailsInstance = nullptr;

void GetSongDetails(auto&& callback) {
    if (songDetailsInstance) {
        callback(songDetailsInstance);
        return;
    }

    getLogger().debug("initializing song details");

    std::thread([callback]() mutable {
        songDetailsInstance = SongDetailsCache::SongDetails::Init().get();
        callback(songDetailsInstance);
    }).detach();
}

#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

void GetMapInfoSS(IDifficultyBeatmap* map, std::string hash) {
    std::string characteristic = map->get_parentDifficultyBeatmapSet()->get_beatmapCharacteristic()->serializedName;
    int difficulty = map->get_difficulty();

    GetSongDetails([hash, characteristic, difficulty](auto details) {
        getLogger().debug("got song details");
        auto& song = details->songs.FindByHash(hash);
        if (song == SongDetailsCache::Song::none)
            return;
        getLogger().debug("processing song details");
        auto& diff = song.GetDifficulty((SongDetailsCache::MapDifficulty) difficulty, characteristic);
        if (diff == SongDetailsCache::SongDifficulty::none)
            return;
        getLogger().debug("found correct difficulty, %.2f stars", diff.starsSS);

        QuestUI::MainThreadScheduler::Schedule([&diff]() {
            latestScoresaberSong = diff.starsSS;
            ssSongValid = true;
            Qounters::BroadcastEvent((int) Qounters::Events::PPInfo);
        });
    });
}

void Qounters::PP::GetMapInfo(IDifficultyBeatmap* map) {
    blSongValid = false;
    ssSongValid = false;
    latestRequest = map;

    std::string id = map->get_level()->i_IPreviewBeatmapLevel()->get_levelID();
    static const std::string prefix = "custom_level_";
    if (id.ends_with(" WIP") || !id.starts_with(prefix))
        return;
    std::string hash = id.substr(prefix.size());

    getLogger().info("Requesting PP info for %s", hash.c_str());

    GetMapInfoBL(map, hash);
    GetMapInfoSS(map, hash);
}
