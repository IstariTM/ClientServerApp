#define BOOST_TEST_MODULE ConfigManagerTest
#include <boost/test/included/unit_test.hpp>
#include "../src/ConfigManager.hpp"
#include <filesystem>

namespace fs = std::filesystem;

BOOST_AUTO_TEST_CASE(config_get_set) {
    const std::string testFile = "config_test.json";
    fs::remove(testFile); // clean slate

    ConfigManager config(testFile);
    config.load();

    config.set("key1", "value1");
    config.set("key2", "value2");

    BOOST_CHECK_EQUAL(config.get("key1"), "value1");
    BOOST_CHECK_EQUAL(config.get("key2"), "value2");

    config.save();

    ConfigManager config2(testFile);
    config2.load();
    BOOST_CHECK_EQUAL(config2.get("key1"), "value1");
    BOOST_CHECK_EQUAL(config2.get("key2"), "value2");

    fs::remove(testFile);
}
