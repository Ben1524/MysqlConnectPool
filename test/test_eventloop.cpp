/**
*@ClassName test_evenloop
*@Author cxk
*@Data 25-6-8 下午5:45
*/
//
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "event/EventLoop.h"
#include "utils/MPSCQueue.h"
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>

using namespace cxk;
using namespace testing;
using namespace std::chrono_literals;


class EventLoopTest : public Test {
protected:
    void SetUp() override {
        loop = std::make_unique<EventLoop>();
    }

    void TearDown() override {
        if (loop->isRunning()) {
            loop->quit();
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    std::unique_ptr<EventLoop> loop;
    std::thread thread;
};

TEST_F(EventLoopTest, ConstructorAndDestructor) {
    EXPECT_FALSE(loop->isRunning());
    EXPECT_EQ(loop->threadId(), std::this_thread::get_id());
}

TEST_F(EventLoopTest, RunInLoopSameThread) {
    bool executed = false;
    loop->runInLoop([&] { executed = true; });
    EXPECT_TRUE(executed);
}

TEST_F(EventLoopTest, QueueInLoopDifferentThread) {
    std::atomic<bool> executed(false);
    std::condition_variable cv;
    std::mutex mutex;

    thread = std::thread([&] {
        loop->moveToCurrentThread();
        loop->runInLoop([&] {
            {
                std::lock_guard<std::mutex> lock(mutex);
                executed = true;
            }
            cv.notify_one();
        });
        loop->loop();
    });

    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, 1s, [&] { return executed.load(); });
    EXPECT_TRUE(executed);

    loop->quit();
    thread.join();
}

TEST_F(EventLoopTest, RunAtTimer) {
    std::atomic<bool> executed(false);
    auto when = DateTime::now().after(1);

    std::thread thread([&] {
        loop->moveToCurrentThread();
        loop->loop();
    });

    loop->runAt(when, [&] { executed = true; });

    std::this_thread::sleep_for(2s);
    loop->quit();
    thread.join();

    EXPECT_TRUE(executed);
}

TEST_F(EventLoopTest, RunAfterTimer) {
    std::atomic<bool> executed(false);

    loop->runAfter(0.1, [&] { executed = true; });

    thread = std::thread([&] { loop->moveToCurrentThread();
        loop->loop(); });

    std::this_thread::sleep_for(200ms);
    loop->quit();
    thread.join();

    EXPECT_TRUE(executed);
}

TEST_F(EventLoopTest, RunEveryTimer) {
    std::atomic<int> count(0);

    loop->runEvery(0.1, [&] {
        if (++count >= 3) {
            loop->quit();
        }
    });

    thread = std::thread([&] { loop->moveToCurrentThread();loop->loop(); });
    thread.join();

    EXPECT_EQ(count, 3);
}

TEST_F(EventLoopTest, InvalidateTimer) {
    std::atomic<bool> executed(false);
    TimerId timerId = loop->runAfter(0.1, [&] { executed = true; });

    loop->invalidateTimer(timerId);

    thread = std::thread([&] {
        loop->moveToCurrentThread();
        loop->loop();
    });

    std::this_thread::sleep_for(200ms);
    loop->quit();
    thread.join();

    EXPECT_FALSE(executed);
}

TEST_F(EventLoopTest, RunOnQuit) {
    std::atomic<bool> executed(false);

    loop->runOnQuit([&] { executed = true; });

    thread = std::thread([&] { loop->loop(); });
    loop->quit();
    thread.join();

    EXPECT_TRUE(executed);
}

TEST_F(EventLoopTest, IsInLoopThread) {
    EXPECT_TRUE(loop->isInLoopThread());

    std::atomic<bool> result(false);
    thread = std::thread([&] {
        result = loop->isInLoopThread();
        // loop->loop();
    });

    std::this_thread::sleep_for(100ms);
    loop->quit();
    thread.join();

    EXPECT_FALSE(result);
}