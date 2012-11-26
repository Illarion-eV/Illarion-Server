//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "globals.hpp"
#include "Item.hpp"
#include "World.hpp"
#include "Logger.hpp"
#include "data/RaceSizeTable.hpp"
#include "ServerCommands.hpp"
#include "netinterface/BasicServerCommand.hpp"
#include "netinterface/NetInterface.hpp"
#include "dialog/InputDialog.hpp"
#include "dialog/MessageDialog.hpp"
#include "dialog/MerchantDialog.hpp"
#include "dialog/SelectionDialog.hpp"
#include "dialog/CraftingDialog.hpp"

extern RaceSizeTable *RaceSizes;

InputDialogTC::InputDialogTC(InputDialog &inputDialog, unsigned int dialogId) : BasicServerCommand(SC_INPUTDIALOG_TC) {
    addStringToBuffer(inputDialog.getTitle());
    addStringToBuffer(inputDialog.getDescription());
    addUnsignedCharToBuffer(inputDialog.isMultiline() ? 1 : 0);
    addShortIntToBuffer(inputDialog.getMaxChars());
    addIntToBuffer(dialogId);
}

MessageDialogTC::MessageDialogTC(MessageDialog &messageDialog, unsigned int dialogId) : BasicServerCommand(SC_MESSAGEDIALOG_TC) {
    addStringToBuffer(messageDialog.getTitle());
    addStringToBuffer(messageDialog.getText());
    addIntToBuffer(dialogId);
}

MerchantDialogTC::MerchantDialogTC(MerchantDialog &merchantDialog, unsigned int dialogId) : BasicServerCommand(SC_MERCHANTDIALOG_TC) {
    addStringToBuffer(merchantDialog.getTitle());
    MerchantDialog::index_type size = merchantDialog.getOffersSize();
    addUnsignedCharToBuffer(size);

    for (auto it = merchantDialog.getOffersBegin(); it != merchantDialog.getOffersEnd(); ++it) {
        OfferProduct &product = *(OfferProduct *)*it;
        addShortIntToBuffer(product.getItem());
        addStringToBuffer(product.getName());
        addIntToBuffer(product.getPrice());
        addShortIntToBuffer(product.getStack());
    }

    size = merchantDialog.getPrimaryRequestsSize();
    addUnsignedCharToBuffer(size);

    for (auto it = merchantDialog.getPrimaryRequestsBegin(); it != merchantDialog.getPrimaryRequestsEnd(); ++it) {
        addShortIntToBuffer((*it)->getItem());
        addStringToBuffer((*it)->getName());
        addIntToBuffer((*it)->getPrice());
    }

    size = merchantDialog.getSecondaryRequestsSize();
    addUnsignedCharToBuffer(size);

    for (auto it = merchantDialog.getSecondaryRequestsBegin(); it != merchantDialog.getSecondaryRequestsEnd(); ++it) {
        addShortIntToBuffer((*it)->getItem());
        addStringToBuffer((*it)->getName());
        addIntToBuffer((*it)->getPrice());
    }

    addIntToBuffer(dialogId);
}

SelectionDialogTC::SelectionDialogTC(SelectionDialog &selectionDialog, unsigned int dialogId) : BasicServerCommand(SC_SELECTIONDIALOG_TC) {
    addStringToBuffer(selectionDialog.getTitle());
    addStringToBuffer(selectionDialog.getText());
    SelectionDialog::index_type size = selectionDialog.getOptionsSize();
    addUnsignedCharToBuffer(size);

    for (auto it = selectionDialog.getOptionsBegin(); it != selectionDialog.getOptionsEnd(); ++it) {
        addShortIntToBuffer((*it)->getItem());
        addStringToBuffer((*it)->getName());
    }

    addIntToBuffer(dialogId);
}

CraftingDialogTC::CraftingDialogTC(CraftingDialog &craftingDialog, unsigned int dialogId) : BasicServerCommand(SC_CRAFTINGDIALOG_TC) {
    addStringToBuffer(craftingDialog.getTitle());
    CraftingDialog::index_t numberOfGroups = craftingDialog.getGroupsSize();
    addUnsignedCharToBuffer(numberOfGroups);

    for (auto it = craftingDialog.getGroupsBegin(); it != craftingDialog.getGroupsEnd(); ++it) {
        addStringToBuffer(*it);
    }

    CraftingDialog::index_t numberOfCraftables = craftingDialog.getCraftablesSize();
    addUnsignedCharToBuffer(numberOfCraftables);

    for (auto it = craftingDialog.getCraftablesBegin(); it != craftingDialog.getCraftablesEnd(); ++it) {
        uint8_t craftableId = it->first;
        Craftable &craftable = *it->second;
        addUnsignedCharToBuffer(craftableId);
        addUnsignedCharToBuffer(craftable.getGroup());
        addShortIntToBuffer(craftable.getItem());
        addStringToBuffer(craftable.getName());
        addShortIntToBuffer(craftable.getDecisecondsToCraft());
        addUnsignedCharToBuffer(craftable.getCraftedStackSize());

        Craftable::index_t numberOfIngredients = craftable.getIngredientsSize();
        addUnsignedCharToBuffer(numberOfIngredients);

        for (auto it2 = craftable.getIngredientsBegin(); it2 != craftable.getIngredientsEnd(); ++it2) {
            Ingredient &ingredient = **it2;
            addShortIntToBuffer(ingredient.getItem());
            addUnsignedCharToBuffer(ingredient.getNumber());
        }
    }

    addIntToBuffer(dialogId);
}

CraftingDialogCraftTC::CraftingDialogCraftTC(uint8_t stillToCraft, uint16_t craftingTime, unsigned int dialogId) : BasicServerCommand(SC_CRAFTINGDIALOGUPDATE_TC) {
    addUnsignedCharToBuffer(0);
    addUnsignedCharToBuffer(stillToCraft);
    addShortIntToBuffer(craftingTime);
    addIntToBuffer(dialogId);
}

CraftingDialogCraftingCompleteTC::CraftingDialogCraftingCompleteTC(unsigned int dialogId) : BasicServerCommand(SC_CRAFTINGDIALOGUPDATE_TC) {
    addUnsignedCharToBuffer(1);
    addIntToBuffer(dialogId);
}

CraftingDialogCraftingAbortedTC::CraftingDialogCraftingAbortedTC(unsigned int dialogId) : BasicServerCommand(SC_CRAFTINGDIALOGUPDATE_TC) {
    addUnsignedCharToBuffer(2);
    addIntToBuffer(dialogId);
}

ItemUpdate_TC::ItemUpdate_TC(position fieldpos, ITEMVECTOR &items) : BasicServerCommand(SC_ITEMUPDATE_TC) {
    Logger::writeMessage("rot_update", "sending new itemstack for pos("+Logger::toString(fieldpos.x)+", "+Logger::toString(fieldpos.y)+", "+Logger::toString(fieldpos.z)+")",false);
    addShortIntToBuffer(fieldpos.x);
    addShortIntToBuffer(fieldpos.y);
    addShortIntToBuffer(fieldpos.z);
    int16_t size = static_cast<unsigned char>(items.size());

    if (size > 255) {
        size = 255;
    }

    addUnsignedCharToBuffer(static_cast<uint8_t>(size));

    for (auto it = items.begin(); it != items.end(); ++it) {
        //we added 255 items
        if (size <= 0) {
            break;
        }

        addShortIntToBuffer(it->getId());
        addShortIntToBuffer(it->getNumber());
        Logger::writeMessage("rot_update", "adding item id: "+Logger::toString(it->getId())+" count: "+Logger::toString(static_cast<int>(it->getNumber())),false);
        size--;
    }
}

ItemUpdate_TC::ItemUpdate_TC(int16_t px, int16_t py, int16_t pz, ITEMVECTOR &items) : BasicServerCommand(SC_ITEMUPDATE_TC) {
    addShortIntToBuffer(px);
    addShortIntToBuffer(py);
    addShortIntToBuffer(pz);
    int16_t size = static_cast<unsigned char>(items.size());

    if (size > 255) {
        size = 255;
    }

    addUnsignedCharToBuffer(static_cast<uint8_t>(size));

    for (auto it = items.begin(); it != items.end(); ++it) {
        //we added 255 items
        if (size <= 0) {
            break;
        }

        addShortIntToBuffer(it->getId());
        addShortIntToBuffer(it->getNumber());
        size--;
    }
}

CharDescription::CharDescription(TYPE_OF_CHARACTER_ID id, std::string description) : BasicServerCommand(SC_LOOKATCHARRESULT_TC) {
    addIntToBuffer(id);
    addStringToBuffer(description);
}

AppearanceTC::AppearanceTC(Character *cc) : BasicServerCommand(SC_APPEARANCE_TC) {
    addIntToBuffer(cc->id);
    addShortIntToBuffer(cc->race);
    addUnsignedCharToBuffer(cc->getAttribute(Character::sex));
    addShortIntToBuffer(cc->getAttribute(Character::hitpoints));
    addUnsignedCharToBuffer(RaceSizes->getRelativeSize(cc->race, cc->getAttribute(Character::height)));
    addUnsignedCharToBuffer(cc->hair);
    addUnsignedCharToBuffer(cc->beard);
    addUnsignedCharToBuffer(cc->hairred);
    addUnsignedCharToBuffer(cc->hairgreen);
    addUnsignedCharToBuffer(cc->hairblue);
    addUnsignedCharToBuffer(cc->skinred);
    addUnsignedCharToBuffer(cc->skingreen);
    addUnsignedCharToBuffer(cc->skinblue);

    for (unsigned char i = 0; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
        addShortIntToBuffer(cc->GetItemAt(i).getId());
    }

    addUnsignedCharToBuffer(cc->getWeaponMode());
    uint8_t deathflag = cc->IsAlive() ? 0 : 1;
    addUnsignedCharToBuffer(deathflag);
}

AnimationTC::AnimationTC(TYPE_OF_CHARACTER_ID id, uint8_t animID) : BasicServerCommand(SC_ANIMATION_TC) {
    addIntToBuffer(id);
    addUnsignedCharToBuffer(animID);
}

BookTC::BookTC(uint16_t bookID) : BasicServerCommand(SC_BOOK_TC) {
    addShortIntToBuffer(bookID);
}

RemoveCharTC::RemoveCharTC(TYPE_OF_CHARACTER_ID id) : BasicServerCommand(SC_REMOVECHAR_TC) {
    addIntToBuffer(id);
}

UpdateTimeTC::UpdateTimeTC(unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, short int year) : BasicServerCommand(SC_UPDATETIME_TC) {
    addUnsignedCharToBuffer(hour);
    addUnsignedCharToBuffer(minute);
    addUnsignedCharToBuffer(day);
    addUnsignedCharToBuffer(month);
    addShortIntToBuffer(year);
}

LogOutTC::LogOutTC(unsigned char reason) : BasicServerCommand(SC_LOGOUT_TC) {
    addUnsignedCharToBuffer(reason);
}

TargetLostTC::TargetLostTC() : BasicServerCommand(SC_TARGETLOST_TC) {
}

AttackAcknowledgedTC::AttackAcknowledgedTC() : BasicServerCommand(SC_ATTACKACKNOWLEDGED_TC) {
}

void addItemLookAt(BasicServerCommand *cmd, const ItemLookAt &lookAt) {
    cmd->addStringToBuffer(lookAt.getName());
    cmd->addUnsignedCharToBuffer((uint8_t)lookAt.getRareness());
    cmd->addStringToBuffer(lookAt.getDescription());
    cmd->addStringToBuffer(lookAt.getCraftedBy());
    cmd->addShortIntToBuffer(lookAt.getWeight());
    cmd->addIntToBuffer(lookAt.getWorth());
    cmd->addStringToBuffer(lookAt.getQualityText());
    cmd->addStringToBuffer(lookAt.getDurabilityText());
    cmd->addUnsignedCharToBuffer(lookAt.getDurabilityValue());
    cmd->addUnsignedCharToBuffer(lookAt.getDiamondLevel());
    cmd->addUnsignedCharToBuffer(lookAt.getEmeraldLevel());
    cmd->addUnsignedCharToBuffer(lookAt.getRubyLevel());
    cmd->addUnsignedCharToBuffer(lookAt.getSapphireLevel());
    cmd->addUnsignedCharToBuffer(lookAt.getAmethystLevel());
    cmd->addUnsignedCharToBuffer(lookAt.getObsidianLevel());
    cmd->addUnsignedCharToBuffer(lookAt.getTopazLevel());
    cmd->addUnsignedCharToBuffer(lookAt.getBonus());
}

LookAtInventoryItemTC::LookAtInventoryItemTC(unsigned char pos, const ItemLookAt &lookAt) : BasicServerCommand(SC_LOOKATINVENTORYITEM_TC) {
    addUnsignedCharToBuffer(pos);
    addItemLookAt(this, lookAt);
}

LookAtShowCaseItemTC::LookAtShowCaseItemTC(unsigned char showcase, unsigned char pos, const ItemLookAt &lookAt) : BasicServerCommand(SC_LOOKATSHOWCASEITEM_TC) {
    addUnsignedCharToBuffer(showcase);
    addUnsignedCharToBuffer(pos);
    addItemLookAt(this, lookAt);
}

LookAtMapItemTC::LookAtMapItemTC(short int x, short int y, short int z, const ItemLookAt &lookAt) : BasicServerCommand(SC_LOOKATMAPITEM_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addItemLookAt(this, lookAt);
}

LookAtDialogItemTC::LookAtDialogItemTC(unsigned int dialogId, uint8_t itemIndex, const ItemLookAt &lookAt) : BasicServerCommand(SC_LOOKATDIALOGITEM_TC) {
    addIntToBuffer(dialogId);
    addUnsignedCharToBuffer(0);
    addUnsignedCharToBuffer(itemIndex);
    addItemLookAt(this, lookAt);
}

LookAtCraftingDialogIngredientTC::LookAtCraftingDialogIngredientTC(unsigned int dialogId, uint8_t itemIndex, uint8_t ingredientIndex, const ItemLookAt &lookAt) : BasicServerCommand(SC_LOOKATDIALOGITEM_TC) {
    addIntToBuffer(dialogId);
    addUnsignedCharToBuffer(1);
    addUnsignedCharToBuffer(itemIndex);
    addUnsignedCharToBuffer(ingredientIndex);
    addItemLookAt(this, lookAt);
}

LookAtTileTC::LookAtTileTC(short int x, short int y, short int z, const std::string &lookAt) : BasicServerCommand(SC_LOOKATTILE_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addStringToBuffer(lookAt);
}

ItemPutTC::ItemPutTC(short int x, short int y, short int z, Item &item) : BasicServerCommand(SC_ITEMPUT_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addShortIntToBuffer(item.getId());

    if (item.isContainer()) {
        addShortIntToBuffer(1);
    } else {
        addShortIntToBuffer(item.getNumber());
    }
}

ItemSwapTC::ItemSwapTC(position pos, unsigned short int id, Item &item) : BasicServerCommand(SC_MAPITEMSWAP) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addShortIntToBuffer(id);
    addShortIntToBuffer(item.getId());

    if (item.isContainer()) {
        addShortIntToBuffer(1);
    } else {
        addShortIntToBuffer(item.getNumber());
    }
}

ItemSwapTC::ItemSwapTC(short int x, short int y, short int z, unsigned short int id, Item &item) : BasicServerCommand(SC_MAPITEMSWAP) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addShortIntToBuffer(id);
    addShortIntToBuffer(item.getId());

    if (item.isContainer()) {
        addUnsignedCharToBuffer(1);
    } else {
        addShortIntToBuffer(item.getNumber());
    }
}

ItemRemoveTC::ItemRemoveTC(short int x, short int y, short int z) : BasicServerCommand(SC_ITEMREMOVE_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
}

AdminViewPlayersTC::AdminViewPlayersTC() : BasicServerCommand(SC_ADMINVIEWPLAYERS_TC) {
    World::PLAYERVECTOR::iterator titerator;
    unsigned short int count = World::get()->Players.size();
    addShortIntToBuffer(count);

    for (titerator = World::get()->Players.begin(); titerator < World::get()->Players.end(); ++titerator) {
        addStringToBuffer((*titerator)->name);
        addStringToBuffer((*titerator)->last_ip);
    }
}

SoundTC::SoundTC(short int x, short int y, short int z, unsigned short int id) : BasicServerCommand(SC_SOUND_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addShortIntToBuffer(id);
}

GraphicEffectTC::GraphicEffectTC(short int x, short int y, short int z, unsigned short int id) : BasicServerCommand(SC_GRAPHICEFFECT_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addShortIntToBuffer(id);
}

UpdateShowCaseTC::UpdateShowCaseTC(unsigned char showcase, const TYPE_OF_CONTAINERSLOTS volume, const Container::ITEMMAP &items) : BasicServerCommand(SC_UPDATESHOWCASE_TC) {
    addUnsignedCharToBuffer(showcase);

    addShortIntToBuffer(volume);

    TYPE_OF_CONTAINERSLOTS size = items.size();
    addShortIntToBuffer(size);

    for (auto it = items.begin(); it != items.end(); ++it) {
        const Item &item = it->second;
        addShortIntToBuffer(it->first);
        addShortIntToBuffer(item.getId());

        if (item.isContainer()) {
            addShortIntToBuffer(1);
        } else {
            addShortIntToBuffer(item.getNumber());
        }
    }
}

MapStripeTC::MapStripeTC(position pos, NewClientView::stripedirection dir) : BasicServerCommand(SC_MAPSTRIPE_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addUnsignedCharToBuffer(static_cast<unsigned char>(dir));
    Field **fields =  World::get()->clientview.mapStripe;
    uint8_t numberOfTiles = World::get()->clientview.getMaxTiles();
    addUnsignedCharToBuffer(numberOfTiles);

    for (int i = 0; i < numberOfTiles; ++i) {
        if (fields[i]) {
            addShortIntToBuffer(fields[i]->getTileCode());
            addShortIntToBuffer(fields[i]->getMusicId());
            addUnsignedCharToBuffer(static_cast<unsigned char>(fields[i]->items.size()));

            for (auto it = fields[i]->items.begin(); it < fields[i]->items.end(); ++it) {
                Item &item = *it;
                addShortIntToBuffer(item.getId());

                if (item.isContainer()) {
                    addShortIntToBuffer(1);
                } else {
                    addShortIntToBuffer(item.getNumber());
                }
            }
        } else {
            addShortIntToBuffer(-1);
            addShortIntToBuffer(0);
            addUnsignedCharToBuffer(0);
        }
    }
}

MapCompleteTC::MapCompleteTC() : BasicServerCommand(SC_MAPCOMPLETE_TC) {
}

MoveAckTC::MoveAckTC(TYPE_OF_CHARACTER_ID id, position pos, unsigned char mode, unsigned char waitpages) : BasicServerCommand(SC_MOVEACK_TC) {
    addIntToBuffer(id);
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addUnsignedCharToBuffer(mode);
    addUnsignedCharToBuffer(waitpages);
}

IntroduceTC::IntroduceTC(TYPE_OF_CHARACTER_ID id, std::string name) : BasicServerCommand(SC_INTRODUCE_TC) {
    addIntToBuffer(id);
    addStringToBuffer(name);
}

ShoutTC::ShoutTC(int16_t x, int16_t y, int16_t z, std::string text) : BasicServerCommand(SC_SHOUT_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addStringToBuffer(text);
}

WhisperTC::WhisperTC(int16_t x, int16_t y, int16_t z, std::string text) : BasicServerCommand(SC_WHISPER_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addStringToBuffer(text);
}

SayTC::SayTC(int16_t x, int16_t y, int16_t z, std::string text) : BasicServerCommand(SC_SAY_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addStringToBuffer(text);
}

InformTC::InformTC(Character::informType type, std::string text) : BasicServerCommand(SC_INFORM_TC) {
    addUnsignedCharToBuffer(type);
    addStringToBuffer(text);
}

MusicTC::MusicTC(short int title) : BasicServerCommand(SC_MUSIC_TC) {
    addShortIntToBuffer(title);
}

MusicDefaultTC::MusicDefaultTC() : BasicServerCommand(SC_MUSICDEFAULT_TC) {
}

UpdateAttribTC::UpdateAttribTC(TYPE_OF_CHARACTER_ID id, std::string name, short int value) : BasicServerCommand(SC_UPDATEATTRIB_TC) {
    addIntToBuffer(id);
    addStringToBuffer(name);
    addShortIntToBuffer(value);
}

UpdateMagicFlagsTC::UpdateMagicFlagsTC(unsigned char type, uint32_t flags) : BasicServerCommand(SC_UPDATEMAGICFLAGS_TC) {
    addUnsignedCharToBuffer(type);
    addIntToBuffer(flags);
}

ClearShowCaseTC::ClearShowCaseTC(unsigned char id) : BasicServerCommand(SC_CLEARSHOWCASE_TC) {
    addUnsignedCharToBuffer(id);
}

UpdateSkillTC::UpdateSkillTC(TYPE_OF_SKILL_ID skill, unsigned short int major, unsigned short int minor) : BasicServerCommand(SC_UPDATESKILL_TC) {
    addUnsignedCharToBuffer(skill);
    addShortIntToBuffer(major);
    addShortIntToBuffer(minor);
}

UpdateWeatherTC::UpdateWeatherTC(WeatherStruct weather) : BasicServerCommand(SC_UPDATEWEATHER_TC) {
    addUnsignedCharToBuffer(weather.cloud_density);
    addUnsignedCharToBuffer(weather.fog_density);
    addUnsignedCharToBuffer(weather.wind_dir);
    addUnsignedCharToBuffer(weather.gust_strength);
    addUnsignedCharToBuffer(weather.percipitation_strength);
    addUnsignedCharToBuffer(static_cast<unsigned char>(weather.per_type));
    addUnsignedCharToBuffer(weather.thunderstorm);
    addUnsignedCharToBuffer(weather.temperature);
}

UpdateWeatherTC::UpdateWeatherTC(uint8_t cd, uint8_t fd, uint8_t wd, uint8_t gs, uint8_t ps, uint8_t pt, uint8_t ts, uint8_t tp) : BasicServerCommand(SC_UPDATEWEATHER_TC) {
    addUnsignedCharToBuffer(cd);
    addUnsignedCharToBuffer(fd);
    addUnsignedCharToBuffer(wd);
    addUnsignedCharToBuffer(gs);
    addUnsignedCharToBuffer(ps);
    addUnsignedCharToBuffer(pt);
    addUnsignedCharToBuffer(ts);
    addUnsignedCharToBuffer(tp);
}

IdTC::IdTC(int id) : BasicServerCommand(SC_ID_TC) {
    addIntToBuffer(id);
}

UpdateInventoryPosTC::UpdateInventoryPosTC(unsigned char pos, TYPE_OF_ITEM_ID id, Item::number_type number) : BasicServerCommand(SC_UPDATEINVENTORYPOS_TC) {
    addUnsignedCharToBuffer(pos);
    addShortIntToBuffer(id);
    addShortIntToBuffer(number);
}

SetCoordinateTC::SetCoordinateTC(position pos) : BasicServerCommand(SC_SETCOORDINATE_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
}

SetCoordinateTC::SetCoordinateTC(short int x, short int y, short int z) : BasicServerCommand(SC_SETCOORDINATE_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
}

PlayerSpinTC::PlayerSpinTC(unsigned char faceto, TYPE_OF_CHARACTER_ID id) : BasicServerCommand(SC_PLAYERSPIN_TC) {
    addUnsignedCharToBuffer(faceto);
    addIntToBuffer(id);
}

