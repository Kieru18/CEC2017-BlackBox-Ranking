cmake_minimum_required(VERSION 3.10)
project(main)


option(USE_STANDALONE_ASIO "set ON to use standalone Asio instead of Boost.Asio" OFF)

set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)


find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem regex date_time chrono thread)
find_package(Poco REQUIRED Net Foundation)


include_directories(${Boost_INCLUDE_DIRS})

# asio
include(FetchContent)
FetchContent_Declare(
 BoostAsio
 GIT_REPOSITORY https://github.com/boostorg/asio.git
 GIT_TAG boost-1.77.0
)
FetchContent_MakeAvailable(BoostAsio)

add_executable(main main.cpp src/mail.cpp src/db.cpp src/server.cpp src/hash.cpp ${SRC})

target_link_libraries(main ${Boost_LIBRARIES})

target_link_libraries(main mysqlcppconn)

target_link_libraries(main
    PocoNetSSL
    PocoCrypto
    PocoUtil
    PocoNet
    PocoXML
    PocoFoundation
    ssl
    crypto
    PocoJSON
    pthread
)
