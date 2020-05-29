#include <gmock/gmock.h>
#include "CharacterContainer.hpp"
#include "World.hpp"
#include "Character.hpp"

using ::testing::AtLeast;
using ::testing::Return;
using ::testing::ReturnRef;

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }
};

class MockCharacter : public Character {
public:
    MOCK_CONST_METHOD0(getId, TYPE_OF_CHARACTER_ID());
    MOCK_CONST_METHOD0(getType, short unsigned int());
    MOCK_CONST_METHOD0(getPosition, const position &());
    MOCK_CONST_METHOD0(to_string, std::string());
};


class CharacterContainerTest : public ::testing::Test {
public:
    CharacterContainerTest() {
        ON_CALL(character, getId()).WillByDefault(Return(42));
        EXPECT_CALL(character, getId()).Times(AtLeast(0));
        ON_CALL(character, getPosition()).WillByDefault(ReturnRef(pos0));
        EXPECT_CALL(character, getPosition()).Times(AtLeast(0));
    }

    ~CharacterContainerTest() override = default;

    position pos {1, 2, 3};
    position pos0 {0, 0, 0};
    MockWorld world;
    MockCharacter character;

    CharacterContainer<Character> container;
};


TEST_F(CharacterContainerTest, empty) {
    EXPECT_TRUE(container.empty());
    EXPECT_EQ(0, container.size());
}

TEST_F(CharacterContainerTest, nonEmpty) {
    container.insert(&character);
    EXPECT_FALSE(container.empty());
    EXPECT_EQ(1, container.size());
}

TEST_F(CharacterContainerTest, insert) {
    container.insert(&character);
    EXPECT_TRUE(container.find(42));
    container.insert(&character);
    EXPECT_EQ(1, container.size());
}

TEST_F(CharacterContainerTest, find) {
    container.insert(&character);
    EXPECT_NE(nullptr, container.find(pos0));
}

TEST_F(CharacterContainerTest, update) {
    container.update(&character, pos);
    EXPECT_EQ(0, container.size());
    container.insert(&character);
    container.update(&character, pos);
    EXPECT_EQ(1, container.size());
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

