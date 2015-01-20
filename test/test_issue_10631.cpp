#include <gmock/gmock.h>

#include "World.hpp"
#include "script/LuaScript.hpp"

using ::testing::_;

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }

    MOCK_METHOD2(getItemName, std::string(TYPE_OF_ITEM_ID, uint8_t));
};

class issue_10631 : public ::testing::Test {
	public:
		MockWorld world;
};

// Script A requires script B. When the server directly loads script B
// afterwards, this must not result in a second execution of B.
// To test this behaviour, B calls world:getItemName. The test is
// successful if and only if world:getItemName is called exactly once.
TEST_F(issue_10631, test_regression) {
    EXPECT_CALL(world, getItemName(_, _)).Times(1);
    LuaScript("issue_10631_a");
    LuaScript("issue_10631_b");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

