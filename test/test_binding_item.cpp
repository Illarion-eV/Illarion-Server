#include <gmock/gmock.h>

#include "script/LuaTestSupportScript.hpp"

#include "Character.hpp"
#include "Item.hpp"
#include "World.hpp"

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }
};

class item_bindings : public ::testing::Test {
	public:
		Item item;
		MockWorld world;
};

TEST_F(item_bindings, test_id_property) {
    item.setId(23);
    LuaTestSupportScript script {"function test(item)\n"
                          "assert(item.id == 23)\n"
			  "item.id = 42\n"
			  "return item\n"
			  "end",
			  "item_id_test"
			 };
    Item retval = script.test<Item, Item>(item);
    EXPECT_EQ(42, retval.getId());
}

TEST_F(item_bindings, test_wear_property) {
    item.setWear(23);
    LuaTestSupportScript script {"function test(item)\n"
                          "assert(item.wear == 23)\n"
			  "item.wear = 42\n"
			  "return item\n"
			  "end",
			  "item_id_test"
			 };
    Item retval = script.test<Item, Item>(item);
    EXPECT_EQ(42, retval.getWear());
}

TEST_F(item_bindings, test_number_property) {
    item.setNumber(23);
    LuaTestSupportScript script {"function test(item)\n"
                          "assert(item.number == 23)\n"
			  "item.number = 42\n"
			  "return item\n"
			  "end",
			  "item_id_test"
			 };
    Item retval = script.test<Item, Item>(item);
    EXPECT_EQ(42, retval.getNumber());
}

TEST_F(item_bindings, test_quality_property) {
    item.setQuality(23);
    LuaTestSupportScript script {"function test(item)\n"
                          "assert(item.quality == 23)\n"
			  "item.quality = 42\n"
			  "return item\n"
			  "end",
			  "item_id_test"
			 };
    Item retval = script.test<Item, Item>(item);
    EXPECT_EQ(42, retval.getQuality());
}

TEST_F(item_bindings, test_data_property) {
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
    Item retval = script.test<Item, Item>(item);
    EXPECT_EQ(std::string("world"), retval.getData("hello"));
    EXPECT_EQ(std::string("42"), retval.getData("answer"));
}

TEST_F(item_bindings, test_constructors) {
    {
    LuaTestSupportScript script {"function test(item)\n"
			  "local foo = Item()\n"
			  "foo.id = 23\n"
			  "return foo\n"
			  "end",
			  "item_constructor_test"
			 };
    Item retval = script.test<Item, Item>(item);
    EXPECT_EQ(23, retval.getId());
    }

    {
    LuaTestSupportScript script {"function test(item)\n"
			  "local foo = Item(1, 2, 3, 4)\n"
			  "return foo\n"
			  "end",
			  "item_constructor_test"
			 };
    Item retval = script.test<Item, Item>(item);
    EXPECT_EQ(1, retval.getId());
    EXPECT_EQ(2, retval.getNumber());
    EXPECT_EQ(3, retval.getWear());
    EXPECT_EQ(4, retval.getQuality());
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

