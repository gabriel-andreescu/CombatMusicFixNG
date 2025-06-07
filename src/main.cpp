#include "Utils.h"
#include "Config.h"

class CombatMusicFix
{
	static constexpr auto kFixDelay = std::chrono::seconds(5);

public:
	static void StopCombatTracks()
	{
		std::thread([] {
			std::this_thread::sleep_for(kFixDelay);
			for (const auto& command : Config::stopCommands) {
				logger::info("Sending command: {}", command);
				inlineUtils::sendConsoleCommand(std::string(command));
			}
		}).detach();
	}
};

inline bool IsPlayerInCombat()
{
	const auto* player = RE::PlayerCharacter::GetSingleton();
	return player && player->IsInCombat();
}

class EventSink final : public RE::BSTEventSink<RE::TESDeathEvent>
{
	EventSink() = default;

public:
	EventSink(const EventSink&) = delete;
	EventSink(EventSink&&) = delete;
	EventSink& operator=(const EventSink&) = delete;
	EventSink& operator=(EventSink&&) = delete;

	static EventSink* GetSingleton() noexcept
	{
		static EventSink singleton;
		return &singleton;
	}

	RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* event, RE::BSTEventSource<RE::TESDeathEvent>*) override
	{
		if (event && event->actorDying && !IsPlayerInCombat()) {
			CombatMusicFix::StopCombatTracks();
		}
		return RE::BSEventNotifyControl::kContinue;
	}
};

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	if (a_message->type == SKSE::MessagingInterface::kPostLoadGame && !IsPlayerInCombat()) {
		CombatMusicFix::StopCombatTracks();
	}
}

extern "C" DLLEXPORT constinit SKSE::PluginVersionData SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v{};
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary();
	v.UsesNoStructs();
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
	logger::info("Game version: {}", a_skse->RuntimeVersion().string());

	Init(a_skse);
	Config::Load();

	RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESDeathEvent>(EventSink::GetSingleton());
	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);

	return true;
}
