#include <gmock/gmock.h>

#include "script/LuaTestSupportScript.hpp"
#include "Monster.hpp"
#include "World.hpp"

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }

    MOCK_METHOD1(findCharacter, Character*(TYPE_OF_CHARACTER_ID id));
    MOCK_METHOD2(getMonsterDefinition, bool(TYPE_OF_CHARACTER_ID, MonsterStruct &));
};

using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::AtLeast;
using ::testing::_;

class world_bindings : public ::testing::Test {
public:
    MockWorld world;
    MonsterStruct monsterDefinition;
    Monster *monster;

    ~world_bindings() {
        LuaScript::shutdownLua();
        delete monster;
    }

    world_bindings() {
        ON_CALL(world, getMonsterDefinition(_, _)).WillByDefault(DoAll(SetArgReferee<1>(monsterDefinition), Return(true)));
        EXPECT_CALL(world, getMonsterDefinition(_, _)).Times(AtLeast(0));
        monster = new Monster(5, position(1, 2, 3));
        ON_CALL(world, findCharacter(monster->getId())).WillByDefault(Return(monster));
        EXPECT_CALL(world, findCharacter(monster->getId())).Times(AtLeast(0));
    }
};

TEST_F(world_bindings, isNewPlayer) {
    LuaTestSupportScript script {"function test(monster) return monster:isNewPlayer() end"};
    bool result = script.test<Monster *, bool>(monster);
    EXPECT_FALSE(result);
}
/*
TEST_F(world_bindings, pageGM) {
    LuaTestSupportScript script {"function test(monster) return monster:pageGM('test') end"};
    EXPECT_FALSE(script.test(&monster));
}

TEST_F(world_bindings, requestInputDialog) {
    LuaTestSupportScript script {"function test(monster) return monster:requestInputDialog() end"};
    EXPECT_FALSE(script.test(&monster));
}
*/
/*
        "player:requestInputDialog(nil) "
        "player:requestMessageDialog(nil) "
        "player:requestMerchantDialog(nil) "
        "player:requestSelectionDialog(nil) "
        "player:requestCraftingDialog(nil) "
        "player:requestCraftingLookAt(0, ItemLookAt()) "
        "player:requestCraftingLookAtIngredient(0, ItemLookAt()) "
        "player:idleTime() "
        "return true "
        "end",
    };

    MockCharacter &player = *world.player;
    
    EXPECT_CALL(player, isNewPlayer()).Times(1);
    EXPECT_CALL(player, pageGM(_)).Times(1);
    EXPECT_CALL(player, requestInputDialog(_)).Times(1);
    EXPECT_CALL(player, requestMessageDialog(_)).Times(1);
    EXPECT_CALL(player, requestMerchantDialog(_)).Times(1);
    EXPECT_CALL(player, requestSelectionDialog(_)).Times(1);
    EXPECT_CALL(player, requestCraftingDialog(_)).Times(1);
    EXPECT_CALL(player, requestCraftingLookAt(_, _)).Times(1);
    EXPECT_CALL(player, requestCraftingLookAtIngredient(_, _)).Times(1);
    EXPECT_CALL(player, idleTime()).Times(1);

    script.test((Character*)&player);
}
*/
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

