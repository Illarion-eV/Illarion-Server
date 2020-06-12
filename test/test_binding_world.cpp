#include "World.hpp"
#include "script/LuaTestSupportScript.hpp"

#include <gmock/gmock.h>

class MockWorld : public World {
public:
    MockWorld() { World::_self = this; }

    MOCK_METHOD5(createDynamicNPC, bool(const std::string &, TYPE_OF_RACE_ID, const position &, Character::sex_type,
                                        const std::string &));
};

class world_bindings : public ::testing::Test {
public:
    MockWorld world;
};

using ::testing::Return;

TEST_F(world_bindings, createDynamicNPC) {
    LuaTestSupportScript script{"function test(world) return world:createDynamicNPC('Hugo', 2, position(4, 5, 6), "
                                "Character.female, 'no script') end"};
    EXPECT_CALL(world, createDynamicNPC("Hugo", 2, position(4, 5, 6), Character::female, "no script"))
            .WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

auto main(int argc, char **argv) -> int {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

