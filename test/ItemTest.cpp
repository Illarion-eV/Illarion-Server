#include <gmock/gmock.h>
#include "data/Data.hpp"
#include "Item.hpp"

static const Item::id_type TESTITEM = 42;
static const Item::id_type MAXSTACK = 50;

class ItemTest : public ItemTable {
public:
    ItemTest() {
        ItemStruct itemStruct;
        itemStruct.id = TESTITEM;
        itemStruct.MaxStack = MAXSTACK;
        emplace(TESTITEM, itemStruct);
        activateBuffer();
    }
};

class ItemEnvironment: public ::testing::Environment {
public:
    void SetUp() override {
        Data::Items = ItemTest();
    }
};

TEST(ItemTest, defaultNumber) {
    Item item;
    EXPECT_EQ(0, item.getNumber());
}

TEST(ItemTest, increaseNumberByZero) {
    Item item;
    item.setId(TESTITEM);
    EXPECT_EQ(0, item.increaseNumberBy(0));
    EXPECT_EQ(0, item.getNumber());
}

TEST(ItemTest, increaseNumberByPositive) {
    Item item;
    item.setId(TESTITEM);
    EXPECT_LE(7, MAXSTACK);
    EXPECT_EQ(0, item.increaseNumberBy(7));
    EXPECT_EQ(7, item.getNumber());
}

TEST(ItemTest, increaseNumberByOverflow) {
    Item item;
    item.setId(TESTITEM);
    EXPECT_EQ(16, item.increaseNumberBy(MAXSTACK+16));
    EXPECT_EQ(MAXSTACK, item.getNumber());
}

TEST(ItemTest, setMinQualityMinMax) {
    Item itemA;
    Item itemB;
    itemA.setQuality(0);
    itemB.setQuality(999);
    itemA.setMinQuality(itemB);
    EXPECT_EQ(0, itemA.getQuality());
}

TEST(ItemTest, setMinQualityMaxMin) {
    Item itemA;
    Item itemB;
    itemA.setQuality(999);
    itemB.setQuality(0);
    itemA.setMinQuality(itemB);
    EXPECT_EQ(0, itemA.getQuality());
}

TEST(ItemTest, setMinQualityLowHigh) {
    Item itemA;
    Item itemB;
    itemA.setQuality(133);
    itemB.setQuality(599);
    itemA.setMinQuality(itemB);
    EXPECT_EQ(133, itemA.getQuality());
}

TEST(ItemTest, setMinQualityHighLow) {
    Item itemA;
    Item itemB;
    itemA.setQuality(866);
    itemB.setQuality(455);
    itemA.setMinQuality(itemB);
    EXPECT_EQ(455, itemA.getQuality());
}

TEST(ItemTest, setMinQualityMixedLowHigh) {
    Item itemA;
    Item itemB;
    itemA.setQuality(388);
    itemB.setQuality(755);
    itemA.setMinQuality(itemB);
    EXPECT_EQ(355, itemA.getQuality());
}

TEST(ItemTest, setMinQualityMixedHighLow) {
    Item itemA;
    Item itemB;
    itemA.setQuality(611);
    itemB.setQuality(277);
    itemA.setMinQuality(itemB);
    EXPECT_EQ(211, itemA.getQuality());
}

TEST(ItemTest, hasNoData) {
    Item item;
    EXPECT_TRUE(item.hasNoData());
    item.setData("testKey", "testValue");
    EXPECT_FALSE(item.hasNoData());
}

TEST(ItemTest, getDataEmpty) {
    Item item;
    EXPECT_EQ("", item.getData("testKey"));
}

TEST(ItemTest, getData) {
    Item item;
    item.setData("testKey", "testValue");
    EXPECT_FALSE(item.hasNoData());
    EXPECT_EQ("testValue", item.getData("testKey"));
}

TEST(ItemTest, setDataEmpty) {
    Item item;
    item.setData("testKey", "testValue");
    item.setData("testKey", "");
    EXPECT_EQ("", item.getData("testKey"));
    EXPECT_TRUE(item.hasNoData());
}

TEST(ItemTest, setDataNumber) {
    Item item;
    item.setData("testKey", 7);
    EXPECT_EQ("7", item.getData("testKey"));
}

TEST(ItemTest, setDataNullptr) {
    Item item;
    item.setData("testKey", "testValue");
    item.setData("testKey2", "testValue2");
    item.setData(nullptr);
    EXPECT_TRUE(item.hasNoData());
}

TEST(ItemTest, setDataUpdate) {
    Item item;
    item.setData("testKey", "testValueA");
    item.setData("testKey", "testValueB");
    EXPECT_EQ("testValueB", item.getData("testKey"));
}

TEST(ItemTest, setDataMap) {
    Item item;
    item.setData("testKey", "testValueA");
    script_data_exchangemap map = {std::make_pair("testKey", "testValueB"), std::make_pair("testKey2", "testValue2")};
    item.setData(&map);
    EXPECT_EQ("testValueB", item.getData("testKey"));
    EXPECT_EQ("testValue2", item.getData("testKey2"));
}

TEST(ItemTest, hasDataNone) {
    Item item;
    EXPECT_TRUE(item.hasData( {}));
    EXPECT_FALSE(item.hasData( {std::make_pair("testKey", "testValue")}));
    EXPECT_FALSE(item.hasData( {std::make_pair("testKey", "testValue"), std::make_pair("testKey2", "testValue2")}));
}

TEST(ItemTest, hasDataOne) {
    Item item;
    item.setData("testKey", "testValue");
    EXPECT_FALSE(item.hasData( {}));
    EXPECT_TRUE(item.hasData( {std::make_pair("testKey", "testValue")}));
    EXPECT_FALSE(item.hasData( {std::make_pair("testKey", "testValue"), std::make_pair("wrongKey", "wrongValue")}));
}

TEST(ItemTest, hasDataMultiple) {
    Item item;
    item.setData("testKey", "testValue");
    item.setData("testKey2", "testValue2");
    EXPECT_FALSE(item.hasData( {}));
    EXPECT_TRUE(item.hasData( {std::make_pair("testKey", "testValue")}));
    EXPECT_TRUE(item.hasData( {std::make_pair("testKey", "testValue"), std::make_pair("testKey2", "testValue2")}));
    EXPECT_FALSE(item.hasData( {std::make_pair("testKey", "testValue"), std::make_pair("wrongKey", "wrongValue")}));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new ItemEnvironment);
    return RUN_ALL_TESTS();
}

