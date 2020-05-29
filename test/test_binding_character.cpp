#include <gmock/gmock.h>

#include "script/LuaTestSupportScript.hpp"
#include "Monster.hpp"
#include "World.hpp"
#include "dialog/InputDialog.hpp"
#include "dialog/MessageDialog.hpp"
#include "dialog/MerchantDialog.hpp"
#include "dialog/SelectionDialog.hpp"
#include "dialog/CraftingDialog.hpp"

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }

    MOCK_METHOD1(findCharacter, Character*(TYPE_OF_CHARACTER_ID id));
};

class Player {};

class MockMonster : public Monster {
public:
    MockMonster() = default;;

    MOCK_CONST_METHOD0(isNewPlayer, bool());
    MOCK_METHOD1(pageGM, bool(const std::string &));
    MOCK_METHOD1(requestInputDialog, void(InputDialog *));
    MOCK_METHOD1(requestMessageDialog, void(MessageDialog *));
    MOCK_METHOD1(requestMerchantDialog, void(MerchantDialog *));
    MOCK_METHOD1(requestSelectionDialog, void(SelectionDialog *));
    MOCK_METHOD1(requestCraftingDialog, void(CraftingDialog *));
    MOCK_METHOD2(requestCraftingLookAt, void(unsigned int, ItemLookAt &));
    MOCK_METHOD2(requestCraftingLookAtIngredient, void(unsigned int, ItemLookAt &));
    MOCK_CONST_METHOD0(idleTime, uint32_t());
    MOCK_METHOD1(sendBook, void(uint16_t));
    MOCK_METHOD0(forceUpdateAppearanceForAll, void());
    MOCK_METHOD1(performAnimation, void(uint8_t));
    MOCK_CONST_METHOD2(alterSpokenMessage, std::string(const std::string &, int));
    MOCK_CONST_METHOD0(actionRunning, bool());
    MOCK_METHOD2(changeQualityAt, void(unsigned char, short int));
    MOCK_CONST_METHOD0(isAdmin, bool());
    MOCK_METHOD2(talk, void(Character::talk_type, const std::string &));
    MOCK_METHOD3(talk, void(Character::talk_type, const std::string &, const std::string &));
    MOCK_METHOD2(sendCharDescription, void(TYPE_OF_CHARACTER_ID, const std::string &));
    MOCK_METHOD5(startAction, void(unsigned short, unsigned short, unsigned short, unsigned short, unsigned short));
    MOCK_METHOD0(abortAction, void());
    MOCK_METHOD0(successAction, void());
    MOCK_METHOD1(actionDisturbed, void(Character *));
    MOCK_METHOD1(changeSource, void(Character *));
    MOCK_METHOD1(changeSource, void(const ScriptItem &));
    MOCK_METHOD1(changeSource, void(const position &));
    MOCK_METHOD0(changeSource, void());
    MOCK_METHOD1(changeTarget, void(Character *));
    MOCK_METHOD1(changeTarget, void(const ScriptItem &));
    MOCK_METHOD1(changeTarget, void(const position &));
    MOCK_METHOD0(changeTarget, void());
    MOCK_METHOD1(introducePlayer, void(Player *));
    MOCK_METHOD2(move, bool(direction, bool));
    MOCK_METHOD1(turn, void(direction));
    MOCK_METHOD1(turn, void(const position &));
    MOCK_CONST_METHOD2(getNextStepDir, bool(const position &, direction &));
    MOCK_METHOD1(changeRace, void(TYPE_OF_RACE_ID));
    MOCK_CONST_METHOD0(getRace, TYPE_OF_RACE_ID());
    MOCK_CONST_METHOD0(getFaceTo, Character::face_to());
    MOCK_CONST_METHOD0(getType, unsigned short());
    MOCK_METHOD1(increasePoisonValue, void(short int));
    MOCK_CONST_METHOD0(getPoisonValue, short int());
    MOCK_METHOD1(setPoisonValue, void(short int));
    MOCK_CONST_METHOD0(getMentalCapacity, int());
    MOCK_METHOD1(setMentalCapacity, void(int));
    MOCK_METHOD1(increaseMentalCapacity, void(int));
    MOCK_METHOD1(setClippingActive, void(bool));
    MOCK_CONST_METHOD0(getClippingActive, bool());
    MOCK_CONST_METHOD1(countItem, int(TYPE_OF_ITEM_ID));
    MOCK_METHOD2(increaseAtPos, int(unsigned char, int));
    MOCK_METHOD3(swapAtPos, bool(unsigned char, TYPE_OF_ITEM_ID, uint16_t));
    MOCK_METHOD3(createAtPos, int(unsigned char, TYPE_OF_ITEM_ID, int));
    MOCK_METHOD1(GetItemAt, ScriptItem(unsigned char));
    MOCK_CONST_METHOD1(getSkillName, std::string(TYPE_OF_SKILL_ID));
    MOCK_CONST_METHOD1(getSkill, unsigned short(TYPE_OF_SKILL_ID));
    MOCK_CONST_METHOD1(getMinorSkill, unsigned short(TYPE_OF_SKILL_ID));
};

using ::testing::Return;
using ::testing::AtLeast;
using ::testing::_;
using ::testing::Property;
using ::testing::SaveArg;
using ::testing::SetArgReferee;

class monster_bindings : public ::testing::Test {
public:
    MockWorld world;
    MockMonster *monster;

    ~monster_bindings() override {
        LuaScript::shutdownLua();
        delete monster;
    }

    monster_bindings() {
        monster = new MockMonster();
        ON_CALL(world, findCharacter(monster->getId())).WillByDefault(Return(monster));
        EXPECT_CALL(world, findCharacter(monster->getId())).Times(AtLeast(0));
    }
};

TEST_F(monster_bindings, isNewPlayer) {
    LuaTestSupportScript script {"function test(monster) return monster:isNewPlayer() end"};
    EXPECT_CALL(*monster, isNewPlayer()).WillOnce(Return(true));
    bool result = script.test<bool, Monster *>(monster);
    EXPECT_TRUE(result);
}

TEST_F(monster_bindings, pageGM) {
    LuaTestSupportScript script {"function test(monster) return monster:pageGM('test') end"};
    EXPECT_CALL(*monster, pageGM("test")).WillOnce(Return(true));
    bool result = script.test<bool, Monster *>(monster);
    EXPECT_TRUE(result);
}

TEST_F(monster_bindings, requestInputDialog) {
    LuaTestSupportScript script {"function test(monster) \
                                      dialog = InputDialog('foo', '', false, 0, nil) \
                                      monster:requestInputDialog(dialog) \
                                      dialog = nil \
                                      collectgarbage() \
                                  end"};
    InputDialog *dialog = nullptr;
    EXPECT_CALL(*monster, requestInputDialog(Property(&InputDialog::getTitle, "foo"))).WillOnce(SaveArg<0>(&dialog));
    script.test<bool, Monster *>(monster);
    delete dialog;
}

TEST_F(monster_bindings, requestMessageDialog) {
    LuaTestSupportScript script {"function test(monster) \
                                      dialog = MessageDialog('foo', '', nil) \
                                      monster:requestMessageDialog(dialog) \
                                      dialog = nil \
                                      collectgarbage() \
                                  end"};
    MessageDialog *dialog = nullptr;
    EXPECT_CALL(*monster, requestMessageDialog(Property(&MessageDialog::getTitle, "foo"))).WillOnce(SaveArg<0>(&dialog));
    script.test<bool, Monster *>(monster);
    delete dialog;
}

TEST_F(monster_bindings, requestMerchantDialog) {
    LuaTestSupportScript script {"function test(monster) \
                                      dialog = MerchantDialog('foo', nil) \
                                      monster:requestMerchantDialog(dialog) \
                                      dialog = nil \
                                      collectgarbage() \
                                  end"};
    MerchantDialog *dialog = nullptr;
    EXPECT_CALL(*monster, requestMerchantDialog(Property(&MerchantDialog::getTitle, "foo"))).WillOnce(SaveArg<0>(&dialog));
    script.test<bool, Monster *>(monster);
    delete dialog;
}

TEST_F(monster_bindings, requestSelectionDialog) {
    LuaTestSupportScript script {"function test(monster) \
                                      dialog = SelectionDialog('foo', '', nil) \
                                      monster:requestSelectionDialog(dialog) \
                                      dialog = nil \
                                      collectgarbage() \
                                  end"};
    SelectionDialog *dialog = nullptr;
    EXPECT_CALL(*monster, requestSelectionDialog(Property(&SelectionDialog::getTitle, "foo"))).WillOnce(SaveArg<0>(&dialog));
    script.test<bool, Monster *>(monster);
    delete dialog;
}

TEST_F(monster_bindings, requestCraftingDialog) {
    LuaTestSupportScript script {"function test(monster) \
                                      dialog = CraftingDialog('foo', 0, 0, nil) \
                                      monster:requestCraftingDialog(dialog) \
                                      dialog = nil \
                                      collectgarbage() \
                                  end"};
    CraftingDialog *dialog = nullptr;
    EXPECT_CALL(*monster, requestCraftingDialog(Property(&CraftingDialog::getTitle, "foo"))).WillOnce(SaveArg<0>(&dialog));
    script.test<bool, Monster *>(monster);
    delete dialog;
}

TEST_F(monster_bindings, idleTime) {
    LuaTestSupportScript script {"function test(monster) return monster:idleTime() end"};
    EXPECT_CALL(*monster, idleTime()).WillOnce(Return(11));
    auto result = script.test<uint32_t, Monster *>(monster);
    EXPECT_EQ(11, result);
}

TEST_F(monster_bindings, sendBook) {
    LuaTestSupportScript script {"function test(monster) monster:sendBook(123) end"};
    EXPECT_CALL(*monster, sendBook(123));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, updateAppearance) {
    LuaTestSupportScript script {"function test(monster) monster:updateAppearance() end"};
    EXPECT_CALL(*monster, forceUpdateAppearanceForAll());
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, performAnimation) {
    LuaTestSupportScript script {"function test(monster) monster:performAnimation(47) end"};
    EXPECT_CALL(*monster, performAnimation(47));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, alterSpokenMessage) {
    LuaTestSupportScript script {"function test(monster) return monster:alterSpokenMessage('foo', 23) end"};
    EXPECT_CALL(*monster, alterSpokenMessage("foo", 23)).WillOnce(Return("bar"));
    auto result = script.test<std::string, Monster *>(monster);
    EXPECT_EQ("bar", result);
}

TEST_F(monster_bindings, actionRunning) {
    LuaTestSupportScript script {"function test(monster) return monster:actionRunning() end"};
    EXPECT_CALL(*monster, actionRunning()).WillOnce(Return(true));
    bool result = script.test<bool, Monster *>(monster);
    EXPECT_TRUE(result);
}

TEST_F(monster_bindings, changeQualityAt) {
    LuaTestSupportScript script {"function test(monster) monster:changeQualityAt(4, 567) end"};
    EXPECT_CALL(*monster, changeQualityAt(4, 567));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, isAdmin) {
    LuaTestSupportScript script {"function test(monster) return monster:isAdmin() end"};
    EXPECT_CALL(*monster, isAdmin()).WillOnce(Return(true));
    bool result = script.test<bool, Monster *>(monster);
    EXPECT_TRUE(result);
}

TEST_F(monster_bindings, talk1) {
    LuaTestSupportScript script {"function test(monster) monster:talk(Character.whisper, 'foobar') end"};
    EXPECT_CALL(*monster, talk(Character::tt_whisper, "foobar"));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, talk2) {
    LuaTestSupportScript script {"function test(monster) monster:talk(Character.yell, 'de', 'en') end"};
    EXPECT_CALL(*monster, talk(Character::tt_yell, "de", "en"));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, sendCharDescription) {
    LuaTestSupportScript script {"function test(monster) monster:sendCharDescription(12, 'foo') end"};
    EXPECT_CALL(*monster, sendCharDescription(12, "foo"));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, startAction) {
    LuaTestSupportScript script {"function test(monster) monster:startAction(1, 2, 3, 4, 5) end"};
    EXPECT_CALL(*monster, startAction(1, 2, 3, 4, 5));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, abortAction) {
    LuaTestSupportScript script {"function test(monster) monster:abortAction() end"};
    EXPECT_CALL(*monster, abortAction());
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, successAction) {
    LuaTestSupportScript script {"function test(monster) monster:successAction() end"};
    EXPECT_CALL(*monster, successAction());
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, disturbAction) {
    LuaTestSupportScript script {"function test(monster) monster:disturbAction(monster) end"};
    EXPECT_CALL(*monster, actionDisturbed(monster));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, changeSource_character) {
    LuaTestSupportScript script {"function test(monster) monster:changeSource(monster) end"};
    EXPECT_CALL(*monster, changeSource(monster));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, changeSource_scriptitem) {
    LuaTestSupportScript script {"function test(monster, item) monster:changeSource(item) end"};
    ScriptItem item;
    item.setId(47);
    EXPECT_CALL(*monster, changeSource(item));
    script.test<bool, Monster *, ScriptItem>(monster, item);
}

TEST_F(monster_bindings, changeSource_position) {
    LuaTestSupportScript script {"function test(monster) monster:changeSource(position(1, 2, 3)) end"};
    EXPECT_CALL(*monster, changeSource(position(1, 2, 3)));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, changeSource_empty) {
    LuaTestSupportScript script {"function test(monster) monster:changeSource() end"};
    EXPECT_CALL(*monster, changeSource());
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, changeTarget_character) {
    LuaTestSupportScript script {"function test(monster) monster:changeTarget(monster) end"};
    EXPECT_CALL(*monster, changeTarget(monster));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, changeTarget_scriptitem) {
    LuaTestSupportScript script {"function test(monster, item) monster:changeTarget(item) end"};
    ScriptItem item;
    item.setId(47);
    EXPECT_CALL(*monster, changeTarget(item));
    script.test<bool, Monster *, ScriptItem>(monster, item);
}

TEST_F(monster_bindings, changeTarget_position) {
    LuaTestSupportScript script {"function test(monster) monster:changeTarget(position(1, 2, 3)) end"};
    EXPECT_CALL(*monster, changeTarget(position(1, 2, 3)));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, changeTarget_empty) {
    LuaTestSupportScript script {"function test(monster) monster:changeTarget() end"};
    EXPECT_CALL(*monster, changeTarget());
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, introduce) {
    LuaTestSupportScript script {"function test(monster, player) monster:introduce(player) end"};
    Player player;
    EXPECT_CALL(*monster, introducePlayer(&player));
    script.test<bool, Monster *, Player *>(monster, &player);
}

TEST_F(monster_bindings, move) {
    LuaTestSupportScript script {"function test(monster) return monster:move(Character.dir_south, true) end"};
    EXPECT_CALL(*monster, move(dir_south, true)).WillOnce(Return(true));
    bool result = script.test<bool, Monster *>(monster);
    EXPECT_TRUE(result);
}

TEST_F(monster_bindings, turn_direction) {
    LuaTestSupportScript script {"function test(monster) monster:turn(Character.dir_west) end"};
    EXPECT_CALL(*monster, turn(dir_west));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, turn_position) {
    LuaTestSupportScript script {"function test(monster) monster:turn(position(1, 2, 3)) end"};
    EXPECT_CALL(*monster, turn(position(1, 2, 3)));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, getNextStepDir_result1) {
    LuaTestSupportScript script {"function test(monster) success, dir = monster:getNextStepDir(position(1, 2, 3)) return success end"};
    EXPECT_CALL(*monster, getNextStepDir(position(1, 2, 3), _)).WillOnce(Return(true));
    bool result = script.test<bool, Monster *>(monster);
    EXPECT_TRUE(result);
}
/*
TEST_F(monster_bindings, getNextStepDir_result2) {
    LuaTestSupportScript script {"function test(monster) success, dir = monster:getNextStepDir(position(1, 2, 3)) return dir end"};
    EXPECT_CALL(*monster, getNextStepDir(position(1, 2, 3), _)).WillOnce(SetArgReferee<1>(dir_east));
    direction result = script.test<direction, Monster *>(monster);
    EXPECT_EQ(dir_east, result);
}
*/
TEST_F(monster_bindings, setRace) {
    LuaTestSupportScript script {"function test(monster) monster:setRace(55) end"};
    EXPECT_CALL(*monster, changeRace(55));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, getRace) {
    LuaTestSupportScript script {"function test(monster) return monster:getRace() end"};
    EXPECT_CALL(*monster, getRace()).WillOnce(Return(44));
    auto result = script.test<TYPE_OF_RACE_ID, Monster *>(monster);
    EXPECT_EQ(44, result);
}

TEST_F(monster_bindings, getFaceTo) {
    LuaTestSupportScript script {"function test(monster) return monster:getFaceTo() end"};
    EXPECT_CALL(*monster, getFaceTo()).WillOnce(Return(Character::northwest));
    auto result = script.test<Character::face_to, Monster *>(monster);
    EXPECT_EQ(Character::northwest, result);
}

TEST_F(monster_bindings, getType) {
    LuaTestSupportScript script {"function test(monster) return monster:getType() end"};
    EXPECT_CALL(*monster, getType()).WillOnce(Return(33));
    auto result = script.test<unsigned short, Monster *>(monster);
    EXPECT_EQ(33, result);
}

TEST_F(monster_bindings, increasePoisonValue) {
    LuaTestSupportScript script {"function test(monster) monster:increasePoisonValue(53) end"};
    EXPECT_CALL(*monster, increasePoisonValue(53));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, getPoisonValue) {
    LuaTestSupportScript script {"function test(monster) return monster:getPoisonValue() end"};
    EXPECT_CALL(*monster, getPoisonValue()).WillOnce(Return(87));
    auto result = script.test<short int, Monster *>(monster);
    EXPECT_EQ(87, result);
}

TEST_F(monster_bindings, setPoisonValue) {
    LuaTestSupportScript script {"function test(monster) monster:setPoisonValue(77) end"};
    EXPECT_CALL(*monster, setPoisonValue(77));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, getMentalCapacity) {
    LuaTestSupportScript script {"function test(monster) return monster:getMentalCapacity() end"};
    EXPECT_CALL(*monster, getMentalCapacity()).WillOnce(Return(14));
    auto result = script.test<int, Monster *>(monster);
    EXPECT_EQ(14, result);
}

TEST_F(monster_bindings, setMentalCapacity) {
    LuaTestSupportScript script {"function test(monster) monster:setMentalCapacity(13) end"};
    EXPECT_CALL(*monster, setMentalCapacity(13));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, increaseMentalCapacity) {
    LuaTestSupportScript script {"function test(monster) monster:increaseMentalCapacity(-12) end"};
    EXPECT_CALL(*monster, increaseMentalCapacity(-12));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, setClippingActive) {
    LuaTestSupportScript script {"function test(monster) monster:setClippingActive(true) end"};
    EXPECT_CALL(*monster, setClippingActive(true));
    script.test<bool, Monster *>(monster);
}

TEST_F(monster_bindings, getClippingActive) {
    LuaTestSupportScript script {"function test(monster) return monster:getClippingActive() end"};
    EXPECT_CALL(*monster, getClippingActive()).WillOnce(Return(true));
    auto result = script.test<bool, Monster *>(monster);
    EXPECT_TRUE(result);
}

TEST_F(monster_bindings, countItem) {
    LuaTestSupportScript script {"function test(monster) return monster:countItem(111) end"};
    EXPECT_CALL(*monster, countItem(111)).WillOnce(Return(19));
    auto result = script.test<TYPE_OF_ITEM_ID, Monster *>(monster);
    EXPECT_EQ(19, result);
}

TEST_F(monster_bindings, increaseAtPos) {
    LuaTestSupportScript script {"function test(monster) return monster:increaseAtPos(5, -2) end"};
    EXPECT_CALL(*monster, increaseAtPos(5, -2)).WillOnce(Return(8));
    auto result = script.test<int, Monster *>(monster);
    EXPECT_EQ(8, result);
}

TEST_F(monster_bindings, swapAtPos) {
    LuaTestSupportScript script {"function test(monster) return monster:swapAtPos(6, -9, 111) end"};
    EXPECT_CALL(*monster, swapAtPos(6, -9, 111)).WillOnce(Return(true));
    auto result = script.test<bool, Monster *>(monster);
    EXPECT_TRUE(result);
}

TEST_F(monster_bindings, createAtPos) {
    LuaTestSupportScript script {"function test(monster) return monster:createAtPos(4, 1111, 92) end"};
    EXPECT_CALL(*monster, createAtPos(4, 1111, 92)).WillOnce(Return(58));
    auto result = script.test<int, Monster *>(monster);
    EXPECT_EQ(58, result);
}

TEST_F(monster_bindings, getItemAt) {
    LuaTestSupportScript script {"function test(monster) return monster:getItemAt(13) end"};
    ScriptItem item;
    item.setId(27);
    EXPECT_CALL(*monster, GetItemAt(13)).WillOnce(Return(item));
    auto result = script.test<ScriptItem, Monster *>(monster);
    EXPECT_EQ(27, result.getId());
}

TEST_F(monster_bindings, getSkillName) {
    LuaTestSupportScript script {"function test(monster) return monster:getSkillName(12) end"};
    EXPECT_CALL(*monster, getSkillName(12)).WillOnce(Return("someskill"));
    auto result = script.test<std::string, Monster *>(monster);
    EXPECT_EQ("someskill", result);
}

TEST_F(monster_bindings, getSkill) {
    LuaTestSupportScript script {"function test(monster) return monster:getSkill(11) end"};
    EXPECT_CALL(*monster, getSkill(11)).WillOnce(Return(77));
    auto result = script.test<unsigned short, Monster *>(monster);
    EXPECT_EQ(77, result);
}

TEST_F(monster_bindings, getMinorSkill) {
    LuaTestSupportScript script {"function test(monster) return monster:getMinorSkill(10) end"};
    EXPECT_CALL(*monster, getMinorSkill(10)).WillOnce(Return(74));
    auto result = script.test<unsigned short, Monster *>(monster);
    EXPECT_EQ(74, result);
}

auto main(int argc, char **argv) -> int {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

