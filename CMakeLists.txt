cmake_minimum_required(VERSION 3.10)
project(main)

set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)

add_executable(main main.cpp src/mail.cpp)

find_package(Poco REQUIRED Net Foundation)

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

target_include_directories(main PUBLIC src)
