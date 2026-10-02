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

    const std::vector<std::string> _commands;
    std::mutex _mutex;
    std::condition_variable_any _changed;
    std::deque<std::chrono::steady_clock::time_point> _deadlines;
    std::jthread _worker;
};
