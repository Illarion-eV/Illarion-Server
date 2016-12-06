#include <gmock/gmock.h>

#include "script/LuaTestSupportScript.hpp"

#include "World.hpp"
#include "Logger.hpp"

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }
};

class random_bindings : public ::testing::Test {
	public:
		MockWorld world;
};

TEST_F(random_bindings, test_uniform_invalid_range) {
    //EXPECT_CALL(log_message(_, LogFacility::Script, _).Times(1));
    LuaTestSupportScript script {
              "function test()\n"
              "    Random.uniform(5, 2)\n"
			  "end",
			  "uniform_invalid_range_test"
			 };
    script.test<>();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

