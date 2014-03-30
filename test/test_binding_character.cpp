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

class MockMonster : public Monster {
public:
    MockMonster() : Monster(5, position(1, 2, 3), nullptr) {}

    MOCK_CONST_METHOD0(isNewPlayer, bool());
    MOCK_METHOD1(pageGM, bool(const std::string &));
    MOCK_METHOD1(requestInputDialog, void(InputDialog *));
    MOCK_METHOD1(requestMessageDialog, void(MessageDialog *));
    MOCK_METHOD1(requestMerchantDialog, void(MerchantDialog *));
    MOCK_METHOD1(requestSelectionDialog, void(SelectionDialog *));
    MOCK_METHOD1(requestCraftingDialog, void(CraftingDialog *));
    MOCK_METHOD2(requestCraftingLookAt, void(unsigned int, ItemLookAt &));
    MOCK_METHOD2(requestCraftingLookAtIngredient, void(unsigned int, ItemLookAt &));
};

using ::testing::Return;
using ::testing::AtLeast;
using ::testing::_;

class monster_bindings : public ::testing::Test {
public:
    MockWorld world;
    MockMonster *monster;

    ~monster_bindings() {
        LuaScript::shutdownLua();
        delete monster;
    }

    monster_bindings() {
        ON_CALL(world, getMonsterDefinition(_, _)).WillByDefault(Return(true));
        EXPECT_CALL(world, getMonsterDefinition(_, _)).Times(AtLeast(0));
        monster = new MockMonster();
        ON_CALL(world, findCharacter(monster->getId())).WillByDefault(Return(monster));
        EXPECT_CALL(world, findCharacter(monster->getId())).Times(AtLeast(0));

        ON_CALL(*monster, isNewPlayer()).WillByDefault(Return(false));
    }
};

TEST_F(monster_bindings, isNewPlayer) {
    LuaTestSupportScript script {"function test(monster) return monster:isNewPlayer() end"};
    EXPECT_CALL(*monster, isNewPlayer());
    bool result = script.test<bool, Monster *>(monster);
    EXPECT_FALSE(result);
}

TEST_F(monster_bindings, pageGM) {
    LuaTestSupportScript script {"function test(monster) return monster:pageGM('test') end"};
    EXPECT_CALL(*monster, pageGM("test"));
    bool result = script.test<bool, Monster *>(monster);
    EXPECT_FALSE(result);
}

TEST_F(monster_bindings, requestInputDialog) {
    LuaTestSupportScript script {"function test(monster) monster:requestInputDialog(InputDialog('', '', false, 0, nil)) end"};
    EXPECT_CALL(*monster, requestInputDialog(_));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, requestMessageDialog) {
    LuaTestSupportScript script {"function test(monster) monster:requestMessageDialog(MessageDialog('', '', nil)) end"};
    EXPECT_CALL(*monster, requestMessageDialog(_));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, requestMerchantDialog) {
    LuaTestSupportScript script {"function test(monster) monster:requestMerchantDialog(MerchantDialog('', nil)) end"};
    EXPECT_CALL(*monster, requestMerchantDialog(_));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, requestSelectionDialog) {
    LuaTestSupportScript script {"function test(monster) monster:requestSelectionDialog(SelectionDialog('', '', nil)) end"};
    EXPECT_CALL(*monster, requestSelectionDialog(_));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, requestCraftingDialog) {
    LuaTestSupportScript script {"function test(monster) monster:requestCraftingDialog(CraftingDialog('', 0, 0, nil)) end"};
    EXPECT_CALL(*monster, requestCraftingDialog(_));
    script.test<bool, Monster *>(monster);
}

/*
        "player:requestCraftingLookAt(0, ItemLookAt()) "
        "player:requestCraftingLookAtIngredient(0, ItemLookAt()) "
        "player:idleTime() "
}
*/
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

