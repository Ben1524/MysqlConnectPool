cmake_minimum_required(VERSION 3.15) # vcpkg需要3.15+
set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file" FORCE)
message(STATUS "Using vcpkg toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/opt/vcpkg/installed/x64-linux/share")


project(MysqlConnectPool)

set(CMAKE_CXX_STANDARD 17)

# 调试配置
set(CMAKE_CXX_FLAGS_DEBUG "-rdynamic -O0 -ggdb -Wall -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined")

option(TEST "ON for complile test" ON)
enable_testing()

# 使用vcpkg方式查找包
find_package(GTest CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
set(MYSQL_CONNECTOR_CXX_INCLUDE_DIR "/usr/include/mysql-cppconn-8" CACHE PATH "MySQL Connector/C++ 头文件路径")
set(MYSQL_CONNECTOR_CXX_LIBRARIES "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so" CACHE FILEPATH "MySQL Connector/C++ 库文件路径")
include_directories(${MYSQL_CONNECTOR_CXX_INCLUDE_DIR})

message(STATUS "protobuf version: ${protobuf_VERSION}")
message(STATUS "protobuf libraries: ${protobuf_LIBRARIES}")
message(STATUS "protobuf include dirs: ${protobuf_INCLUDE_DIRS}")

include_directories(.)

add_executable(MysqlConnectPool main.cpp
        db/DatabaseManager.cpp
        db/DatabaseManager.h
        NonCopyable.h
        db/Result.cpp
        db/Result.h
        db/Field.h
        db/ArrayParser.cpp
        db/ArrayParser.h
        db/Exception.h
        db/Exception.cpp
        test/test_field.cpp
        db/Row.cpp
        db/Row.h
        db/RowIterator.h
        db/ResultImpl.h
        db/ResultIterator.h
        db/Field.cpp
        utils/utils.cpp
        utils/utils.h)


target_link_libraries(MysqlConnectPool PUBLIC
        protobuf::libprotoc
        protobuf::libprotobuf
        protobuf::libprotobuf-lite
        ZLIB::ZLIB
        libmysqlclient.so
        spdlog::spdlog
        JsonCpp::JsonCpp
        Threads::Threads
        asio::asio
        GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
        unofficial::concurrentqueue::concurrentqueue
        unofficial::mysql-connector-cpp::connector
        fmt::fmt
)