#include "Settings.h"

#include <SKSE/SKSE.h>

#include <BMK/Settings.h>
#include <CLIBUtil/simpleINI.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Settings {
namespace {
    constexpr auto kSection = "CombatMusic";
    constexpr auto kDebugLoggingKey = "bDebugLogging";
    constexpr auto kOnlyAtSaveLoadKey = "bOnlyAtSaveLoad";
    constexpr auto kStopTracksKey = "StopTracks";
    constexpr auto kDefaultStopTracks
        = "MUScombat, MUScombatCivilWar, MUScombatBossUmbra, MUScombatBossDLC1, MUScombatBossChargen, MUScombatBoss, "
          "DLC2MUScombatKarstaag, DLC2MUScombatBoss";

    std::vector<std::string> ParseCommands(const std::string_view a_value) {
        std::vector<std::string> commands;
        std::stringstream entries {std::string(a_value)};
        std::string track;
        while (std::getline(entries, track, ',')) {
            std::erase_if(track, [](const unsigned char a_character) { return std::isspace(a_character) != 0; });
            if (!track.empty()) {
                commands.emplace_back("removemusic " + track);
            }
        }
        return commands;
    }

    Values MakeDefaults() {
        return {.commands = ParseCommands(kDefaultStopTracks)};
    }

    Values& Current() {
        static auto current = MakeDefaults();
        return current;
    }

    void ReadValues(CSimpleIniA& a_ini, Values& a_values, std::string& a_tracks) {
        clib_util::ini::get_value(
            a_ini,
            a_values.debugLogging,
            kSection,
            kDebugLoggingKey,
            "; Toggle debug logging",
            clib_util::ini::bool_format::kNumeric
        );
        clib_util::ini::get_value(
            a_ini,
            a_values.onlyAtSaveLoad,
            kSection,
            kOnlyAtSaveLoadKey,
            "; Only stop music after loading a save.",
            clib_util::ini::bool_format::kNumeric
        );
        clib_util::ini::get_value(
            a_ini,
            a_tracks,
            kSection,
            kStopTracksKey,
            "; Comma-separated music types to stop when the player is out of combat."
        );
    }

}

const Values& Get() {
    return Current();
}

void Load() {
    auto loaded = BMK::Settings::Load(
        {
            .defaults = L"Data/MCM/Config/CombatMusicFixNG/settings.ini",
            .user = L"Data/MCM/Settings/CombatMusicFixNG.ini",
        },
        MakeDefaults(),
        [](CSimpleIniA& a_defaults, CSimpleIniA& a_user, Values& a_candidate) {
            std::string tracks = kDefaultStopTracks;
            ReadValues(a_defaults, a_candidate, tracks);
            ReadValues(a_user, a_candidate, tracks);
            a_candidate.commands = ParseCommands(tracks);
        }
    );
    if (!loaded) {
        SKSE::log::warn("Cannot load settings: {}", loaded.error().message);
        return;
    }
    if (loaded->saveFailure) {
        SKSE::log::warn("Cannot save settings: {}", loaded->saveFailure->message);
    }

    Current() = std::move(loaded->values);
    BMK::Settings::ApplyLogLevel(Current().debugLogging, SKSE::InitInfo {}.logLevel);
    SKSE::log::info("Loaded {} combat music tracks from settings", Current().commands.size());
}
}
