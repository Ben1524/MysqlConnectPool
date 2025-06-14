cmake_minimum_required(VERSION 3.15) # vcpkg需要3.15+
set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file" FORCE)
message(STATUS "Using vcpkg toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/opt/vcpkg/installed/x64-linux/share")


project(MySQLConnectPool)

set(CMAKE_CXX_STANDARD 17)

# 调试配置
set(CMAKE_CXX_FLAGS_DEBUG "-rdynamic -O0 -ggdb -Wall -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined")

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 配置全局头文件路径（使用INTERFACE目标）
add_library(project_headers INTERFACE)
target_include_directories(project_headers
        INTERFACE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/db
        ${CMAKE_SOURCE_DIR}/MySQL
        ${CMAKE_SOURCE_DIR}/third
)




include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https:///github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

option(TEST "ON for complile test" ON)
enable_testing()
include(GoogleTest)



# 使用vcpkg方式查找包
find_package(protobuf CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)

set(MYSQL_CONNECTOR_CXX_INCLUDE_DIR "/usr/include/mariadb/" CACHE PATH "MySQL Connector/C++ 头文件路径")
set(MYSQL_CONNECTOR_CXX_LIBRARIES "/usr/lib/x86_64-linux-gnu/libmariadb.so" CACHE FILEPATH "MySQL Connector/C++ 库文件路径")
include_directories(${MYSQL_CONNECTOR_CXX_INCLUDE_DIR})

message(STATUS "protobuf version: ${protobuf_VERSION}")
message(STATUS "protobuf libraries: ${protobuf_LIBRARIES}")
message(STATUS "protobuf include dirs: ${protobuf_INCLUDE_DIRS}")

include_directories(.)


# 将核心功能作为库构建
add_library(mysqlconnectpool_lib SHARED
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
        db/Row.cpp
        db/Row.h
        db/RowIterator.h
        db/ResultImpl.h
        db/ResultIterator.h
        db/Field.cpp
        utils/utils.cpp
        utils/utils.h
        MySQLImpl/MySQLResultImpl.cpp
        MySQLImpl/MySQLResultImpl.h
        MySQLImpl/MySQLConnector.cpp
        MySQLImpl/MySQLConnector.h
        db/DbConnection.h
        event/EventLoop.cpp
        event/EventLoop.h
        event/EventDispatcher.cpp
        event/EventDispatcher.h
        utils/MPSCQueue.h
        time/TimerQueue.cpp
        time/TimerQueue.h
        time/Timer.cpp
        time/Timer.h
        poll/EpollPoller.cpp
        poll/EpollPoller.h
        utils/ScopeExit.h
        time/DateTime.cpp
        time/DateTime.h
        db/DbTypes.h
        db/DbConnection.cpp
        event/EventLoopThread.cpp
        event/EventLoopThread.h
        event/EventLoopThreadPool.cpp
        event/EventLoopThreadPool.h
)



target_link_libraries(mysqlconnectpool_lib PUBLIC
        ZLIB::ZLIB
        ${CMAKE_SOURCE_DIR}/lib/libmysqlcppconnx.so.2
        ${MYSQL_CONNECTOR_CXX_LIBRARIES}
        /usr/lib/x86_64-linux-gnu/libmariadb.so
        spdlog::spdlog
        JsonCpp::JsonCpp
        Threads::Threads
        asio::asio
        GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
        unofficial::concurrentqueue::concurrentqueue
        fmt::fmt
        protobuf::libprotoc
        protobuf::libprotobuf
        protobuf::libprotobuf-lite
)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/db
        ${CMAKE_SOURCE_DIR}/MySQL
        ${CMAKE_SOURCE_DIR}/third
        ${CMAKE_SOURCE_DIR}/third/mysqlx/devapi
)
add_executable(MySQLConnectPool main.cpp
)




target_link_libraries(MySQLConnectPool
        PRIVATE
        mysqlconnectpool_lib
        ZLIB::ZLIB
        /usr/lib/x86_64-linux-gnu/libmariadb.so
        spdlog::spdlog
        JsonCpp::JsonCpp
        Threads::Threads
        asio::asio
        GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
        unofficial::concurrentqueue::concurrentqueue
        fmt::fmt
        protobuf::libprotoc
        protobuf::libprotobuf
        protobuf::libprotobuf-lite
        ${CMAKE_SOURCE_DIR}/lib/libmysqlcppconnx.so.2
        absl::any
        absl::log
        absl::base
        absl::bits
)
target_include_directories(MySQLConnectPool
        PRIVATE
        ${Protobuf_INCLUDE_DIRS}
)
# 测试配置
if(TEST)
    # 定义测试源文件列表
    set(TEST_SOURCES
            test/test_field.cpp
            test/test_eventloop.cpp
    )

    # 为每个测试文件创建单独的测试目标
    foreach(test_source ${TEST_SOURCES})
        # 获取不带路径和扩展名的文件名
        get_filename_component(test_name ${test_source} NAME_WE)

        # 创建测试可执行文件
        add_executable(${test_name} ${test_source}

        )

        # 链接必要的库
        target_link_libraries(${test_name}
                PRIVATE
                mysqlconnectpool_lib
        )

        # 将测试添加到CTest
        gtest_discover_tests(${test_name})

        # 添加编译选项
        if(USE_SANITIZERS)
            target_compile_options(${test_name} PRIVATE -fsanitize=address -fsanitize=leak -fsanitize=undefined)
            target_link_options(${test_name} PRIVATE -fsanitize=address -fsanitize=leak -fsanitize=undefined)
        endif()
    endforeach()
endif()