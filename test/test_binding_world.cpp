#include "World.hpp"
#include "script/LuaTestSupportScript.hpp"

#include <gmock/gmock.h>

class MockWorld : public World {
public:
    MockWorld() { World::_self = this; }

    // Related to Character

    MOCK_METHOD5(createDynamicNPC, bool(const std::string &, TYPE_OF_RACE_ID, const position &, Character::sex_type,
                                        const std::string &));
    MOCK_METHOD3(createMonster, character_ptr(unsigned short, const position &, short));
    MOCK_METHOD1(deleteNPC, bool(unsigned int));
    MOCK_CONST_METHOD2(getCharactersInRangeOf, std::vector<Character *>(const position &, uint8_t));
    MOCK_CONST_METHOD2(getMonstersInRangeOf, std::vector<Monster *>(const position &, uint8_t));
    MOCK_CONST_METHOD0(getNPCS, std::vector<NPC *>());
    MOCK_CONST_METHOD2(getNPCSInRangeOf, std::vector<NPC *>(const position &, uint8_t));
    MOCK_CONST_METHOD2(getPlayersInRangeOf, std::vector<Player *>(const position &, uint8_t));
    MOCK_CONST_METHOD0(getPlayersOnline, std::vector<Player *>());

    // Related to Data

    MOCK_METHOD2(getArmorStruct, bool(TYPE_OF_ITEM_ID, ArmorStruct &));
    MOCK_CONST_METHOD2(getItemName, std::string(TYPE_OF_ITEM_ID, uint8_t));
    MOCK_CONST_METHOD1(getItemStats, ItemStruct(const ScriptItem &));
    MOCK_CONST_METHOD1(getItemStatsFromId, ItemStruct(TYPE_OF_ITEM_ID));
    MOCK_METHOD2(getMonsterAttack, bool(TYPE_OF_RACE_ID, AttackBoni &));
    MOCK_METHOD2(getNaturalArmor, bool(TYPE_OF_RACE_ID, MonsterArmor &));
    MOCK_METHOD2(getWeaponStruct, bool(TYPE_OF_ITEM_ID, WeaponStruct &));

    // Related to Item

    MOCK_METHOD1(changeItem, bool(ScriptItem));
    MOCK_METHOD2(changeQuality, void(ScriptItem, short int));
    MOCK_METHOD6(createFromId, ScriptItem(TYPE_OF_ITEM_ID, unsigned short int, const position &, bool,
                              int, script_data_exchangemap const *));
    MOCK_METHOD3(createFromItem, bool(const ScriptItem &, const position &, bool));
    MOCK_METHOD2(erase, bool(ScriptItem, int));
    MOCK_METHOD2(increase, bool(ScriptItem, short int));
    MOCK_METHOD3(swap, bool(ScriptItem, TYPE_OF_ITEM_ID, unsigned short int));

    // Related to WorldMap

    MOCK_CONST_METHOD2(blockingLineOfSight, std::list<BlockingObject>(const position &, const position &));
    MOCK_METHOD2(changeTile, void(short int, const position &));
    MOCK_METHOD4(createSavedArea, bool(uint16_t, const position &, uint16_t, uint16_t));
    MOCK_METHOD1(fieldAt, map::Field &(const position &));
    MOCK_CONST_METHOD1(fieldAt, const map::Field &(const position &));
    MOCK_CONST_METHOD1(getCharacterOnField, character_ptr(const position &));
    MOCK_CONST_METHOD1(isCharacterOnField, bool(const position &));
    MOCK_METHOD1(getItemOnField, ScriptItem(const position &));
    MOCK_CONST_METHOD1(isItemOnField, bool(const position &));
    MOCK_METHOD1(makePersistentAt, void(const position &));
    MOCK_METHOD1(removePersistenceAt, void(const position &));
    MOCK_CONST_METHOD1(isPersistentAt, bool(const position &));

    // Other

    MOCK_CONST_METHOD2(broadcast, void(const std::string &, const std::string &));
    MOCK_CONST_METHOD1(getTime, int(const std::string &));
    MOCK_CONST_METHOD2(gfx, void(unsigned short int, const position &));
    MOCK_CONST_METHOD2(makeSound, void(unsigned short int, const position &));
    MOCK_CONST_METHOD2(sendMonitoringMessage, void(const std::string &, unsigned char));
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

