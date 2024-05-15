cmake_minimum_required(VERSION 3.10)
project(BlackBoxRanking)

option(USE_STANDALONE_ASIO "set ON to use standalone Asio instead of Boost.Asio" OFF)

if(NOT MSVC)
    add_compile_options(-std=c++2a -Wall -Wextra -Werror -pedantic)
else()
    add_compile_options(/W1)
endif()

# Ustawianie ścieżki do plików nagłówkowych MySQL C++ Connector
include_directories(/usr/include/mysql/cppconn)

set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)

# Boost
find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem regex date_time chrono thread)
find_package(Poco REQUIRED Net Foundation)


# If Boost was found, link directories might be needed (depends on Boost and system setup)
include_directories(${Boost_INCLUDE_DIRS})

# asio
include(FetchContent)
# set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
 BoostAsio
 GIT_REPOSITORY https://github.com/boostorg/asio.git
 GIT_TAG boost-1.77.0
)
FetchContent_MakeAvailable(BoostAsio)

file(GLOB_RECURSE LIB_SRC "lib/src/*.cc" "lib/headers/*.h")
target_link_libraries(LIB ${Boost_LIBRARIES} mysqlcppconn Poco::Net Poco::Foundation)
add_library(LIB ${LIB_SRC})


file(GLOB_RECURSE APP_SRC "app/*.cc" "app/*.h")
add_executable(APP ${APP_SRC})


target_link_libraries(APP LIB)
target_link_libraries(APP ${Boost_LIBRARIES})


# Dodanie opcji linkowania do biblioteki MySQL C++ Connector
target_link_libraries(APP ${Boost_LIBRARIES} mysqlcppconn Poco::Net Poco::Foundation)

# Dodatkowe opcje linkowania
target_link_directories(APP PRIVATE /usr/lib)  # Ścieżka do bibliotek linkujących (jeśli wymagane)
target_link_libraries(APP mysqlcppconn)         # Biblioteka do linkowania
