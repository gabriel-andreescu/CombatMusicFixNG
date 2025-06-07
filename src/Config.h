#pragma once

#include <SimpleIni.h>
#include <string>
#include <vector>
#include <sstream>

namespace Config
{
	inline std::vector<std::string> stopCommands;

	inline void Load()
	{
		CSimpleIniA ini;
		ini.SetUnicode();
		if (ini.LoadFile(R"(Data\SKSE\Plugins\CombatMusicFixNG.ini)") < 0) {
			logger::warn("Failed to load CombatMusicFixNG.ini");
			return;
		}

		auto raw = ini.GetValue("CombatMusic", "StopTracks", "");
		std::stringstream ss(raw ? raw : "");
		std::string token;

		stopCommands.clear();
		while (std::getline(ss, token, ',')) {
			std::erase_if(token, ::isspace);
			if (!token.empty()) {
				stopCommands.emplace_back("removemusic " + token);
			}
		}

		logger::info("Loaded {} combat music tracks from config", stopCommands.size());
	}
}
