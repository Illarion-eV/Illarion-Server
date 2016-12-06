#include <gmock/gmock.h>
#include "Random.hpp"
#include <stdexcept>

TEST(random_tests, uniform_invalid_range) {
    try {
        Random::uniform(10, 3);
        FAIL() << "Expected std::invalid_argument";
    }
    catch(const std::invalid_argument & err) {
        EXPECT_EQ(err.what(), std::string(
            "Random::uniform: Invalid arguments, min(10) > max(3)"));
    }
    catch(...) {
        FAIL() << "Expected std::invalid_argument";
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
