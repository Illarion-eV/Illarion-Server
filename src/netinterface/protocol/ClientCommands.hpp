/*
 * illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of illarionserver.
 *
 * illarionserver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * illarionserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
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
    C_IMOVERSTART_TS = 0x20,
    C_IMOVEREND_TS = 0x2A,
    C_PSPINRSTART_TS = 0x40,
    C_PSPINREND_TS = 0x49,
    C_LOOKATMAPITEM_TS = 0xFF,
    C_LOOKATCHARACTER_TS = 0x18,
    C_USE_TS = 0xFE,
    C_CAST_TS = 0xFD,
    C_ATTACKPLAYER_TS = 0xFA,
    C_INTRODUCE_TS = 0xF6,
    C_SAY_TS = 0xF5,
    C_SHOUT_TS = 0xF4,
    C_WHISPER_TS = 0xF3,
    C_REFRESH_TS = 0xF2,
    C_LOGOUT_TS = 0xF1,
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
    unsigned int dialogId;
    bool success;
    std::string input;

public:
    InputDialogTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MessageDialogTS : public BasicClientCommand {
private:
    unsigned int dialogId;

public:
    MessageDialogTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MerchantDialogTS : public BasicClientCommand {
private:
    unsigned int dialogId;
    uint8_t result;
    uint8_t purchaseIndex;
    uint16_t purchaseAmount;
    uint8_t saleLocation;
    uint16_t saleSlot;
    uint16_t saleAmount;

public:
    MerchantDialogTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class SelectionDialogTS : public BasicClientCommand {
private:
    unsigned int dialogId;
    bool success;
    uint8_t selectedIndex;

public:
    SelectionDialogTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class CraftingDialogTS : public BasicClientCommand {
private:
    unsigned int dialogId;
    uint8_t result;
    uint8_t craftIndex;
    uint8_t craftAmount;
    uint8_t craftIngredient;

public:
    CraftingDialogTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class RequestAppearanceTS : public BasicClientCommand {
private:
    TYPE_OF_CHARACTER_ID id;

public:
    RequestAppearanceTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LookAtCharacterTS : public BasicClientCommand {
private:
    TYPE_OF_CHARACTER_ID id;
    uint8_t mode;

public:
    LookAtCharacterTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class CastTS : public BasicClientCommand {
private:
    unsigned char showcase;
    unsigned char pos;
    position castPosition;
    unsigned char cid;
    unsigned long int spellId;

public:
    CastTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class UseTS : public BasicClientCommand {
private:
    unsigned char useId;
    unsigned char showcase;
    unsigned char pos;
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
    unsigned char pos;

public:
    LookAtInventoryItemTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LookAtShowCaseItemTS : public BasicClientCommand {
private:
    unsigned char showcase;
    unsigned char pos;

public:
    LookAtShowCaseItemTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemFromPlayerToShowCaseTS : public BasicClientCommand {
private:
    unsigned char showcase;
    unsigned char pos;
    unsigned char cpos;
    unsigned short count;

public:
    MoveItemFromPlayerToShowCaseTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemFromShowCaseToPlayerTS : public BasicClientCommand {
private:
    unsigned char showcase;
    unsigned char pos;
    unsigned char cpos;
    unsigned short count;

public:
    MoveItemFromShowCaseToPlayerTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemInsideInventoryTS : public BasicClientCommand {
private:
    unsigned char opos;
    unsigned char npos;
    unsigned short count;

public:
    MoveItemInsideInventoryTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class DropItemFromInventoryOnMapTS : public BasicClientCommand {
private:
    unsigned char pos;
    position mapPosition;
    unsigned short count;

public:
    DropItemFromInventoryOnMapTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemFromMapToPlayerTS : public BasicClientCommand {
private:
    direction dir;
    unsigned char pos;
    unsigned short count;

public:
    MoveItemFromMapToPlayerTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemFromMapIntoShowCaseTS : public BasicClientCommand {
private:
    direction dir;
    unsigned char showcase;
    unsigned char pos;
    unsigned short count;

public:
    MoveItemFromMapIntoShowCaseTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class MoveItemBetweenShowCasesTS : public BasicClientCommand {
private:
    unsigned char source;
    unsigned char spos;
    unsigned char dest;
    unsigned char dpos;
    unsigned short count;

public:
    MoveItemBetweenShowCasesTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class DropItemFromShowCaseOnMapTS : public BasicClientCommand {
private:
    unsigned char showcase;
    unsigned char pos;
    position mapPosition;
    unsigned short count;

public:
    DropItemFromShowCaseOnMapTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class CloseContainerInShowCaseTS : public BasicClientCommand {
private:
    unsigned char showcase;

public:
    CloseContainerInShowCaseTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LookIntoShowCaseContainerTS : public BasicClientCommand {
    unsigned char showcase;
    unsigned char pos;

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
    direction dir;

public:
    LookIntoContainerOnFieldTS();
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


class AttackPlayerTS : public BasicClientCommand {
private:
    uint32_t enemyid;

public:
    AttackPlayerTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LookAtMapItemTS : public BasicClientCommand {
private:
    position pos;

public:
    LookAtMapItemTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class PSpinActionTS : public BasicClientCommand {
private:
    uint8_t direction;

public:
    PSpinActionTS(uint8_t dir);
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class CharMoveTS : public BasicClientCommand {
private:
    TYPE_OF_CHARACTER_ID charid;
    unsigned char dir;
    unsigned char mode;

public:
    CharMoveTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class IMoverActionTS : public BasicClientCommand {
private:
    position pos;
    uint16_t count;
    direction dir;

public:
    IMoverActionTS(uint8_t dir);
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};


class LoginCommandTS : public BasicClientCommand {
private:
    unsigned short clientVersion;
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
    uint8_t width;
    uint8_t height;

public:
    ScreenSizeCommandTS();
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

#endif

