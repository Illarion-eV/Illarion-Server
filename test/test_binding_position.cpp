#include <gmock/gmock.h>

#include "script/LuaTestSupportScript.hpp"
#include "globals.hpp"
#include "World.hpp"

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }
};

class position_bindings : public ::testing::Test {
	public:
		position pos;
		MockWorld world;
};

TEST_F(position_bindings, test_x_property) {
	pos.x = 23;
	LuaTestSupportScript script { "function test(pos)\n"
			"assert(pos.x == 23)\n"
			"pos.x = 42\n"
			"return pos\n"
			"end",
			"position_x_test"
	};

	auto retval = script.test(pos);
	EXPECT_EQ(42, retval.x);
}

TEST_F(position_bindings, test_y_property) {
	pos.y = 23;
	LuaTestSupportScript script { "function test(pos)\n"
			"assert(pos.y == 23)\n"
			"pos.y = 42\n"
			"return pos\n"
			"end",
			"position_y_test"
	};

	auto retval = script.test(pos);
	EXPECT_EQ(42, retval.y);
}

TEST_F(position_bindings, test_z_property) {
	pos.z = 23;
	LuaTestSupportScript script { "function test(pos)\n"
			"assert(pos.z == 23)\n"
			"pos.z = 42\n"
			"return pos\n"
			"end",
			"position_z_test"
	};

	auto retval = script.test(pos);
	EXPECT_EQ(42, retval.z);
}

TEST_F(position_bindings, test_tostring) {
	pos = {23, 42, 5};
	// in luabind this is: (23, 42, 5)
	LuaTestSupportScript script { "function test(pos)\n"
		"assert(tostring(pos):find(\"23\"))\n"
		"assert(tostring(pos):find(\"42\"))\n"
		"assert(tostring(pos):find(\"5\"))\n"
			"pos.x = 42\n"
			"return pos\n"
			"end",
			"position_tostring_test"
	};

	auto retval = script.test(pos);
	EXPECT_EQ(42, retval.x);
}

TEST_F(position_bindings, test_compare) {
	pos = {23, 42, 5};
	LuaTestSupportScript script { "function test(pos)\n"
		"assert(pos == position(23, 42, 5))\n"
		"assert(not(pos == 7))\n"
			"pos.x = 42\n"
			"return pos\n"
			"end",
			"position_compare_test"
	};

	auto retval = script.test(pos);
	EXPECT_EQ(42, retval.x);
}

TEST_F(position_bindings, test_constructors) {
	{
	LuaTestSupportScript script { "function test(pos)\n"
			"local foo = position()\n"
			"foo.x = 23\n"
			"return foo\n"
			"end",
			"position_constructor1_test"
	};
	auto retval = script.test(pos);
	EXPECT_EQ(23, retval.x);
	}
	{
	LuaTestSupportScript script { "function test(pos)\n"
			"local foo = position(23, 42, 5)\n"
			"return foo\n"
			"end",
			"position_constructor2_test"
	};
	auto retval = script.test(pos);
	EXPECT_EQ(23, retval.x);
	}
};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

