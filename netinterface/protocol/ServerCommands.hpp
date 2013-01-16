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

#ifndef _SERVER_COMMANDS_HPP_
#define _SERVER_COMMANDS_HPP_

#include "NewClientView.hpp"
#include "netinterface/BasicServerCommand.hpp"

struct WeatherStruct;
class Item;
class ItemLookAt;
typedef std::vector < Item > ITEMVECTOR;

enum ServerCommands {
    SC_ID_TC = 0xCA,
    SC_SETCOORDINATE_TC = 0xBD,
    SC_MAPSTRIPE_TC = 0xA1,
    SC_MAPCOMPLETE_TC = 0xA2,
    SC_PLAYERSPIN_TC = 0xE0,
    SC_UPDATEINVENTORYPOS_TC = 0xC1,
    SC_CLEARSHOWCASE_TC = 0xC4,
    SC_UPDATEWEATHER_TC = 0xB7,
    SC_UPDATESKILL_TC = 0xD1,
    SC_UPDATEMAGICFLAGS_TC = 0xB8,
    SC_UPDATEATTRIB_TC = 0xB9,
    SC_MUSIC_TC = 0xC8,
    SC_MUSICDEFAULT_TC = 0xCE,
    SC_SAY_TC = 0xD7,
    SC_WHISPER_TC = 0xD5,
    SC_SHOUT_TC = 0xD6,
    SC_INFORM_TC = 0xD8,
    SC_INTRODUCE_TC = 0xD4,
    SC_MOVEACK_TC = 0xDF,
    SC_UPDATESHOWCASE_TC = 0xC5,
    SC_MAPITEMSWAP = 0xD9,
    SC_GRAPHICEFFECT_TC = 0xC9,
    SC_SOUND_TC = 0xC7,
    SC_ANIMATION_TC = 0xCB,
    SC_BOOK_TC = 0xCD,
    SC_ADMINVIEWPLAYERS_TC = 0xC6,
    SC_ITEMREMOVE_TC = 0xC3,
    SC_ITEMPUT_TC = 0xC2,
    SC_LOOKATMAPITEM_TC = 0xC0,
    SC_LOOKATTILE_TC = 0xBC,
    SC_LOOKATSHOWCASEITEM_TC = 0xBF,
    SC_LOOKATINVENTORYITEM_TC = 0xBE,
    SC_LOOKATDIALOGITEM_TC = 0xB5,
    SC_ATTACKACKNOWLEDGED_TC = 0xBB,
    SC_TARGETLOST_TC = 0xBA,
    SC_LOGOUT_TC = 0xCC,
    SC_UPDATETIME_TC = 0xB6,
    SC_APPEARANCE_TC = 0xE1,
    SC_REMOVECHAR_TC = 0xE2,
    SC_LOOKATCHARRESULT_TC = 0x18,
    SC_ITEMUPDATE_TC = 0x19,
    SC_INPUTDIALOG_TC = 0x50,
    SC_MESSAGEDIALOG_TC = 0x51,
    SC_MERCHANTDIALOG_TC = 0x52,
    SC_SELECTIONDIALOG_TC = 0x53,
    SC_CRAFTINGDIALOG_TC = 0x54,
    SC_CRAFTINGDIALOGUPDATE_TC = 0x55,
    SC_CLOSEDIALOG_TC = 0x5F
};

class InputDialog;
class InputDialogTC : public BasicServerCommand {
public:
    InputDialogTC(InputDialog &inputDialog, unsigned int dialogId);
};

class MessageDialog;
class MessageDialogTC : public BasicServerCommand {
public:
    MessageDialogTC(MessageDialog &messageDialog, unsigned int dialogId);
};

class MerchantDialog;
class MerchantDialogTC : public BasicServerCommand {
public:
    MerchantDialogTC(MerchantDialog &merchantDialog, unsigned int dialogId);
};

class SelectionDialog;
class SelectionDialogTC : public BasicServerCommand {
public:
    SelectionDialogTC(SelectionDialog &selectionDialog, unsigned int dialogId);
};

class CraftingDialog;
class CraftingDialogTC : public BasicServerCommand {
public:
    CraftingDialogTC(CraftingDialog &craftingDialog, unsigned int dialogId);
};
class CraftingDialogCraftTC : public BasicServerCommand {
public:
    CraftingDialogCraftTC(uint8_t stillToCraft, uint16_t craftingTime, unsigned int dialogId);
};
class CraftingDialogCraftingCompleteTC : public BasicServerCommand {
public:
    CraftingDialogCraftingCompleteTC(unsigned int dialogId);
};
class CraftingDialogCraftingAbortedTC : public BasicServerCommand {
public:
    CraftingDialogCraftingAbortedTC(unsigned int dialogId);
};

class CloseDialogTC : public BasicServerCommand {
public:
    CloseDialogTC(unsigned int dialogId);
};

class ItemUpdate_TC : public BasicServerCommand {
public:
    ItemUpdate_TC(position fieldpos, ITEMVECTOR &items);
    ItemUpdate_TC(int16_t px, int16_t py, int16_t pz, ITEMVECTOR &items);
};

class CharDescription : public BasicServerCommand {
public:
    CharDescription(TYPE_OF_CHARACTER_ID id, std::string description);
};

class Character;
class AppearanceTC : public BasicServerCommand {
public:
    AppearanceTC(Character *cc, Player *receivingPlayer);
};

class AnimationTC : public BasicServerCommand {
public:
    AnimationTC(TYPE_OF_CHARACTER_ID id, uint8_t animID);
};

class BookTC : public BasicServerCommand {
public:
    BookTC(uint16_t bookID);
};


class RemoveCharTC : public BasicServerCommand {
public:
    RemoveCharTC(TYPE_OF_CHARACTER_ID id);
};

class UpdateTimeTC : public BasicServerCommand {
public:
    UpdateTimeTC(unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, short int year);
};

class LogOutTC : public BasicServerCommand {
public:
    LogOutTC(unsigned char reason);
};

class TargetLostTC : public BasicServerCommand {
public:
    TargetLostTC();
};

class AttackAcknowledgedTC : public BasicServerCommand {
public:
    AttackAcknowledgedTC();
};

void addItemLookAt(const ItemLookAt &lookAt);

class LookAtInventoryItemTC : public BasicServerCommand {
public:
    LookAtInventoryItemTC(unsigned char pos, const ItemLookAt &lookAt);
};

class LookAtShowCaseItemTC : public BasicServerCommand {
public:
    LookAtShowCaseItemTC(unsigned char showcase, unsigned char pos, const ItemLookAt &lookAt);
};

class LookAtMapItemTC : public BasicServerCommand {
public:
    LookAtMapItemTC(short int x, short int y, short int z, const ItemLookAt &lookAt);
};

class LookAtDialogItemTC : public BasicServerCommand {
public:
    LookAtDialogItemTC(unsigned int dialogId, uint8_t itemIndex, const ItemLookAt &lookAt);
};

class LookAtCraftingDialogIngredientTC : public BasicServerCommand {
public:
    LookAtCraftingDialogIngredientTC(unsigned int dialogId, uint8_t itemIndex, uint8_t ingredientIndex, const ItemLookAt &lookAt);
};

class LookAtTileTC : public BasicServerCommand {
public:
    LookAtTileTC(short int x, short int y, short int z, const std::string &lookAt);
};

class ItemPutTC : public BasicServerCommand {
public:
    ItemPutTC(short int x, short int y, short int z, Item &item);
};

class ItemSwapTC : public BasicServerCommand {
public:
    ItemSwapTC(position pos, unsigned short int id, Item &item);
    ItemSwapTC(short int x, short int y, short int z, unsigned short int id, Item &item);
};

class ItemRemoveTC : public BasicServerCommand {
public:
    ItemRemoveTC(short int x, short int y, short int z);
};

class AdminViewPlayersTC : public BasicServerCommand {
public:
    AdminViewPlayersTC();
};

class SoundTC : public BasicServerCommand {
public:
    SoundTC(short int x, short int y, short int z, unsigned short int id);
};

class GraphicEffectTC : public BasicServerCommand {
public:
    GraphicEffectTC(short int x, short int y, short int z, unsigned short int id);
};

class PushPlayerTC : public BasicServerCommand {
public:
    PushPlayerTC(char xoffs, char yoffs, uint32_t id, unsigned short int appearance, unsigned char direction);
};

class UpdateShowCaseTC : public BasicServerCommand {
public:
    UpdateShowCaseTC(unsigned char showcase, const TYPE_OF_CONTAINERSLOTS volume, const Container::ITEMMAP &items);
};

class MapStripeTC : public BasicServerCommand {
public:
    MapStripeTC(position pos, NewClientView::stripedirection dir);
};

class MapCompleteTC : public BasicServerCommand {
public:
    MapCompleteTC();
};

class MoveAckTC : public BasicServerCommand {
public:
    MoveAckTC(TYPE_OF_CHARACTER_ID id, position pos, unsigned char mode, unsigned char waitpages);
};

class IntroduceTC : public BasicServerCommand {
public:
    IntroduceTC(TYPE_OF_CHARACTER_ID id, std::string name);
};

class ShoutTC : public BasicServerCommand {
public:
    ShoutTC(int16_t x, int16_t y, int16_t z, std::string text);
};

class WhisperTC : public BasicServerCommand {
public:
    WhisperTC(int16_t x, int16_t y, int16_t z, std::string text);
};

class SayTC : public BasicServerCommand {
public:
    SayTC(int16_t x, int16_t y, int16_t z, std::string text);
};

class InformTC : public BasicServerCommand {
public:
    InformTC(Character::informType type, std::string text);
};

class MusicTC : public BasicServerCommand {
public:
    MusicTC(short int title);
};

class MusicDefaultTC : public BasicServerCommand {
public:
    MusicDefaultTC();
};

class UpdateAttribTC : public BasicServerCommand {
public:
    UpdateAttribTC(TYPE_OF_CHARACTER_ID id, std::string name, short int value);
};

class UpdateMagicFlagsTC : public BasicServerCommand {
public:
    UpdateMagicFlagsTC(unsigned char type, uint32_t flags);
};

class ClearShowCaseTC : public BasicServerCommand {
public:
    ClearShowCaseTC(unsigned char id);
};

class UpdateSkillTC : public BasicServerCommand {
public:
    UpdateSkillTC(TYPE_OF_SKILL_ID skill, unsigned short int major, unsigned short int minor);
};

class UpdateWeatherTC : public BasicServerCommand {
public:
    UpdateWeatherTC(WeatherStruct weather);
    UpdateWeatherTC(uint8_t cloudDensity, uint8_t fogDensity,
                    uint8_t windDirection, uint8_t windForce, uint8_t precipitationStrength,
                    uint8_t precipitationType, uint8_t thunderstorm, uint8_t temperature);
};

class IdTC : public BasicServerCommand {
public:
    IdTC(int id);
};

class UpdateInventoryPosTC : public BasicServerCommand {
public:
    UpdateInventoryPosTC(unsigned char pos, TYPE_OF_ITEM_ID id, Item::number_type number);
};

class SetCoordinateTC : public BasicServerCommand {
public:
    SetCoordinateTC(position pos);
    SetCoordinateTC(short int x, short int y, short int z);
};

class PlayerSpinTC : public BasicServerCommand {
public:
    PlayerSpinTC(unsigned char faceto, TYPE_OF_CHARACTER_ID id);
};

#endif

