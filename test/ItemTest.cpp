#include <gmock/gmock.h>
#include "data/Data.hpp"
#include "Item.hpp"

class CommonTest: public CommonObjectTable {
public:
    CommonTest() {
        CommonStruct common;
        common.id = 42;
        common.MaxStack = 50;
        emplace(42, common);
        activateBuffer();
    }
};

class ItemEnvironment: public ::testing::Environment {
public:
    virtual void SetUp() override {
        Data::CommonItems = CommonTest();
    }
};

TEST(ItemTest, defaultNumber) {
    Item item;
    EXPECT_EQ(0, item.getNumber());
}

TEST(ItemTest, increaseNumberByZero) {
    Item item;
    item.setId(42);
    EXPECT_EQ(0, item.increaseNumberBy(0));
    EXPECT_EQ(0, item.getNumber());
}

TEST(ItemTest, increaseNumberByPositive) {
    Item item;
    item.setId(42);
    EXPECT_EQ(0, item.increaseNumberBy(7));
    EXPECT_EQ(7, item.getNumber());
}

TEST(ItemTest, increaseNumberByOverflow) {
    Item item;
    item.setId(42);
    EXPECT_EQ(16, item.increaseNumberBy(66));
    EXPECT_EQ(50, item.getNumber());
}

TEST(ItemTest, setMinQuality) {
    {
        Item itemA;
        Item itemB;
        itemA.setQuality(0);
        itemB.setQuality(999);
        itemA.setMinQuality(itemB);
        EXPECT_EQ(0, itemA.getQuality());
    }
    {
        Item itemA;
        Item itemB;
        itemA.setQuality(999);
        itemB.setQuality(0);
        itemA.setMinQuality(itemB);
        EXPECT_EQ(0, itemA.getQuality());
    }
    {
        Item itemA;
        Item itemB;
        itemA.setQuality(133);
        itemB.setQuality(599);
        itemA.setMinQuality(itemB);
        EXPECT_EQ(133, itemA.getQuality());
    }
    {
        Item itemA;
        Item itemB;
        itemA.setQuality(866);
        itemB.setQuality(455);
        itemA.setMinQuality(itemB);
        EXPECT_EQ(455, itemA.getQuality());
    }
    {
        Item itemA;
        Item itemB;
        itemA.setQuality(388);
        itemB.setQuality(755);
        itemA.setMinQuality(itemB);
        EXPECT_EQ(355, itemA.getQuality());
    }
    {
        Item itemA;
        Item itemB;
        itemA.setQuality(611);
        itemB.setQuality(277);
        itemA.setMinQuality(itemB);
        EXPECT_EQ(211, itemA.getQuality());
    }
}

TEST(ItemTest, hasNoData) {
    Item item;
    EXPECT_TRUE(item.hasNoData());
}

TEST(ItemTest, setgetData) {
    Item item;
    EXPECT_EQ("", item.getData("testKey"));
    item.setData("testKey", "testValue");
    EXPECT_FALSE(item.hasNoData());
    EXPECT_EQ("testValue", item.getData("testKey"));
    item.setData("testKey", "");
    EXPECT_EQ("", item.getData("testKey"));
    EXPECT_TRUE(item.hasNoData());
}

TEST(ItemTest, hasData) {
    {
        Item item;
        EXPECT_TRUE(item.hasData({}));
        EXPECT_FALSE(item.hasData({std::make_pair("testKey", "testValue")}));
        EXPECT_FALSE(item.hasData({std::make_pair("testKey", "testValue"), std::make_pair("testKey2", "testValue2")}));
    }
    {
        Item item;
        item.setData("testKey", "testValue");
        EXPECT_FALSE(item.hasData({}));
        EXPECT_TRUE(item.hasData({std::make_pair("testKey", "testValue")}));
        EXPECT_FALSE(item.hasData({std::make_pair("testKey", "testValue"), std::make_pair("wrongKey", "wrongValue")}));
    }
    {
        Item item;
        item.setData("testKey", "testValue");
        item.setData("testKey2", "testValue2");
        EXPECT_FALSE(item.hasData({}));
        EXPECT_TRUE(item.hasData({std::make_pair("testKey", "testValue")}));
        EXPECT_TRUE(item.hasData({std::make_pair("testKey", "testValue"), std::make_pair("testKey2", "testValue2")}));
        EXPECT_FALSE(item.hasData({std::make_pair("testKey", "testValue"), std::make_pair("wrongKey", "wrongValue")}));
    }
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new ItemEnvironment);
    return RUN_ALL_TESTS();
}

