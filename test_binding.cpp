#include <gmock/gmock.h>

#include "script/LuaItemScript.hpp"
#include "Character.hpp"
#include "World.hpp"

class MockCharacter : public Character {
	public:
		MOCK_CONST_METHOD0(to_string, std::string());
		MOCK_CONST_METHOD2(inform, void(const std::string&, informType));
		MOCK_CONST_METHOD3(inform, void(const std::string&, const std::string&, informType));
};

class MockWorld : public World {
	public:
		MockWorld() {
			World::_self = this;
		}

		MOCK_METHOD2(getItemName, std::string(TYPE_OF_ITEM_ID itemid, uint8_t language));
		MOCK_METHOD3(itemInform, void(Character *user, ScriptItem item, ItemLookAt lookAt));
};

using ::testing::Return;
using ::testing::_;

class world_bindings : public ::testing::Test {
	public:
		MockWorld world;
		MockCharacter player;
		ScriptItem item;
		CommonStruct itemdef;
};

TEST_F(world_bindings, LookAtItem) {
	LuaItemScript script{"function LookAtItem(player, item)\n"
			"local lookAt = ItemLookAt()\n"
			"lookAt.name = world:getItemName(item.id,Player.english)\n"
			"world:itemInform(player, item, lookAt)\n"
			"return true\n"
			"end",
			"lookat_test", itemdef};

	EXPECT_CALL(world, getItemName(_, _)).Times(1).WillOnce(Return("itemname"));
	EXPECT_CALL(world, itemInform(_, _, _)).Times(1);
	script.LookAtItem(&player, item);
}

TEST_F(world_bindings, UseItem) {
	LuaItemScript script{"function UseItem(User, SourceItem, ltstate)\n"
			"User:inform(\"You have used the item with the name '\"..world:getItemName(SourceItem.id,Player.english)..\"' and the ID \"..tonumber(SourceItem.id)..\".\")\n"
			"end",
			"useitem_test", itemdef};

	EXPECT_CALL(world, getItemName(_, _)).Times(1).WillOnce(Return("itemname"));
	EXPECT_CALL(player, inform(_,_)).Times(1);
	script.UseItem(&player, item, 1);
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

