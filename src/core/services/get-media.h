#pragma once
#include <string>
#include <vector>

struct MediaEntry {
    std::string name;
    bool isDir;
};

struct MediaFileTree {
    std::vector<MediaEntry> entries;
    bool success;
    std::string currentPath;
};
