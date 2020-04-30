/*
 * illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of illarionserver.
 *
 * illarionserver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * illarionserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CLIENT_COMMANDS_HPP_
#define _CLIENT_COMMANDS_HPP_

#include <string>
#include "types.hpp"
#include "globals.hpp"
#include "netinterface/BasicClientCommand.hpp"

enum clientcommands {
    C_LOGIN_TS = 0x0D,
    C_SCREENSIZE_TS = 0xA0,
    C_CHARMOVE_TS = 0x10,
    C_PLAYERSPIN_TS = 0x11,
    C_LOOKATMAPITEM_TS = 0xFF,
    C_LOOKATCHARACTER_TS = 0x18,
    C_USE_TS = 0xFE,
    C_CAST_TS = 0xFD,
    C_ATTACKPLAYER_TS = 0xFA,
    C_CUSTOMNAME_TS = 0xF7,
    C_INTRODUCE_TS = 0xF6,
    C_SAY_TS = 0xF5,
    C_SHOUT_TS = 0xF4,
    C_WHISPER_TS = 0xF3,
    C_REFRESH_TS = 0xF2,
    C_LOGOUT_TS = 0xF1,
    C_MOVEITEMFROMMAPTOMAP_TS = 0xEF,
    C_PICKUPALLITEMS_TS = 0xEE,
    C_PICKUPITEM_TS = 0xED,
    C_LOOKINTOCONTAINERONFIELD_TS = 0xEC,
    C_LOOKINTOINVENTORY_TS = 0xEB,
    C_LOOKINTOSHOWCASECONTAINER_TS = 0xEA,
    C_CLOSECONTAINERINSHOWCASE_TS = 0xE9,
    C_DROPITEMFROMSHOWCASEONMAP_TS = 0xE8,
    C_MOVEITEMBETWEENSHOWCASES_TS = 0xE7,
    C_MOVEITEMFROMMAPINTOSHOWCASE_TS = 0xE6,
    C_MOVEITEMFROMMAPTOPLAYER_TS = 0xE5,
    C_DROPITEMFROMPLAYERONMAP_TS = 0xE4,
    C_MOVEITEMINSIDEINVENTORY_TS = 0xE3,
    C_MOVEITEMFROMSHOWCASETOPLAYER_TS = 0xE2,
    C_MOVEITEMFROMPLAYERTOSHOWCASE_TS = 0xE1,
    C_LOOKATSHOWCASEITEM_TS = 0xE0,
    C_LOOKATINVENTORYITEM_TS = 0xDF,
    C_ATTACKSTOP_TS = 0xDE,
    C_REQUESTSKILLS_TS = 0xDD,
    C_KEEPALIVE_TS = 0xD8,
    C_REQUESTAPPEARANCE_TS = 0x0E,
    C_INPUTDIALOG_TS = 0x50,
    C_MESSAGEDIALOG_TS = 0x51,
    C_MERCHANTDIALOG_TS = 0x52,
    C_SELECTIONDIALOG_TS = 0x53,
    C_CRAFTINGDIALOG_TS = 0x54
};

class InputDialogTS : public BasicClientCommand {
private:
    unsigned int dialogId = 0;
    bool success = false;
    std::string input;

public:
    InputDialogTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MessageDialogTS : public BasicClientCommand {
private:
    unsigned int dialogId = 0;

public:
    MessageDialogTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MerchantDialogTS : public BasicClientCommand {
private:
    unsigned int dialogId = 0;
    uint8_t result = 0;
    uint8_t purchaseIndex = 0;
    uint16_t purchaseAmount = 0;
    uint8_t saleLocation = 0;
    uint16_t saleSlot = 0;
    uint16_t saleAmount = 0;
    uint8_t lookAtList = 0;
    uint8_t lookAtSlot = 0;

public:
    MerchantDialogTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class SelectionDialogTS : public BasicClientCommand {
private:
    unsigned int dialogId = 0;
    bool success = false;
    uint8_t selectedIndex = 0;

public:
    SelectionDialogTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class CraftingDialogTS : public BasicClientCommand {
private:
    unsigned int dialogId = 0;
    uint8_t result = 0;
    uint8_t craftIndex = 0;
    uint8_t craftAmount = 0;
    uint8_t craftIngredient = 0;

public:
    CraftingDialogTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class RequestAppearanceTS : public BasicClientCommand {
private:
    TYPE_OF_CHARACTER_ID id = 0;

public:
    RequestAppearanceTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LookAtCharacterTS : public BasicClientCommand {
private:
    TYPE_OF_CHARACTER_ID id = 0;
    uint8_t mode = 0;

public:
    LookAtCharacterTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class CastTS : public BasicClientCommand {
private:
    unsigned char showcase = 0;
    unsigned char pos = 0;
    position castPosition;
    unsigned char cid = 0;
    unsigned long int spellId = 0;

public:
    CastTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class UseTS : public BasicClientCommand {
private:
    unsigned char useId = 0;
    unsigned char showcase = 0;
    unsigned char pos = 0;
    position usePosition;

public:
    UseTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class KeepAliveTS : public BasicClientCommand {
public:
    KeepAliveTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class RequestSkillsTS : public BasicClientCommand {
public:
    RequestSkillsTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class AttackStopTS : public BasicClientCommand {
public:
    AttackStopTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LookAtInventoryItemTS : public BasicClientCommand {
private:
    unsigned char pos = 0;

public:
    LookAtInventoryItemTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LookAtShowCaseItemTS : public BasicClientCommand {
private:
    unsigned char showcase = 0;
    unsigned char pos = 0;

public:
    LookAtShowCaseItemTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemFromPlayerToShowCaseTS : public BasicClientCommand {
private:
    unsigned char showcase = 0;
    unsigned char pos = 0;
    unsigned char cpos = 0;
    unsigned short count = 0;

public:
    MoveItemFromPlayerToShowCaseTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemFromShowCaseToPlayerTS : public BasicClientCommand {
private:
    unsigned char showcase = 0;
    unsigned char pos = 0;
    unsigned char cpos = 0;
    unsigned short count = 0;

public:
    MoveItemFromShowCaseToPlayerTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemInsideInventoryTS : public BasicClientCommand {
private:
    unsigned char opos = 0;
    unsigned char npos = 0;
    unsigned short count = 0;

public:
    MoveItemInsideInventoryTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class DropItemFromInventoryOnMapTS : public BasicClientCommand {
private:
    unsigned char pos = 0;
    position mapPosition;
    unsigned short count = 0;

public:
    DropItemFromInventoryOnMapTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemFromMapToPlayerTS : public BasicClientCommand {
private:
    position sourcePosition;
    unsigned char inventorySlot = 0;
    unsigned short count = 0;

public:
    MoveItemFromMapToPlayerTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemFromMapIntoShowCaseTS : public BasicClientCommand {
private:
    position sourcePosition;
    unsigned char showcase = 0;
    unsigned char showcaseSlot = 0;
    unsigned short count = 0;

public:
    MoveItemFromMapIntoShowCaseTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemFromMapToMapTS : public BasicClientCommand {
private:
    position sourcePosition;
    position targetPosition;
    unsigned short count = 0;

public:
    MoveItemFromMapToMapTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemBetweenShowCasesTS : public BasicClientCommand {
private:
    unsigned char source = 0;
    unsigned char spos = 0;
    unsigned char dest = 0;
    unsigned char dpos = 0;
    unsigned short count = 0;

public:
    MoveItemBetweenShowCasesTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class DropItemFromShowCaseOnMapTS : public BasicClientCommand {
private:
    unsigned char showcase = 0;
    unsigned char pos = 0;
    position mapPosition;
    unsigned short count = 0;

public:
    DropItemFromShowCaseOnMapTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class CloseContainerInShowCaseTS : public BasicClientCommand {
private:
    unsigned char showcase = 0;

public:
    CloseContainerInShowCaseTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LookIntoShowCaseContainerTS : public BasicClientCommand {
    unsigned char showcase = 0;
    unsigned char pos = 0;

public:
    LookIntoShowCaseContainerTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LookIntoInventoryTS : public BasicClientCommand {
public:
    LookIntoInventoryTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class LookIntoContainerOnFieldTS : public BasicClientCommand {
private:
    direction dir = dir_north;

public:
    LookIntoContainerOnFieldTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class PickUpItemTS : public BasicClientCommand {
private:
    position pos;

public:
    PickUpItemTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class PickUpAllItemsTS : public BasicClientCommand {
public:
    PickUpAllItemsTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class LogOutTS : public BasicClientCommand {
public:
    LogOutTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class WhisperTS : public BasicClientCommand {
private:
    std::string text;

public:
    WhisperTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class ShoutTS : public BasicClientCommand {
private:
    std::string text;

public:
    ShoutTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class SayTS : public BasicClientCommand {
private:
    std::string text;

public:
    SayTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class RefreshTS : public BasicClientCommand {
public:
    RefreshTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class IntroduceTS : public BasicClientCommand {
public:
    IntroduceTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class CustomNameTS : public BasicClientCommand {
private:
    TYPE_OF_CHARACTER_ID playerId = 0;
    std::string playerName;
    
public:
    CustomNameTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class AttackPlayerTS : public BasicClientCommand {
private:
    uint32_t enemyid = 0;

public:
    AttackPlayerTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LookAtMapItemTS : public BasicClientCommand {
private:
    position pos;
    uint8_t stackPos = 0;

public:
    LookAtMapItemTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class PlayerSpinTS : public BasicClientCommand {
private:
    direction dir = dir_north;

public:
    PlayerSpinTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class CharMoveTS : public BasicClientCommand {
private:
    TYPE_OF_CHARACTER_ID charid = 0;
    unsigned char dir = 0;
    unsigned char mode = 0;

public:
    CharMoveTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LoginCommandTS : public BasicClientCommand {
private:
    unsigned short clientVersion = 0;
    std::string loginName;
    std::string password;

public:
    LoginCommandTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;

    unsigned short getClientVersion() const;
    const std::string &getLoginName() const;
    const std::string &getPassword() const;
};


class ScreenSizeCommandTS : public BasicClientCommand {
private:
    uint8_t width = 0;
    uint8_t height = 0;

public:
    ScreenSizeCommandTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

#endif

