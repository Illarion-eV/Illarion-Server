#include <gmock/gmock.h>

#include "script/LuaTestSupportScript.hpp"

#include "Character.hpp"
#include "Item.hpp"
#include "World.hpp"
#include "Container.hpp"

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }

    MOCK_METHOD1(findCharacter, Character*(TYPE_OF_CHARACTER_ID id));
};

class MockCharacter : public Character {
public:
    MOCK_CONST_METHOD0(getId, TYPE_OF_CHARACTER_ID());
    MOCK_CONST_METHOD0(getType, unsigned short());
    MOCK_CONST_METHOD0(to_string, std::string());
    MOCK_CONST_METHOD2(inform, void(const std::string &, informType));
    MOCK_CONST_METHOD3(inform, void(const std::string &, const std::string &, informType));
    MOCK_CONST_METHOD0(GetBackPack, Container *());
};

using ::testing::Return;
using ::testing::AtLeast;

class scriptitem_bindings : public ::testing::Test {
	public:
		ScriptItem item;
		MockWorld world;
		MockCharacter character;

	scriptitem_bindings() {
	    ON_CALL(character, getId()).WillByDefault(Return(1));
        EXPECT_CALL(character, getId()).Times(AtLeast(0));
        ON_CALL(world, findCharacter(character.getId())).WillByDefault(Return(&character));
        EXPECT_CALL(world, findCharacter(character.getId())).Times(AtLeast(0));
	}
};

TEST_F(scriptitem_bindings, test_id_property) {
    item.setId(23);
    LuaTestSupportScript script {"function test(item)\n"
                          "assert(item.id == 23)\n"
			  "item.id = 42\n"
                          "assert(item.id == 42)\n"
			  "return item\n"
			  "end",
			  "item_id_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(42, retval.getId());
}

TEST_F(scriptitem_bindings, test_wear_property) {
    item.setWear(23);
    LuaTestSupportScript script {"function test(item)\n"
                          "assert(item.wear == 23)\n"
			  "item.wear = 42\n"
			  "return item\n"
			  "end",
			  "item_id_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(42, retval.getWear());
}

TEST_F(scriptitem_bindings, test_number_property) {
    item.setNumber(23);
    LuaTestSupportScript script {"function test(item)\n"
                          "assert(item.number == 23)\n"
			  "item.number = 42\n"
			  "return item\n"
			  "end",
			  "item_id_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(42, retval.getNumber());
}

TEST_F(scriptitem_bindings, test_quality_property) {
    item.setQuality(23);
    LuaTestSupportScript script {"function test(item)\n"
                          "assert(item.quality == 23)\n"
			  "item.quality = 42\n"
			  "return item\n"
			  "end",
			  "item_id_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(42, retval.getQuality());
}

TEST_F(scriptitem_bindings, test_data_property) {
    item.setData("foo", 1);
    item.setData("bar", "baz");
    LuaTestSupportScript script {"function test(item)\n"
                          "assert(item:getData(\"foo\") == \"1\")\n"
                          "assert(item:getData(\"bar\") == \"baz\")\n"
			  "item:setData(\"hello\", \"world\")\n"
			  "item:setData(\"answer\", 42)\n"
			  "return item\n"
			  "end",
			  "item_id_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(std::string("world"), retval.getData("hello"));
    EXPECT_EQ(std::string("42"), retval.getData("answer"));
}

TEST_F(scriptitem_bindings, test_constructor) {
    LuaTestSupportScript script {"function test(item)\n"
			  "local foo = scriptItem()\n"
			  "foo.id = 23\n"
			  "return foo\n"
			  "end",
			  "scriptitem_constructor_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(23, retval.getId());
}

TEST_F(scriptitem_bindings, test_constants) {
	std::list<std::pair<ScriptItem::itemtype, std::string>> constants {
		{ScriptItem::notdefined, "scriptItem.notdefined"},
		{ScriptItem::it_field, "scriptItem.field"},
		{ScriptItem::it_inventory, "scriptItem.inventory"},
		{ScriptItem::it_belt, "scriptItem.belt"},
		{ScriptItem::it_container, "scriptItem.container"},
	};
	auto function = [](ScriptItem::itemtype constant, const std::string& lua_name) {
		std::string code = "function test()\n";
		code += "return ";
		code += lua_name + "\n";
		code += "end";
		LuaTestSupportScript script {code,
			  "scriptitem_constants_test"
			  };
        auto retval = script.test<int, int>(0);
		EXPECT_EQ(constant, retval);
	};
	for (auto entry : constants) {
		function(entry.first, entry.second);
	}
}

TEST_F(scriptitem_bindings, test_owner) {
    item.setId(0);
    item.owner = &character;
    LuaTestSupportScript script {"function test(item)\n"
			  "assert(item.owner ~= nil)\n"
			  "item.id = 23\n"
			  "return item\n"
			  "end",
			  "scriptitem_owner_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(23, retval.getId());
}

TEST_F(scriptitem_bindings, test_pos) {
    item.setId(0);
    item.pos = { 1, 2, 3 };
    LuaTestSupportScript script {"function test(item)\n"
			  "assert(item.pos == position(1,2,3))\n"
			  "item.id = 23\n"
			  "return item\n"
			  "end",
			  "scriptitem_pos_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(23, retval.getId());
}

TEST_F(scriptitem_bindings, test_itempos) {
    item.setId(0);
    item.itempos = 42;
    LuaTestSupportScript script {"function test(item)\n"
			  "assert(item.itempos == 42)\n"
			  "item.id = 23\n"
			  "return item\n"
			  "end",
			  "scriptitem_itempos_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(23, retval.getId());
}

TEST_F(scriptitem_bindings, test_gettype) {
    item.setId(0);
    item.type = ScriptItem::it_field;
    LuaTestSupportScript script {"function test(item)\n"
			  "assert(item:getType() == scriptItem.field)\n"
			  "item.id = 23\n"
			  "return item\n"
			  "end",
			  "scriptitem_gettype_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(23, retval.getId());
}

TEST_F(scriptitem_bindings, test_inside) {
    item.setId(0);
    item.inside = new Container(23);
    LuaTestSupportScript script {"function test(item)\n"
			  "assert(item.inside ~= nil)\n"
			  "item.id = 23\n"
			  "return item\n"
			  "end",
			  "scriptitem_inside_test"
			 };
    auto retval = script.test<ScriptItem, ScriptItem>(item);
    EXPECT_EQ(23, retval.getId());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

