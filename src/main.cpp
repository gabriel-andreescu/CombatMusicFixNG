#include "Utils.h"

class CombatMusicFix
{
	static inline const std::vector<std::string> stopCombatMusic = {
		"removemusic MUScombat",
		"removemusic MUScombatCivilWar",
		"removemusic MUScombatBossUmbra",
		"removemusic MUScombatBossDLC1",
		"removemusic MUScombatBossChargen",
		"removemusic MUScombatBoss",
		"removemusic DLC2MUScombatKarstaag",
		"removemusic DLC2MUScombatBoss",
	};

public:
	static void fix()
	{
		auto asyncFunc = []() {
			std::this_thread::sleep_for(std::chrono::seconds(5));
			for (const auto& command : stopCombatMusic) {
				inlineUtils::sendConsoleCommand(command);
			}
		};
		std::jthread t(asyncFunc);
		t.detach();
	}
};

class EventSink : public RE::BSTEventSink<RE::TESDeathEvent>
{
	EventSink() = default;

public:
	EventSink(const EventSink&) = delete;
	EventSink(EventSink&&) = delete;
	EventSink& operator=(const EventSink&) = delete;
	EventSink& operator=(EventSink&&) = delete;

	static EventSink* GetSingleton()
	{
		static EventSink singleton;
		return &singleton;
	}

	RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* event, RE::BSTEventSource<RE::TESDeathEvent>*) override
	{
		if (auto playerCharacter = RE::PlayerCharacter::GetSingleton(); event && !playerCharacter->IsInCombat()) {
			CombatMusicFix::fix();
		}

		return RE::BSEventNotifyControl::kContinue;
	}
};

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	// ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
	switch (a_message->type) {
	case SKSE::MessagingInterface::kPostLoadGame:
		auto playerCharacter = RE::PlayerCharacter::GetSingleton();
		if (playerCharacter && !playerCharacter->IsInCombat()) {
			CombatMusicFix::fix();
		}
		break;
	}
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary(true);
	v.HasNoStructUse();
	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory");
	}

	*path /= Plugin::NAME;
	*path += ".log";
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v");

	logger::info(FMT_STRING("{} v{}"), Plugin::NAME, Plugin::VERSION);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();

	logger::info("Game version : {}", a_skse->RuntimeVersion().string());

	Init(a_skse);

	auto* eventSink = EventSink::GetSingleton();

	auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
	eventSourceHolder->AddEventSink<RE::TESDeathEvent>(eventSink);

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageHandler);

	return true;
}
