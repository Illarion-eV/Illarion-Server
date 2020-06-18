#include "LongTimeAction.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "script/LuaTestSupportScript.hpp"

#include <gmock/gmock.h>

class MockWorld : public World {
public:
    MockWorld() { World::_self = this; }

    MOCK_METHOD(bool, createDynamicNPC,
                (const std::string &, TYPE_OF_RACE_ID, const position &, Character::sex_type, const std::string &),
                (override));
    MOCK_METHOD(character_ptr, createMonster, (unsigned short, const position &, short), (override));
    MOCK_METHOD(bool, deleteNPC, (unsigned int), (override));
    MOCK_METHOD(std::vector<Character *>, getCharactersInRangeOf, (const position &, uint8_t), (const override));
    MOCK_METHOD(std::vector<Monster *>, getMonstersInRangeOf, (const position &, uint8_t), (const override));
    MOCK_METHOD(std::vector<NPC *>, getNPCS, (), (const override));
    MOCK_METHOD(std::vector<NPC *>, getNPCSInRangeOf, (const position &, uint8_t), (const override));
    MOCK_METHOD(std::vector<Player *>, getPlayersInRangeOf, (const position &, uint8_t), (const override));
    MOCK_METHOD(std::vector<Player *>, getPlayersOnline, (), (const override));
    MOCK_METHOD(bool, getArmorStruct, (TYPE_OF_ITEM_ID, ArmorStruct &), (override));
    MOCK_METHOD(std::string, getItemName, (TYPE_OF_ITEM_ID, uint8_t), (const override));
    MOCK_METHOD(ItemStruct, getItemStats, (const ScriptItem &), (const override));
    MOCK_METHOD(ItemStruct, getItemStatsFromId, (TYPE_OF_ITEM_ID), (const override));
    MOCK_METHOD(bool, getMonsterAttack, (TYPE_OF_RACE_ID, AttackBoni &), (override));
    MOCK_METHOD(bool, getNaturalArmor, (TYPE_OF_RACE_ID, MonsterArmor &), (override));
    MOCK_METHOD(bool, getWeaponStruct, (TYPE_OF_ITEM_ID, WeaponStruct &), (override));
    MOCK_METHOD(bool, changeItem, (ScriptItem), (override));
    MOCK_METHOD(void, changeQuality, (ScriptItem, short int), (override));
    MOCK_METHOD(ScriptItem, createFromId,
                (TYPE_OF_ITEM_ID, unsigned short int, const position &, bool, int, script_data_exchangemap const *),
                (override));
    MOCK_METHOD(bool, createFromItem, (const ScriptItem &, const position &, bool), (override));
    MOCK_METHOD(bool, erase, (ScriptItem, int), (override));
    MOCK_METHOD(bool, increase, (ScriptItem, short int), (override));
    MOCK_METHOD(bool, swap, (ScriptItem, TYPE_OF_ITEM_ID, unsigned short int), (override));
    MOCK_METHOD(std::list<BlockingObject>, blockingLineOfSight, (const position &, const position &), (const override));
    MOCK_METHOD(void, changeTile, (short int, const position &), (override));
    MOCK_METHOD(bool, createSavedArea, (uint16_t, const position &, uint16_t, uint16_t), (override));
    MOCK_METHOD(map::Field &, fieldAt, (const position &), (override));
    MOCK_METHOD(const map::Field &, fieldAt, (const position &), (const override));
    MOCK_METHOD(character_ptr, getCharacterOnField, (const position &), (const override));
    MOCK_METHOD(bool, isCharacterOnField, (const position &), (const override));
    MOCK_METHOD(ScriptItem, getItemOnField, (const position &), (override));
    MOCK_METHOD(bool, isItemOnField, (const position &), (override));
    MOCK_METHOD(void, makePersistentAt, (const position &), (override));
    MOCK_METHOD(void, removePersistenceAt, (const position &), (override));
    MOCK_METHOD(bool, isPersistentAt, (const position &), (const override));
    MOCK_METHOD(void, broadcast, (const std::string &, const std::string &), (const override));
    MOCK_METHOD(int, getTime, (const std::string &), (const override));
    MOCK_METHOD(void, gfx, (unsigned short int, const position &), (const override));
    MOCK_METHOD(void, makeSound, (unsigned short int, const position &), (const override));
    MOCK_METHOD(void, sendMonitoringMessage, (const std::string &, unsigned char), (const override));

    MOCK_METHOD(Character *, findCharacter, (TYPE_OF_CHARACTER_ID));
};

class MockMonster : public Monster {
public:
    MockMonster(TYPE_OF_CHARACTER_ID id) { setId(id); };
};

class MockPlayer : public Player {
public:
    MockPlayer(TYPE_OF_CHARACTER_ID id) { setId(id); };
};

class MockNPC : public NPC {
public:
    MockNPC(TYPE_OF_CHARACTER_ID id) { setId(id); };
};

using ::testing::_;
using ::testing::AtLeast;
using ::testing::ElementsAre;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgReferee;

class world_bindings : public ::testing::Test {
public:
    MockWorld world;
    MockMonster *m1, *m2, *m3;
    MockPlayer *player;
    MockNPC *npc;

    ~world_bindings() override {
        LuaScript::shutdownLua();
        delete m1;
        delete m2;
        delete m3;
        delete player;
        delete npc;
    }

    world_bindings() {
        m1 = new MockMonster(1);
        m2 = new MockMonster(2);
        m3 = new MockMonster(3);
        player = new MockPlayer(4);
        npc = new MockNPC(5);
        ON_CALL(world, findCharacter(m1->getId())).WillByDefault(Return(m1));
        ON_CALL(world, findCharacter(m2->getId())).WillByDefault(Return(m2));
        ON_CALL(world, findCharacter(m3->getId())).WillByDefault(Return(m3));
        ON_CALL(world, findCharacter(player->getId())).WillByDefault(Return(player));
        ON_CALL(world, findCharacter(npc->getId())).WillByDefault(Return(npc));
        EXPECT_CALL(world, findCharacter(_)).Times(AtLeast(0));
    }
};

TEST_F(world_bindings, createDynamicNPC) {
    LuaTestSupportScript script{"function test(world) return world:createDynamicNPC('foo', 24, position(2, 3, 5), "
                                "Character.female, 'foo') end"};
    EXPECT_CALL(world, createDynamicNPC("foo", 24, position(2, 3, 5), Character::female, "foo")).WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, createMonster) {
    LuaTestSupportScript script{"function test(world) return world:createMonster(6, position(2, 3, 5), -3).id end"};
    EXPECT_CALL(world, createMonster(6, position(2, 3, 5), -3)).WillOnce(Return(character_ptr(m2)));
    auto result = script.test<TYPE_OF_CHARACTER_ID, World *>(&world);
    EXPECT_EQ(result, m2->getId());
}

TEST_F(world_bindings, deleteNPC) {
    LuaTestSupportScript script{"function test(world) return world:deleteNPC(5) end"};
    EXPECT_CALL(world, deleteNPC(5)).WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, getCharactersInRangeOf) {
    std::vector<Character *> chars{m1, m2, m3};
    LuaTestSupportScript script{
            "function test(world) return world:getCharactersInRangeOf(position(2, 3, 5), 42)[3].id end"};
    EXPECT_CALL(world, getCharactersInRangeOf(position(2, 3, 5), 42)).WillOnce(Return(chars));
    auto result = script.test<TYPE_OF_CHARACTER_ID, World *>(&world);
    EXPECT_EQ(result, m3->getId());
}

TEST_F(world_bindings, getMonstersInRangeOf) {
    std::vector<Monster *> chars{m1, m2, m3};
    LuaTestSupportScript script{
            "function test(world) return world:getMonstersInRangeOf(position(2, 3, 5), 42)[3].id end"};
    EXPECT_CALL(world, getMonstersInRangeOf(position(2, 3, 5), 42)).WillOnce(Return(chars));
    auto result = script.test<TYPE_OF_CHARACTER_ID, World *>(&world);
    EXPECT_EQ(result, m3->getId());
}

TEST_F(world_bindings, getNPCS) {
    std::vector<NPC *> chars{npc};
    LuaTestSupportScript script{"function test(world) return world:getNPCS()[1].id end"};
    EXPECT_CALL(world, getNPCS()).WillOnce(Return(chars));
    auto result = script.test<TYPE_OF_CHARACTER_ID, World *>(&world);
    EXPECT_EQ(result, npc->getId());
}

TEST_F(world_bindings, getNPCSInRangeOf) {
    std::vector<NPC *> chars{npc};
    LuaTestSupportScript script{"function test(world) return world:getNPCSInRangeOf(position(2, 3, 5), 42)[1].id end"};
    EXPECT_CALL(world, getNPCSInRangeOf(position(2, 3, 5), 42)).WillOnce(Return(chars));
    auto result = script.test<TYPE_OF_CHARACTER_ID, World *>(&world);
    EXPECT_EQ(result, npc->getId());
}

TEST_F(world_bindings, getPlayersInRangeOf) {
    std::vector<Player *> chars{player};
    LuaTestSupportScript script{
            "function test(world) return world:getPlayersInRangeOf(position(2, 3, 5), 42)[1].id end"};
    EXPECT_CALL(world, getPlayersInRangeOf(position(2, 3, 5), 42)).WillOnce(Return(chars));
    auto result = script.test<TYPE_OF_CHARACTER_ID, World *>(&world);
    EXPECT_EQ(result, player->getId());
}

TEST_F(world_bindings, getPlayersOnline) {
    std::vector<Player *> chars{player};
    LuaTestSupportScript script{"function test(world) return world:getPlayersOnline()[1].id end"};
    EXPECT_CALL(world, getPlayersOnline()).WillOnce(Return(chars));
    auto result = script.test<TYPE_OF_CHARACTER_ID, World *>(&world);
    EXPECT_EQ(result, player->getId());
}

TEST_F(world_bindings, getArmorStruct1) {
    LuaTestSupportScript script{"function test(world) return world:getArmorStruct(23) end"};
    EXPECT_CALL(world, getArmorStruct(23, _)).WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, getArmorStruct2) {
    ArmorStruct armor;
    armor.Absorb = 7;
    LuaTestSupportScript script{"function test(world) _, armor = world:getArmorStruct(23) return armor end"};
    EXPECT_CALL(world, getArmorStruct(23, _)).WillOnce(SetArgReferee<1>(armor));
    auto result = script.test<ArmorStruct, World *>(&world);
    EXPECT_EQ(result.Absorb, armor.Absorb);
}

TEST_F(world_bindings, getItemName) {
    LuaTestSupportScript script{"function test(world) return world:getItemName(23, 42) end"};
    EXPECT_CALL(world, getItemName(23, 42)).WillOnce(Return("bar"));
    auto result = script.test<std::string, World *>(&world);
    EXPECT_EQ(result, "bar");
}

TEST_F(world_bindings, getItemStats) {
    ScriptItem scriptItem;
    ItemStruct item;
    item.Rareness = 3;
    LuaTestSupportScript script{"function test(world, item) return world:getItemStats(item) end"};
    EXPECT_CALL(world, getItemStats(scriptItem)).WillOnce(Return(item));
    auto result = script.test<ItemStruct, World *>(&world, &scriptItem);
    EXPECT_EQ(result.Rareness, item.Rareness);
}

TEST_F(world_bindings, getItemStatsFromId) {
    ItemStruct item;
    item.Rareness = 3;
    LuaTestSupportScript script{"function test(world) return world:getItemStatsFromId(23) end"};
    EXPECT_CALL(world, getItemStatsFromId(23)).WillOnce(Return(item));
    auto result = script.test<ItemStruct, World *>(&world);
    EXPECT_EQ(result.Rareness, item.Rareness);
}

TEST_F(world_bindings, getMonsterAttack1) {
    LuaTestSupportScript script{"function test(world) return world:getMonsterAttack(24) end"};
    EXPECT_CALL(world, getMonsterAttack(24, _)).WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, getMonsterAttack2) {
    AttackBoni attack;
    attack.attackValue = 8;
    LuaTestSupportScript script{"function test(world) _, attack = world:getMonsterAttack(24) return attack end"};
    EXPECT_CALL(world, getMonsterAttack(24, _)).WillOnce(SetArgReferee<1>(attack));
    auto result = script.test<AttackBoni, World *>(&world);
    EXPECT_EQ(result.attackValue, attack.attackValue);
}

TEST_F(world_bindings, getNaturalArmor1) {
    LuaTestSupportScript script{"function test(world) return world:getNaturalArmor(24) end"};
    EXPECT_CALL(world, getNaturalArmor(24, _)).WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, getNaturalArmor2) {
    MonsterArmor armor;
    armor.punctureArmor = 12;
    LuaTestSupportScript script{"function test(world) _, armor = world:getNaturalArmor(24) return armor end"};
    EXPECT_CALL(world, getNaturalArmor(24, _)).WillOnce(SetArgReferee<1>(armor));
    auto result = script.test<MonsterArmor, World *>(&world);
    EXPECT_EQ(result.punctureArmor, armor.punctureArmor);
}

TEST_F(world_bindings, getWeaponStruct1) {
    LuaTestSupportScript script{"function test(world) return world:getWeaponStruct(23) end"};
    EXPECT_CALL(world, getWeaponStruct(23, _)).WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, getWeaponStruct2) {
    WeaponStruct weapon;
    weapon.Range = 9;
    LuaTestSupportScript script{"function test(world) _, weapon = world:getWeaponStruct(23) return weapon end"};
    EXPECT_CALL(world, getWeaponStruct(23, _)).WillOnce(SetArgReferee<1>(weapon));
    auto result = script.test<WeaponStruct, World *>(&world);
    EXPECT_EQ(result.Range, weapon.Range);
}

TEST_F(world_bindings, changeItem) {
    ScriptItem item;
    LuaTestSupportScript script{"function test(world, item) return world:changeItem(item) end"};
    EXPECT_CALL(world, changeItem(item)).WillOnce(Return(true));
    auto result = script.test<bool, World *, ScriptItem *>(&world, &item);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, changeQuality) {
    ScriptItem item;
    item.setQuality(18);
    LuaTestSupportScript script{"function test(world, item) world:changeQuality(item, -7) end"};
    EXPECT_CALL(world, changeQuality(item, -7));
    script.test<bool, World *, ScriptItem *>(&world, &item);
}

TEST_F(world_bindings, createFromId) {
    ScriptItem item;
    item.setNumber(66);
    LuaTestSupportScript script{
            "function test(world) return world:createItemFromId(23, 13, position(2, 3, 5), true, 47, {}) end"};
    EXPECT_CALL(world, createFromId(23, 13, position(2, 3, 5), true, 47, _)).WillOnce(Return(item));
    auto result = script.test<ScriptItem, World *>(&world);
    EXPECT_EQ(result, item);
}

TEST_F(world_bindings, createFromItem) {
    ScriptItem item;
    LuaTestSupportScript script{
            "function test(world, item) return world:createItemFromItem(item, position(2, 3, 5), true) end"};
    EXPECT_CALL(world, createFromItem(item, position(2, 3, 5), true)).WillOnce(Return(true));
    auto result = script.test<bool, World *, ScriptItem *>(&world, &item);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, erase) {
    ScriptItem item;
    LuaTestSupportScript script{"function test(world, item) return world:erase(item, 47) end"};
    EXPECT_CALL(world, erase(item, 47)).WillOnce(Return(true));
    auto result = script.test<bool, World *, ScriptItem *>(&world, &item);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, increase) {
    ScriptItem item;
    LuaTestSupportScript script{"function test(world, item) return world:increase(item, -7) end"};
    EXPECT_CALL(world, increase(item, -7)).WillOnce(Return(true));
    auto result = script.test<bool, World *, ScriptItem *>(&world, &item);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, swap) {
    ScriptItem item;
    LuaTestSupportScript script{"function test(world, item) return world:swap(item, 23, 13) end"};
    EXPECT_CALL(world, swap(item, 23, 13)).WillOnce(Return(true));
    auto result = script.test<bool, World *, ScriptItem *>(&world, &item);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, blockingLineOfSight) {
    BlockingObject b;
    b.blockingType = BlockingObject::BT_CHARACTER;
    b.blockingChar = m2;
    std::list<BlockingObject> blockers;
    blockers.push_back(b);
    blockers.push_back(b);
    LuaTestSupportScript script{"function test(world) return world:LoS(position(2, 3, 5), position(2, 3, "
                                "5))[1].TYPE end"};
    EXPECT_CALL(world, blockingLineOfSight(position(2, 3, 5), position(2, 3, 5))).WillOnce(Return(blockers));
    auto result = script.test<std::string, World *>(&world);
    EXPECT_EQ(result, "CHARACTER");
}

TEST_F(world_bindings, changeTile) {
    LuaTestSupportScript script{"function test(world) world:changeTile(-7, position(2, 3, 5)) end"};
    EXPECT_CALL(world, changeTile(-7, position(2, 3, 5)));
    script.test<bool, World *>(&world);
}

TEST_F(world_bindings, createSavedArea) {
    LuaTestSupportScript script{"function test(world) return world:createSavedArea(41, position(2, 3, 5), 41, 41) end"};
    EXPECT_CALL(world, createSavedArea(41, position(2, 3, 5), 41, 41)).WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, fieldAt) {
    map::Field field(position(3, 5, 7));
    LuaTestSupportScript script{"function test(world) return world:getField(position(2, 3, 5)) end"};
    EXPECT_CALL(world, fieldAt(position(2, 3, 5))).WillOnce(ReturnRef(field));
    auto result = script.test<map::Field *, World *>(&world);
    EXPECT_EQ(result->getPosition(), field.getPosition());
}

TEST_F(world_bindings, getCharacterOnField) {
    LuaTestSupportScript script{"function test(world) return world:getCharacterOnField(position(2, 3, 5)) end"};
    EXPECT_CALL(world, getCharacterOnField(position(2, 3, 5))).WillOnce(Return(character_ptr()));
    auto result = script.test<character_ptr, World *>(&world);
    EXPECT_FALSE(result);
}

TEST_F(world_bindings, isCharacterOnField) {
    LuaTestSupportScript script{"function test(world) return world:isCharacterOnField(position(2, 3, 5)) end"};
    EXPECT_CALL(world, isCharacterOnField(position(2, 3, 5))).WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, getItemOnField) {
    ScriptItem item;
    item.setWear(22);
    LuaTestSupportScript script{"function test(world) return world:getItemOnField(position(2, 3, 5)) end"};
    EXPECT_CALL(world, getItemOnField(position(2, 3, 5))).WillOnce(Return(item));
    auto result = script.test<ScriptItem, World *>(&world);
    EXPECT_EQ(result, item);
}

TEST_F(world_bindings, isItemOnField) {
    LuaTestSupportScript script{"function test(world) return world:isItemOnField(position(2, 3, 5)) end"};
    EXPECT_CALL(world, isItemOnField(position(2, 3, 5))).WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, makePersistentAt) {
    LuaTestSupportScript script{"function test(world) world:makePersistentAt(position(2, 3, 5)) end"};
    EXPECT_CALL(world, makePersistentAt(position(2, 3, 5)));
    script.test<bool, World *>(&world);
}

TEST_F(world_bindings, removePersistenceAt) {
    LuaTestSupportScript script{"function test(world) world:removePersistenceAt(position(2, 3, 5)) end"};
    EXPECT_CALL(world, removePersistenceAt(position(2, 3, 5)));
    script.test<bool, World *>(&world);
}

TEST_F(world_bindings, isPersistentAt) {
    LuaTestSupportScript script{"function test(world) return world:isPersistentAt(position(2, 3, 5)) end"};
    EXPECT_CALL(world, isPersistentAt(position(2, 3, 5))).WillOnce(Return(true));
    auto result = script.test<bool, World *>(&world);
    EXPECT_TRUE(result);
}

TEST_F(world_bindings, broadcast) {
    LuaTestSupportScript script{"function test(world) world:broadcast('foo', 'foo') end"};
    EXPECT_CALL(world, broadcast("foo", "foo"));
    script.test<bool, World *>(&world);
}

TEST_F(world_bindings, getTime) {
    LuaTestSupportScript script{"function test(world) return world:getTime('foo') end"};
    EXPECT_CALL(world, getTime("foo")).WillOnce(Return(47));
    auto result = script.test<int, World *>(&world);
    EXPECT_EQ(result, 47);
}

TEST_F(world_bindings, gfx) {
    LuaTestSupportScript script{"function test(world) world:gfx(13, position(2, 3, 5)) end"};
    EXPECT_CALL(world, gfx(13, position(2, 3, 5)));
    script.test<bool, World *>(&world);
}

TEST_F(world_bindings, makeSound) {
    LuaTestSupportScript script{"function test(world) world:makeSound(13, position(2, 3, 5)) end"};
    EXPECT_CALL(world, makeSound(13, position(2, 3, 5)));
    script.test<bool, World *>(&world);
}

TEST_F(world_bindings, sendMonitoringMessage) {
    LuaTestSupportScript script{"function test(world) world:sendMonitoringMessage('foo', 4) end"};
    EXPECT_CALL(world, sendMonitoringMessage("foo", 4));
    script.test<bool, World *>(&world);
}

TEST_F(world_bindings, weather) {
    LuaTestSupportScript script{"function test(world) return world.weather.wind_dir end"};
    auto result = script.test<int, World *>(&world);
    EXPECT_EQ(result, 50);
}

auto main(int argc, char **argv) -> int {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
