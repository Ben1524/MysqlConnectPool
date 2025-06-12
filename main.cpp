/**
*@ClassName test_mysq
*@Author cxk
*@Data 25-6-11 下午3:33
*/
//

#include "MySQLImpl/MySQLConnector.h"
#include <db/Result.h>
#include <db/ResultIterator.h>
#include <memory>
#include <functional>
#include <future>
#include <thread>
#include <chrono>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/globals.h"
#include <absl/log/log.h>
#include <absl/log/initialize.h>
#include <absl/log/absl_log.h>

#include "Field.h"

// 测试环境配置 - 请根据实际情况修改
const std::string TEST_CONN_INFO =
    "host=47.97.210.72 user=test password=123456 port=3306 dbname=test";
using namespace cxk;
// // 测试简单查询
int main(){
    absl::InitializeLog();
    // 设置日志级别
    // 设置日志级别
    absl::SetStderrThreshold(
        static_cast<absl::LogSeverity>(absl::LogSeverity::kInfo));


    std::shared_ptr<EventLoop> loop_;
    std::shared_ptr<MySQLConnector> connection_;
    bool connectionEstablished_ = false;
    bool connectionClosed_ = false;
    loop_ = std::make_shared<EventLoop>();
    connection_ = std::make_shared<MySQLConnector>(loop_.get(), TEST_CONN_INFO);
    loop_->runOnQuit([&connectionClosed_, &connection_] {
        ABSL_LOG(INFO) << "Connection closed, cleaning up resources";
        connection_.reset();
    });

    std::thread([&]() {
                    // 将事件循环移动到当前线程
                 ABSL_LOG(INFO) << "Starting event loop in thread: " << std::this_thread::get_id();
                 loop_->moveToCurrentThread();
                 loop_->loop(); // 处理一次事件循环
             }).detach(); // 等待事件处理完成

    loop_->queueInLoop([] {
        ABSL_LOG(WARNING) << "Initializing MySQL connection";
    });
















    connection_->init();
    // 设置连接成功回调
    connection_->setOkCallback([&connectionEstablished_] (const std::shared_ptr<DbConnection>& conn) {
        connectionEstablished_ = true;
    });

    // 设置连接关闭回调
    connection_->setCloseCallback([&connectionClosed_] (const std::shared_ptr<DbConnection>& conn) {
        connectionClosed_ = true;
    });




    // 等待连接建立
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 等待连接建立

    std::promise<Result> resultPromise;
    auto future = resultPromise.get_future();
    std::exception_ptr exceptionPtr = nullptr;

    if (!connectionEstablished_) {
        ABSL_LOG(ERROR) << "Connection not established";
        return -1;
    }

    // 执行查询
    connection_->execSql(
        "SELECT * FROM test_users WHERE id = 1",
        0,
        {},
        {},
        {},
        std::move([&resultPromise](const Result& result) {
            resultPromise.set_value(result);
        }),
        std::move([&exceptionPtr](const std::exception_ptr& e) {
            ABSL_LOG(WARNING) << "Exception during query execution: " ;
            exceptionPtr = e;
        })
    );


    // 验证结果
    if (exceptionPtr) {
        std::rethrow_exception(exceptionPtr);
        ABSL_LOG(ERROR) << "Exception occurred during query execution";
    }

    const Result result = future.get();
    if (result.empty()) {
        ABSL_LOG(ERROR) << "Query returned no results";
        return -1;
    }
    ABSL_LOG(INFO) << "Query executed successfully, number of rows: " << result.size();
    for (const auto & row: result) {
        std::cout << "Row: "<<row.at("id").as<int32_t>() << ", "
                       << row.at("name").as<std::string>() << ", "
                       << row.at("email").as<std::string>();
    }



    loop_->quit();
    while (1);
}
