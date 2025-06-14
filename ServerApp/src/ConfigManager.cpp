#include "ConfigManager.hpp"
#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>

using json = nlohmann::json;

ConfigManager::ConfigManager(const std::string& fileName)
    : fileName_(fileName), dirty_(false) {
}

ConfigManager::~ConfigManager() {
    stop();
}

void ConfigManager::start() {
    stopRequested_ = false;
    backgroundThread_ = std::thread([this]() {
        while (!stopRequested_) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (dirty_.exchange(false)) {
                save();
            }
        }
    });
}

void ConfigManager::stop() {
    stopRequested_ = true;
    if (backgroundThread_.joinable()) {
        backgroundThread_.join();
    }
}

void ConfigManager::load() {
    std::ifstream file(fileName_);
    if (!file.is_open()) {
        std::cerr << "Config file not found, creating: " << fileName_ << "\n";
        {
            std::unique_lock lock(mutex_);
            jsonData_ = nlohmann::json::object();
        }
        save();
        return;
    }

    std::unique_lock lock(mutex_);
    try {
        file >> jsonData_;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse config: " << e.what() << "\n";
        jsonData_ = nlohmann::json::object();
    }
}

void ConfigManager::save() {
    std::shared_lock lock(mutex_);
    std::ofstream file(fileName_);
    if (!file.is_open()) {
        std::cerr << "Unable to write config file: " << fileName_ << "\n";
        return;
    }
    file << jsonData_.dump(4);
}

std::string ConfigManager::get(const std::string& key) {
    std::shared_lock lock(mutex_);
    if (jsonData_.contains(key)) {
        return jsonData_[key];
    }
    return "";
}

void ConfigManager::set(const std::string& key, const std::string& value) {
    {
        std::unique_lock lock(mutex_);
        jsonData_[key] = value;
    }
    dirty_ = true;
}
