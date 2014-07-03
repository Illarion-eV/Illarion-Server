#include <gmock/gmock.h>

#include "script/LuaItemScript.hpp"
#include "Character.hpp"
#include "World.hpp"
#include "Container.hpp"

class MockCharacter : public Character {
public:
    MOCK_CONST_METHOD0(getId, TYPE_OF_CHARACTER_ID());
    MOCK_CONST_METHOD0(getType, unsigned short());
    MOCK_CONST_METHOD0(to_string, std::string());
    MOCK_CONST_METHOD2(inform, void(const std::string &, informType));
    MOCK_CONST_METHOD3(inform, void(const std::string &, const std::string &, informType));
    MOCK_CONST_METHOD0(GetBackPack, Container *());
};

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }

    MOCK_METHOD2(getItemName, std::string(TYPE_OF_ITEM_ID itemid, uint8_t language));
    MOCK_METHOD1(findCharacter, Character*(TYPE_OF_CHARACTER_ID id));
};

class MockContainer : public Container {
public:
    MockContainer() : Container(0) {}

    MOCK_CONST_METHOD2(countItem, int(Item::id_type, script_data_exchangemap const *));
    MOCK_METHOD3(eraseItem, int(Item::id_type, Item::number_type, script_data_exchangemap const *));
};

using ::testing::Return;
using ::testing::AtLeast;
using ::testing::_;
using ::testing::Pointee;

class world_bindings : public ::testing::Test {
public:
    MockWorld world;
    MockCharacter player;
    ScriptItem item;
    CommonStruct itemdef;
    MockContainer container;

    ~world_bindings() {
        LuaScript::shutdownLua();
    }

    world_bindings() {
        ON_CALL(player, getId()).WillByDefault(Return(1));
        EXPECT_CALL(player, getId()).Times(AtLeast(0));
	    ON_CALL(world, findCharacter(player.getId())).WillByDefault(Return(&player));
        EXPECT_CALL(world, findCharacter(player.getId())).Times(AtLeast(0));
    }
};

TEST_F(world_bindings, LookAtItem) {
    LuaItemScript script {"function LookAtItem(player, item)\n"
                          "local lookAt = ItemLookAt()\n"
                          "lookAt.name = world:getItemName(item.id,Player.english)\n"
                          "return lookAt\n"
                          "end",
                          "lookat_test", itemdef
                         };

    ItemLookAt lookAt;
    lookAt.setName("itemname");
    EXPECT_CALL(world, getItemName(item.getId(), static_cast<uint8_t>(Language::english))).Times(1).WillOnce(Return("itemname"));
    ItemLookAt result = script.LookAtItem(&player, item);
    EXPECT_EQ(lookAt, result);
}

TEST_F(world_bindings, UseItem) {
    LuaItemScript script {"function UseItem(User, SourceItem, ltstate)\n"
                          "User:inform(\"You have used the item with the name '\"..world:getItemName(SourceItem.id,Player.english)..\"' and the ID \"..tonumber(SourceItem.id)..\".\")\n"
                          "end",
                          "useitem_test", itemdef
                         };

    EXPECT_CALL(world, getItemName(item.getId(), _)).Times(1).WillOnce(Return("itemname"));
    EXPECT_CALL(player, inform(_,_)).Times(1);
    script.UseItem(&player, item, 1);
}

TEST_F(world_bindings, ContainerCountItem) {
    LuaItemScript script {"function UseItem(User, SourceItem, ltstate)\n"
                          "local container = User:getBackPack()\n"
                          "container:countItem(1, {key1 = \"value1\", key2 = \"value2\"})"
                          "end",
                          "container_countitem_test", itemdef
                         };
    const script_data_exchangemap data {{"key1", "value1"}, {"key2", "value2"}};

    EXPECT_CALL(player, GetBackPack()).Times(1).WillOnce(Return(&container));
    EXPECT_CALL(container, countItem(1, Pointee(data))).Times(1);
    script.UseItem(&player, item, 1);
}

TEST_F(world_bindings, ContainerEraseItem) {
    LuaItemScript script {"function UseItem(User, SourceItem, ltstate)\n"
                          "local container = User:getBackPack()\n"
                          "container:eraseItem(1, 1, {key1 = \"value1\", key2 = \"value2\"})"
                          "end",
                          "container_eraseitem_test", itemdef
                         };
    const script_data_exchangemap data {{"key1", "value1"}, {"key2", "value2"}};

    EXPECT_CALL(player, GetBackPack()).Times(1).WillOnce(Return(&container));
    EXPECT_CALL(container, eraseItem(1, 1, Pointee(data))).Times(1);
    script.UseItem(&player, item, 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

