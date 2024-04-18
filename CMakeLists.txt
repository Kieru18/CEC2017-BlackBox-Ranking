cmake_minimum_required(VERSION 3.10)
project(BlackBoxRanking)

option(USE_STANDALONE_ASIO "set ON to use standalone Asio instead of Boost.Asio" OFF)

if(NOT MSVC)
    add_compile_options(-std=c++2a -Wall -Wextra -Werror -pedantic)
else()
    add_compile_options(/W1)
endif()


file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")          #Get all cpp files
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/headers/*.h")   #Get all header files

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

add_executable(main main.cc ${SRC})
target_link_libraries(main ${Boost_LIBRARIES})

# Dodanie opcji linkowania do biblioteki MySQL C++ Connector
target_link_libraries(main ${Boost_LIBRARIES} mysqlcppconn Poco::Net Poco::Foundation)

# Dodatkowe opcje linkowania
target_link_directories(main PRIVATE /usr/lib)  # Ścieżka do bibliotek linkujących (jeśli wymagane)
target_link_libraries(main mysqlcppconn)         # Biblioteka do linkowania
