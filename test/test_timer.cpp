#include "Timer.hpp"

#include <gtest/gtest.h>
#include <thread>

TEST(timer_tests, premature_call) {
    Timer t{std::chrono::seconds(100)};
    EXPECT_FALSE(t.intervalExceeded());
}

TEST(timer_tests, time_exceeded) {
    Timer t{std::chrono::milliseconds(10)};
    std::this_thread::sleep_for(std::chrono::milliseconds(11));
    EXPECT_TRUE(t.intervalExceeded());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
