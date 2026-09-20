#include "CombatMusicFix.h"
#include "Settings.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace {
bool IsPlayerInCombat() {
    const auto* player = RE::PlayerCharacter::GetSingleton();
    return player != nullptr && player->IsInCombat();
}

class EventSink final : public RE::BSTEventSink<RE::TESDeathEvent> {
public:
    RE::BSEventNotifyControl ProcessEvent(
        const RE::TESDeathEvent* a_event,
        [[maybe_unused]] RE::BSTEventSource<RE::TESDeathEvent>* a_source
    ) override {
        if (a_event != nullptr && a_event->actorDying && !IsPlayerInCombat()) {
            CombatMusicFix::GetSingleton().Schedule();
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

void MessageHandler(SKSE::MessagingInterface::Message* a_message) { // NOLINT(misc-const-correctness)
    if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
        if (!Settings::Get().onlyAtSaveLoad) {
            static EventSink sink;
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(&sink);
        }
    } else if (a_message->type == SKSE::MessagingInterface::kPostLoadGame && !IsPlayerInCombat()) {
        CombatMusicFix::GetSingleton().Schedule();
    }
}
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_extender) {
    SKSE::Init(a_extender, {.logPattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v"});
    Settings::Load();
    CombatMusicFix::GetSingleton();
    SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
    return true;
}
