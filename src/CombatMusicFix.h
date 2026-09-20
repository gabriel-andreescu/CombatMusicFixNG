#pragma once

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

class CombatMusicFix {
public:
    static CombatMusicFix& GetSingleton();
    void Schedule();

private:
    CombatMusicFix();
    void Run(const std::stop_token& a_stop);

    const std::vector<std::string> commands_;
    std::mutex mutex_;
    std::condition_variable_any changed_;
    std::deque<std::chrono::steady_clock::time_point> deadlines_;
    std::jthread worker_;
};
