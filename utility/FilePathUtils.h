#pragma once

#include <filesystem>
#include <string>

inline std::filesystem::path getProjectRoot() {
    std::filesystem::path current = std::filesystem::current_path();

    while (!current.empty()) {
        if (std::filesystem::exists(current / ".git") || 
            std::filesystem::exists(current / "CMakeLists.txt")) {
            return current;
        }
        current = current.parent_path();
    }

    throw std::runtime_error("Project root not found.");
}