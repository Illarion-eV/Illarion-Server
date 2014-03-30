#include <gmock/gmock.h>

#include "script/LuaTestSupportScript.hpp"
#include "LongTimeAction.hpp"
#include "World.hpp"

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }
};

class longtimeaction_bindings : public ::testing::Test {
	public:
        int dummy = 0;
		MockWorld world;
};

TEST_F(longtimeaction_bindings, test_state_none) {
	LuaTestSupportScript script { "function test()\n"
			"return Action.none\n"
			"end",
			"longtimeaction_none_test"
	};

	auto none = script.test<int, int>(dummy);
	EXPECT_EQ(LongTimeAction::ST_NONE, none);
}

TEST_F(longtimeaction_bindings, test_state_abort) {
    LuaTestSupportScript script { "function test()\n"
            "return Action.abort\n"
            "end",
            "longtimeaction_abort_test"
    };

    auto abort = script.test<int, int>(dummy);
    EXPECT_EQ(LongTimeAction::ST_ABORT, abort);
}

TEST_F(longtimeaction_bindings, test_state_success) {
    LuaTestSupportScript script { "function test()\n"
            "return Action.success\n"
            "end",
            "longtimeaction_success_test"
    };

    auto success = script.test<int, int>(dummy);
    EXPECT_EQ(LongTimeAction::ST_SUCCESS, success);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

