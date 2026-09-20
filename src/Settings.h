#pragma once

#include <string>
#include <vector>

namespace Settings {
struct Values {
    bool debugLogging = false;
    bool onlyAtSaveLoad = false;
    std::vector<std::string> commands;
};

const Values& Get();
void Load();
}
