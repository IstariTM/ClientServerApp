project(ServerApp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DBOOST_ASIO_HAS_CO_AWAIT)

find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(ServerApp 
    src/main.cpp
    src/Server.cpp
    src/ConfigManager.cpp
    src/Stats.cpp
    )

target_include_directories(ServerApp PRIVATE
    src
)

target_link_libraries(ServerApp PRIVATE
    Boost::boost
    Boost::system
    nlohmann_json::nlohmann_json
    ZLIB::ZLIB
)

enable_testing()

# ConfigManager Tests
add_executable(ConfigManagerTests
    test/TestConfig.cpp
    src/ConfigManager.cpp
)
target_include_directories(ConfigManagerTests PRIVATE 
    ${Boost_INCLUDE_DIRS}
    src)
target_link_libraries(ConfigManagerTests PRIVATE 
    nlohmann_json
    )

add_test(NAME ConfigManagerTest COMMAND ConfigManagerTests)

# Stats Tests
add_executable(StatsTests
    test/TestStats.cpp
    src/Stats.cpp
)
target_include_directories(StatsTests PRIVATE 
    ${Boost_INCLUDE_DIRS}
    src
    )

add_test(NAME StatsTest COMMAND StatsTests)