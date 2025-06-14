#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <thread>

class Stats
{
public:
    /**
     * @brief Constructor for Stats.
     *
     * Initializes the statistics counters to zero.
     */
    Stats();
    
    /**
     * @brief Starts the statistics tracking thread.
     *
     * This method launches a background thread that periodically prints the total and last 5 seconds
     * GET and SET counts. The thread runs until stop() is called.
     */
    void start();

    /**
     * @brief Stops the statistics tracking thread.
     *
     * This method sets a flag to request the thread to stop and waits for it to finish.
     */
    void stop();

    /**
     * @brief Increments the GET count for a specific key.
     *
     * This method increments the total and last GET counts, and updates the read count for the specified key.
     *
     * @param key The key for which the GET count is being incremented.
     */
    void incrementGet(const std::string &key);

    /**
     * @brief Increments the SET count for a specific key.
     *
     * This method increments the total and last SET counts, and updates the write count for the specified key.
     *
     * @param key The key for which the SET count is being incremented.
     */
    void incrementSet(const std::string &key);

    /**
     * @brief Gets the read count for a specific key.
     *
     * This method retrieves the number of reads (GETs) for the specified key.
     *
     * @param key The key for which the read count is being retrieved.
     * @return The number of reads for the specified key.
     */
    int getReads(const std::string &key);

    /**
     * @brief Gets the write count for a specific key.
     *
     * This method retrieves the number of writes (SETs) for the specified key.
     *
     * @param key The key for which the write count is being retrieved.
     * @return The number of writes for the specified key.
     */
    int getWrites(const std::string &key);
    
    /**
     * @brief Gets the read and write statistics for a specific key.
     *
     * This method retrieves both the read (GET) and write (SET) counts for the specified key.
     *
     * @param key The key for which the statistics are being retrieved.
     * @return A pair containing the number of reads and writes for the specified key.
     */
    std::pair<int, int> getStats(const std::string &key);

private:
    /**
     * @brief Background thread that prints statistics every 5 seconds.
     *
     * This method runs in a loop, printing the total and last 5 seconds GET and SET counts.
     * The loop continues until stopRequested_ is set to true.
     */
    void printLoop();

    /**
     * @brief Mutex to protect access to keyStats_.
     *
     * Ensures thread-safe operations on the keyStats_ member variable.
     */
    std::mutex mutex_;
    std::unordered_map<std::string, std::pair<int, int>> keyStats_; // key -> (reads, writes)
    std::atomic<int> totalGets_;
    std::atomic<int> totalSets_;
    std::atomic<int> lastGets_;
    std::atomic<int> lastSets_;

    std::atomic<bool> stopRequested_ = false;
    std::thread printThread_;
};
