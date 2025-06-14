#pragma once

#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <nlohmann/json.hpp>
#include <atomic>

class ConfigManager
{
public:
    /**
     * @brief Constructs a ConfigManager object and initializes it with the given configuration file.
     *
     * This constructor sets the configuration filename and initializes the dirty flag to false.
     *
     * @param fileName The path to the configuration file to be managed.
     */
    ConfigManager(const std::string &fileName);

    /**
     * @brief Destructor for ConfigManager.
     *
     * Stops the background thread if it is running and cleans up resources.
     */
    ~ConfigManager();

    /**
     * @brief Starts the background thread that periodically saves the configuration.
     *
     * This method initializes the background thread to run every 5 seconds, checking if the configuration
     * has been modified (dirty). If it has, it calls the save() method to persist the changes.
     * The thread will stop when stopRequested_ is set to true.
     */
    void start();

    /**
     * @brief Stops the background thread and waits for it to finish.
     *
     * This method sets the stopRequested_ flag to true, signaling the background thread to stop.
     * It then joins the thread to ensure it has completed before returning.
     */
    void stop();

    /**
     * @brief Loads the configuration from the file specified by fileName_.
     *
     * Attempts to open and parse the configuration file as JSON. If the file does not exist,
     * it creates a new empty JSON object and saves it to the file. If the file exists but
     * cannot be parsed, it logs an error and falls back to an empty JSON object.
     *
     * Thread safety is ensured by acquiring a unique lock on mutex_ during modifications
     * to the internal JSON data.
     *
     * @note This function will create the configuration file if it does not exist.
     */
    void load();

    /**
     * @brief Saves the current configuration data to a file.
     *
     * This method acquires a shared lock to ensure thread-safe access to the configuration data,
     * then writes the JSON representation of the configuration to the specified file.
     * If the file cannot be opened for writing, an error message is printed to std::cerr.
     *
     * @note The JSON data is formatted with an indentation of 4 spaces for readability.
     */
    void save();

    /**
     * @brief Retrieves the value associated with the specified key from the configuration.
     *
     * Acquires a shared lock to ensure thread-safe access to the configuration data.
     * If the key exists in the internal JSON data, returns its corresponding value as a string.
     * If the key does not exist, returns an empty string.
     *
     * @param key The configuration key to look up.
     * @return The value associated with the key, or an empty string if the key is not found.
     */
    std::string get(const std::string &key);

    /**
     * @brief Sets the value for a given configuration key.
     *
     * Updates the internal JSON data with the specified key-value pair.
     * Marks the configuration as dirty to indicate that changes have been made.
     * Thread-safe: acquires a lock to ensure safe concurrent access.
     *
     * @param key The configuration key to set.
     * @param value The value to associate with the key.
     */
    void set(const std::string &key, const std::string &value);

private:
    std::string fileName_;
    
    /**
     * @brief Mutex to protect access to jsonData_.
     *
     * Ensures thread-safe operations on the jsonData_ member variable.
     * Allows multiple readers or one writer at a time.
     */
    std::shared_mutex mutex_;
    nlohmann::json jsonData_;
    std::atomic<bool> dirty_;
    std::atomic<bool> stopRequested_ = false;
    std::thread backgroundThread_;
};
