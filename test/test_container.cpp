#include <gmock/gmock.h>

#include "Container.hpp"
#include "World.hpp"

const Item::id_type itemid_1 = 0x23;
const Item::id_type itemid_2 = 0x42;

using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::AtLeast;
using ::testing::_;

class MockContainer : public Container {
	public:
		MockContainer(Item::id_type id) : Container(id) {
		}

		MOCK_CONST_METHOD0(getSlotCount, TYPE_OF_CONTAINERSLOTS());
};

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }

    //MOCK_METHOD2(getItemName, std::string(TYPE_OF_ITEM_ID itemid, uint8_t language));
    //MOCK_METHOD3(itemInform, void(Character *user, const ScriptItem &item, const ItemLookAt &lookAt));
    //MOCK_METHOD1(findCharacter, Character*(TYPE_OF_CHARACTER_ID id));
};

class container_tests : public ::testing::Test {
	public:
		container_tests() : container{0x13} {
			ON_CALL(container, getSlotCount()).WillByDefault(Return(100));
            EXPECT_CALL(container, getSlotCount()).Times(AtLeast(0));
		}

		~container_tests() {
		}

		MockContainer container;
        MockWorld world;
        uint16_t size_c1 = 1;
		uint16_t size_c2 = 1;
};

TEST_F(container_tests, eraseItemWithoutDataDoesntDeleteDataItem) {
	Item it1{itemid_1, 1, 0};
	EXPECT_TRUE(container.InsertItem(it1, false));

	EXPECT_EQ(1, container.countItem(itemid_1));

	EXPECT_EQ(10, container.eraseItem(itemid_2, 10));
	EXPECT_EQ(9, container.eraseItem(itemid_1, 10));

	Item it2{itemid_1, 1, 0};
	it2.setData("foodata", 1);
	EXPECT_TRUE(container.InsertItem(it1, false));
	EXPECT_TRUE(container.InsertItem(it2, false));

	script_data_exchangemap filter;

	EXPECT_EQ(10, container.eraseItem(itemid_2, 10, &filter));
	EXPECT_EQ(9, container.eraseItem(itemid_1, 10, &filter));

	EXPECT_TRUE(container.InsertItem(it1, false));

	EXPECT_EQ(10, container.eraseItem(itemid_2, 10));
	EXPECT_EQ(8, container.eraseItem(itemid_1, 10));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
