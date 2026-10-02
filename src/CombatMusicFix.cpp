#include "CombatMusicFix.h"

#include "Settings.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <chrono>
#include <memory>
#include <mutex>
#include <stop_token>
#include <string>
#include <vector>

namespace {
void StopTracks(const std::vector<std::string>& a_commands) {
    const auto* player = RE::PlayerCharacter::GetSingleton();
    if (player == nullptr || player->IsInCombat()) {
        return;
    }

    auto* factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
    if (factory == nullptr) {
        return;
    }
    for (const auto& command : a_commands) {
        if (const auto script = std::unique_ptr<RE::Script>(factory->Create())) {
            script->SetCommand(command);
            script->CompileAndRun(nullptr);
            SKSE::log::debug("Sending command: {}", command);
        }
    }
}
}

CombatMusicFix& CombatMusicFix::GetSingleton() {
    // Joining the worker during DLL teardown can deadlock. Keep it alive until process exit.
    static auto* const instance = new CombatMusicFix;
    return *instance;
}

CombatMusicFix::CombatMusicFix()
    : _commands(Settings::Get().commands)
    , _worker([this](const std::stop_token& a_stop) { Run(a_stop); }) {}

void CombatMusicFix::Schedule() {
    constexpr auto kDelay = std::chrono::seconds(5);
    if (_commands.empty()) {
        return;
    }
    {
        const std::scoped_lock lock(_mutex);
        _deadlines.push_back(std::chrono::steady_clock::now() + kDelay);
    }
    _changed.notify_one();
}

void CombatMusicFix::Run(const std::stop_token& a_stop) {
    std::unique_lock lock(_mutex);
    while (_changed.wait(lock, a_stop, [this] { return !_deadlines.empty(); })) {
        const auto deadline = _deadlines.front();
        _changed.wait_until(lock, a_stop, deadline, [] { return false; });
        if (a_stop.stop_requested()) {
            return;
        }
        _deadlines.pop_front();
        lock.unlock();
        SKSE::GetTaskInterface()->AddTask([commands = _commands] { StopTracks(commands); });
        lock.lock();
    }
}
