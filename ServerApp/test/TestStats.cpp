#define BOOST_TEST_MODULE StatsTest
#include <boost/test/included/unit_test.hpp>
#include "../src/Stats.hpp"

BOOST_AUTO_TEST_CASE(stats_counting) {
    Stats stats;

    stats.incrementGet("apple");
    stats.incrementGet("apple");
    stats.incrementSet("apple");

    stats.incrementGet("banana");

    BOOST_CHECK_EQUAL(stats.getReads("apple"), 2);
    BOOST_CHECK_EQUAL(stats.getWrites("apple"), 1);

    BOOST_CHECK_EQUAL(stats.getReads("banana"), 1);
    BOOST_CHECK_EQUAL(stats.getWrites("banana"), 0);
}
