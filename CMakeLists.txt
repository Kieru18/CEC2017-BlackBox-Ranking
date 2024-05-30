cmake_minimum_required(VERSION 3.10)
project(BlackBoxRanking)


option(USE_STANDALONE_ASIO "set ON to use standalone Asio instead of Boost.Asio" OFF)

if(NOT MSVC)
    add_compile_options(-std=c++2a -Wall -Wextra -Werror -pedantic)
else()
    add_compile_options(/W1)
endif()

# Setting the path to MySQL C++ Connector header files
include_directories(/usr/include/mysql/cppconn)

set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)


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

# Source files
file(GLOB_RECURSE APP_SRC "app/*.cc" "app/*.h" "app/*.hpp")
file(GLOB_RECURSE LIB_SRC "lib/*.cc" "lib/*.h" "lib/*.hpp")

# Library
add_library(LIB SHARED ${LIB_SRC})
target_link_libraries(LIB ${Boost_LIBRARIES} Poco::Net Poco::Foundation Poco::NetSSL mysqlcppconn)

# Executable
add_executable(APP ${APP_SRC})
target_link_libraries(APP ${Boost_LIBRARIES} Poco::Net Poco::Foundation Poco::NetSSL mysqlcppconn LIB)

target_link_directories(APP PRIVATE /usr/lib)
