#include <gmock/gmock.h>

#include "script/LuaTestSupportScript.hpp"
#include "Character.hpp"
#include "World.hpp"

class MockCharacter : public Character {
public:
    TYPE_OF_CHARACTER_ID getId() const {return 1;};
    unsigned short getType() const {return 0;};
    std::string to_string() const {return {};};

    MOCK_CONST_METHOD0(isNewPlayer, bool());
    MOCK_METHOD1(pageGM, bool(const std::string &));
    MOCK_METHOD1(requestInputDialog, void(InputDialog *));
    MOCK_METHOD1(requestMessageDialog, void(MessageDialog *));
    MOCK_METHOD1(requestMerchantDialog, void(MerchantDialog *));
    MOCK_METHOD1(requestSelectionDialog, void(SelectionDialog *));
    MOCK_METHOD1(requestCraftingDialog, void(CraftingDialog *));
    MOCK_METHOD2(requestCraftingLookAt, void(unsigned int, ItemLookAt &));
    MOCK_METHOD2(requestCraftingLookAtIngredient, void(unsigned int, ItemLookAt &));
    MOCK_CONST_METHOD0(idleTime, uint32_t());
};

class MockWorld : public World {
public:
    MockCharacter *player;

    MockWorld() {
        World::_self = this;
        player = new MockCharacter();
    }

    ~MockWorld() {
        delete player;
    }

    Character *findCharacter(TYPE_OF_CHARACTER_ID id) {return player;};
};

using ::testing::Return;
using ::testing::_;

class world_bindings : public ::testing::Test {
public:
    MockWorld world;

    ~world_bindings() {
        LuaScript::shutdownLua();
    }
};

TEST_F(world_bindings, character_methods) {
    LuaTestSupportScript script {"function test(player) "
        "player:isNewPlayer() "
        "player:pageGM('test') "
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

