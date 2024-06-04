cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0135 NEW)
project(BlackBoxRanking)

option(USE_STANDALONE_ASIO "set ON to use standalone Asio instead of Boost.Asio" OFF)

if(NOT MSVC)
    add_compile_options(-std=c++2a -Wall -Wextra -Werror -pedantic)
else()
    add_compile_options(/W1)
endif()

set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)

# Boost
find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem regex date_time chrono thread)
find_package(Poco REQUIRED Net Foundation NetSSL)

include_directories(${Boost_INCLUDE_DIRS})

# Asio
include(FetchContent)
FetchContent_Declare(
  BoostAsio
  GIT_REPOSITORY https://github.com/boostorg/asio.git
  GIT_TAG boost-1.77.0
)
FetchContent_MakeAvailable(BoostAsio)

# nlohmann json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# Google Test
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

# Source files
file(GLOB_RECURSE APP_SRC "app/*.cc" "app/*.h" "app/*.hpp")
file(GLOB_RECURSE LIB_SRC "lib/*.cc" "lib/*.h" "lib/*.hpp" "cec_codes/*.h" "cec_codes/*.cpp")

add_library(LIB SHARED ${LIB_SRC})
target_link_libraries(LIB ${Boost_LIBRARIES} Poco::Net Poco::Foundation Poco::NetSSL mysqlcppconn nlohmann_json::nlohmann_json)

add_executable(APP ${APP_SRC})
target_link_libraries(APP ${Boost_LIBRARIES} Poco::Net Poco::Foundation Poco::NetSSL mysqlcppconn nlohmann_json::nlohmann_json LIB)

target_link_directories(APP PRIVATE /usr/lib)

# Tests
file(GLOB_RECURSE TEST_SRC "test/*.cc")

add_executable(TEST ${TEST_SRC})
target_link_libraries(TEST gtest gtest_main gmock LIB)
add_test(NAME TEST COMMAND TEST)
