#include "Stats.hpp"
#include <iostream>
#include <chrono>

Stats::Stats()
    : totalGets_(0), totalSets_(0), lastGets_(0), lastSets_(0) {}

void Stats::start() {
    printThread_ = std::thread(&Stats::printLoop, this);
}

void Stats::stop() {
    stopRequested_ = true;
    if (printThread_.joinable()) {
        printThread_.join();
    }
}

void Stats::incrementGet(const std::string& key) {
    totalGets_++;
    lastGets_++;
    std::lock_guard<std::mutex> lock(mutex_);
    keyStats_[key].first++;
}

void Stats::incrementSet(const std::string& key) {
    totalSets_++;
    lastSets_++;
    std::lock_guard<std::mutex> lock(mutex_);
    keyStats_[key].second++;
}

int Stats::getReads(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return keyStats_[key].first;
}

int Stats::getWrites(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return keyStats_[key].second;
}

std::pair<int, int> Stats::getStats(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return keyStats_[key];
}

void Stats::printLoop() {
    using namespace std::chrono_literals;
    while (!stopRequested_) {
        std::this_thread::sleep_for(5s);
        std::cout << "[Stats] Total GETs: " << totalGets_ << ", SETs: " << totalSets_ << "\n";
        std::cout << "[Stats] Last 5s GETs: " << lastGets_ << ", SETs: " << lastSets_ << "\n";
        lastGets_ = 0;
        lastSets_ = 0;
    }
}
