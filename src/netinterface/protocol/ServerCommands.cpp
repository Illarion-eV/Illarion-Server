//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#include "ServerCommands.hpp"

#include "globals.hpp"
#include "Item.hpp"
#include "Player.hpp"
#include "Monster.hpp"
#include "Field.hpp"
#include "World.hpp"
#include "Logger.hpp"

#include "data/Data.hpp"

#include "netinterface/BasicServerCommand.hpp"
#include "netinterface/NetInterface.hpp"

#include "dialog/InputDialog.hpp"
#include "dialog/MessageDialog.hpp"
#include "dialog/MerchantDialog.hpp"
#include "dialog/SelectionDialog.hpp"
#include "dialog/CraftingDialog.hpp"

KeepAliveTC::KeepAliveTC() : BasicServerCommand(SC_KEEPALIVE_TC) {
}

QuestProgressTC::QuestProgressTC(TYPE_OF_QUEST_ID id,
                                 const std::string &title,
                                 const std::string &description,
                                 const std::vector<position> &targets,
                                 bool final) : BasicServerCommand(SC_QUESTPROGRESS_TC) {
    addShortIntToBuffer(id);
    addStringToBuffer(title);
    addStringToBuffer(description);
    addUnsignedCharToBuffer(final ? 1 : 0);

    if (targets.size() > 255) {
        addUnsignedCharToBuffer(255);
    } else {
        addUnsignedCharToBuffer(targets.size());
    }

    for (size_t i = 0; (i < targets.size()) && (i < 255); ++i) {
        const position &pos = targets[i];
        addShortIntToBuffer(pos.x);
        addShortIntToBuffer(pos.y);
        addShortIntToBuffer(pos.z);
    }
}

AbortQuestTC::AbortQuestTC(TYPE_OF_QUEST_ID id) : BasicServerCommand(SC_ABORTQUEST_TC) {
    addShortIntToBuffer(id);
}

AvailableQuestsTC::AvailableQuestsTC(const std::vector<position> &availableNow,
                                     const std::vector<position> &availableSoon) : BasicServerCommand(SC_AVAILABLEQUESTS_TC) {
    addShortIntToBuffer(availableNow.size());
    
    for (const auto &pos : availableNow) {
        addShortIntToBuffer(pos.x);
        addShortIntToBuffer(pos.y);
        addShortIntToBuffer(pos.z);
    }

    addShortIntToBuffer(availableSoon.size());

    for (const auto &pos : availableSoon) {
        addShortIntToBuffer(pos.x);
        addShortIntToBuffer(pos.y);
        addShortIntToBuffer(pos.z);
    }
}

InputDialogTC::InputDialogTC(const InputDialog &inputDialog, unsigned int dialogId) : BasicServerCommand(SC_INPUTDIALOG_TC) {
    addStringToBuffer(inputDialog.getTitle());
    addStringToBuffer(inputDialog.getDescription());
    addUnsignedCharToBuffer(inputDialog.isMultiline() ? 1 : 0);
    addShortIntToBuffer(inputDialog.getMaxChars());
    addIntToBuffer(dialogId);
}

MessageDialogTC::MessageDialogTC(const MessageDialog &messageDialog, unsigned int dialogId) : BasicServerCommand(SC_MESSAGEDIALOG_TC) {
    addStringToBuffer(messageDialog.getTitle());
    addStringToBuffer(messageDialog.getText());
    addIntToBuffer(dialogId);
}

MerchantDialogTC::MerchantDialogTC(const MerchantDialog &merchantDialog, unsigned int dialogId) : BasicServerCommand(SC_MERCHANTDIALOG_TC) {
    addStringToBuffer(merchantDialog.getTitle());
    MerchantDialog::index_type size = merchantDialog.getOffersSize();
    addUnsignedCharToBuffer(size);

    for (auto it = merchantDialog.getOffersBegin(); it != merchantDialog.getOffersEnd(); ++it) {
        const OfferProduct &offer = *it;
        addShortIntToBuffer(offer.getItem());
        addStringToBuffer(offer.getName());
        addIntToBuffer(offer.getPrice());
        addShortIntToBuffer(offer.getStack());
    }

    size = merchantDialog.getPrimaryRequestsSize();
    addUnsignedCharToBuffer(size);

    for (auto it = merchantDialog.getPrimaryRequestsBegin(); it != merchantDialog.getPrimaryRequestsEnd(); ++it) {
        addShortIntToBuffer(it->getItem());
        addStringToBuffer(it->getName());
        addIntToBuffer(it->getPrice());
    }

    size = merchantDialog.getSecondaryRequestsSize();
    addUnsignedCharToBuffer(size);

    for (auto it = merchantDialog.getSecondaryRequestsBegin(); it != merchantDialog.getSecondaryRequestsEnd(); ++it) {
        addShortIntToBuffer(it->getItem());
        addStringToBuffer(it->getName());
        addIntToBuffer(it->getPrice());
    }

    addIntToBuffer(dialogId);
}

SelectionDialogTC::SelectionDialogTC(const SelectionDialog &selectionDialog, unsigned int dialogId) : BasicServerCommand(SC_SELECTIONDIALOG_TC) {
    addStringToBuffer(selectionDialog.getTitle());
    addStringToBuffer(selectionDialog.getText());
    SelectionDialog::index_type size = selectionDialog.getOptionsSize();
    addUnsignedCharToBuffer(size);

    for (const auto &option : selectionDialog) {
        addShortIntToBuffer(option.getItem());
        addStringToBuffer(option.getName());
    }

    addIntToBuffer(dialogId);
}

CraftingDialogTC::CraftingDialogTC(const CraftingDialog &craftingDialog, unsigned int dialogId) : BasicServerCommand(SC_CRAFTINGDIALOG_TC) {
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
        const Craftable &craftable = it->second;
        addUnsignedCharToBuffer(craftableId);
        addUnsignedCharToBuffer(craftable.getGroup());
        addShortIntToBuffer(craftable.getItem());
        addStringToBuffer(craftable.getName());
        addShortIntToBuffer(craftable.getDecisecondsToCraft());
        addUnsignedCharToBuffer(craftable.getCraftedStackSize());

        Craftable::index_t numberOfIngredients = craftable.getIngredientsSize();
        addUnsignedCharToBuffer(numberOfIngredients);

        for (const auto &ingredient : craftable) {
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

CloseDialogTC::CloseDialogTC(unsigned int dialogId) : BasicServerCommand(SC_CLOSEDIALOG_TC) {
    addIntToBuffer(dialogId);
}

void addMovementCostToBuffer(BasicServerCommand *cmd, const position &pos) {
    auto field = World::get()->GetField(pos);

    if (field) {
        cmd->addUnsignedCharToBuffer(field->getMovementCost());
    } else {
        cmd->addUnsignedCharToBuffer(255);
    }
}

ItemUpdate_TC::ItemUpdate_TC(const position &pos, const ITEMVECTOR &items) : BasicServerCommand(SC_ITEMUPDATE_TC) {
    Logger::debug(LogFacility::World) << "sending new itemstack for pos " << pos << Log::end;
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    int16_t size = static_cast<unsigned char>(items.size());

    if (size > 255) {
        size = 255;
    }

    addUnsignedCharToBuffer(static_cast<uint8_t>(size));

    for (const auto &item : items) {
        //we added 255 items
        if (size <= 0) {
            break;
        }

        addShortIntToBuffer(item.getId());
        addShortIntToBuffer(item.getNumber());
        Logger::debug(LogFacility::World) << "adding item id: " << item.getId() << " count: " << static_cast<int>(item.getNumber()) << Log::end;
        size--;
    }

    addMovementCostToBuffer(this, pos);
}

CharDescription::CharDescription(TYPE_OF_CHARACTER_ID id, const std::string &description) : BasicServerCommand(SC_LOOKATCHARRESULT_TC) {
    addIntToBuffer(id);
    addStringToBuffer(description);
}

AppearanceTC::AppearanceTC(Character *cc, const Player *receivingPlayer) : BasicServerCommand(SC_APPEARANCE_TC) {
    addIntToBuffer(cc->getId());

    if (cc->getType() == Character::player) {
        Player *player = dynamic_cast<Player *>(cc);

        if (receivingPlayer->knows(player)) {
            addStringToBuffer(player->getName());
        } else {
            addStringToBuffer("");
        }

        addStringToBuffer(receivingPlayer->getCustomNameOf(player));
    } else if (cc->getType() == Character::monster) {
        Monster *monster = dynamic_cast<Monster *>(cc);
        addStringToBuffer(receivingPlayer->nls(monster->nameDe, monster->getName()));
        addStringToBuffer("");
    } else {
        addStringToBuffer(cc->getName());
        addStringToBuffer("");
    }

    addShortIntToBuffer(cc->getRace());
    addUnsignedCharToBuffer(cc->getAttribute(Character::sex));
    addShortIntToBuffer(cc->getAttribute(Character::hitpoints));
    addUnsignedCharToBuffer(Data::RaceAttributes.getRelativeSize(cc->getRace(), cc->getAttribute(Character::height)));
    const Character::appearance appearance = cc->getAppearance();
    addUnsignedCharToBuffer(appearance.hairtype);
    addUnsignedCharToBuffer(appearance.beardtype);
    addUnsignedCharToBuffer(appearance.hair.red);
    addUnsignedCharToBuffer(appearance.hair.green);
    addUnsignedCharToBuffer(appearance.hair.blue);
    addUnsignedCharToBuffer(appearance.skin.red);
    addUnsignedCharToBuffer(appearance.skin.green);
    addUnsignedCharToBuffer(appearance.skin.blue);

    for (unsigned char i = 0; i < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
        addShortIntToBuffer(cc->GetItemAt(i).getId());
    }

    uint8_t deathflag = cc->isAlive() ? 0 : 1;
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
    cmd->addStringToBuffer(lookAt.getType());
    cmd->addUnsignedCharToBuffer(lookAt.getLevel());
    cmd->addUnsignedCharToBuffer(lookAt.isUsable());
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

LookAtMapItemTC::LookAtMapItemTC(const position &pos, const ItemLookAt &lookAt) : BasicServerCommand(SC_LOOKATMAPITEM_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
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

LookAtTileTC::LookAtTileTC(const position &pos, const std::string &lookAt) : BasicServerCommand(SC_LOOKATTILE_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addStringToBuffer(lookAt);
}

ItemPutTC::ItemPutTC(const position &pos, const Item &item) : BasicServerCommand(SC_ITEMPUT_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addShortIntToBuffer(item.getId());

    if (item.isContainer()) {
        addShortIntToBuffer(1);
    } else {
        addShortIntToBuffer(item.getNumber());
    }

    addMovementCostToBuffer(this, pos);
}

ItemSwapTC::ItemSwapTC(const position &pos, unsigned short int id, const Item &item) : BasicServerCommand(SC_MAPITEMSWAP) {
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

    addMovementCostToBuffer(this, pos);
}

ItemRemoveTC::ItemRemoveTC(const position &pos) : BasicServerCommand(SC_ITEMREMOVE_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addMovementCostToBuffer(this, pos);
}

AdminViewPlayersTC::AdminViewPlayersTC() : BasicServerCommand(SC_ADMINVIEWPLAYERS_TC) {
    unsigned short int count = World::get()->Players.size();
    addShortIntToBuffer(count);

    World::get()->Players.for_each([this](Player *p) {
        addStringToBuffer(p->getName());
        addStringToBuffer(p->last_ip);
    });
}

SoundTC::SoundTC(const position &pos, unsigned short int id) : BasicServerCommand(SC_SOUND_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addShortIntToBuffer(id);
}

GraphicEffectTC::GraphicEffectTC(const position &pos, unsigned short int id) : BasicServerCommand(SC_GRAPHICEFFECT_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addShortIntToBuffer(id);
}

UpdateShowcaseTC::UpdateShowcaseTC(unsigned char showcase, const ItemLookAt &lookAt,
        TYPE_OF_CONTAINERSLOTS volume, const Container::ITEMMAP &items) : BasicServerCommand(SC_UPDATESHOWCASE_TC) {
    addUnsignedCharToBuffer(showcase);
    addStringToBuffer(lookAt.getName());
    addStringToBuffer(lookAt.getDescription());
    addShortIntToBuffer(volume);

    TYPE_OF_CONTAINERSLOTS size = items.size();
    addShortIntToBuffer(size);

    for (const auto &it : items) {
        const Item &item = it.second;
        addShortIntToBuffer(it.first);
        addShortIntToBuffer(item.getId());

        if (item.isContainer()) {
            addShortIntToBuffer(1);
        } else {
            addShortIntToBuffer(item.getNumber());
        }
    }
}

UpdateShowcaseSlotTC::UpdateShowcaseSlotTC(unsigned char showcase, TYPE_OF_CONTAINERSLOTS slot) : BasicServerCommand(SC_UPDATESHOWCASESLOT_TC) {
    addUnsignedCharToBuffer(showcase);
    addShortIntToBuffer(slot);
    addShortIntToBuffer(0);
    addShortIntToBuffer(0);
}

UpdateShowcaseSlotTC::UpdateShowcaseSlotTC(unsigned char showcase, TYPE_OF_CONTAINERSLOTS slot, const Item &item) : BasicServerCommand(SC_UPDATESHOWCASESLOT_TC) {
    addUnsignedCharToBuffer(showcase);
    addShortIntToBuffer(slot);
    addShortIntToBuffer(item.getId());
    
    if (item.isContainer()) {
        addShortIntToBuffer(1);
    } else {
        addShortIntToBuffer(item.getNumber());
    }
}

MapStripeTC::MapStripeTC(const position &pos, NewClientView::stripedirection dir) : BasicServerCommand(SC_MAPSTRIPE_TC) {
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
            addUnsignedCharToBuffer(fields[i]->getMovementCost());
            addShortIntToBuffer(fields[i]->getMusicId());
            addUnsignedCharToBuffer(static_cast<unsigned char>(fields[i]->items.size()));

            for (const auto &item : fields[i]->items) {
                addShortIntToBuffer(item.getId());

                if (item.isContainer()) {
                    addShortIntToBuffer(1);
                } else {
                    addShortIntToBuffer(item.getNumber());
                }
            }
        } else {
            addShortIntToBuffer(-1);
            addUnsignedCharToBuffer(0);
            addShortIntToBuffer(0);
            addUnsignedCharToBuffer(0);
        }
    }
}

MapCompleteTC::MapCompleteTC() : BasicServerCommand(SC_MAPCOMPLETE_TC) {
}

MoveAckTC::MoveAckTC(TYPE_OF_CHARACTER_ID id, const position &pos, unsigned char mode, TYPE_OF_WALKINGCOST duration) : BasicServerCommand(SC_MOVEACK_TC) {
    addIntToBuffer(id);
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addUnsignedCharToBuffer(mode);
    //round to hundreds for now, to mirror action points
    addShortIntToBuffer((duration/100)*100);
}

IntroduceTC::IntroduceTC(TYPE_OF_CHARACTER_ID id, const std::string &name) : BasicServerCommand(SC_INTRODUCE_TC) {
    addIntToBuffer(id);
    addStringToBuffer(name);
}

ShoutTC::ShoutTC(const position &pos, const std::string &text) : BasicServerCommand(SC_SHOUT_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addStringToBuffer(text);
}

WhisperTC::WhisperTC(const position &pos, const std::string &text) : BasicServerCommand(SC_WHISPER_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addStringToBuffer(text);
}

SayTC::SayTC(const position &pos, const std::string &text) : BasicServerCommand(SC_SAY_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addStringToBuffer(text);
}

InformTC::InformTC(Character::informType type, const std::string &text) : BasicServerCommand(SC_INFORM_TC) {
    addUnsignedCharToBuffer(type);
    addStringToBuffer(text);
}

MusicTC::MusicTC(short int title) : BasicServerCommand(SC_MUSIC_TC) {
    addShortIntToBuffer(title);
}

MusicDefaultTC::MusicDefaultTC() : BasicServerCommand(SC_MUSICDEFAULT_TC) {
}

UpdateAttribTC::UpdateAttribTC(TYPE_OF_CHARACTER_ID id, const std::string &name, unsigned short int value) : BasicServerCommand(SC_UPDATEATTRIB_TC) {
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

UpdateWeatherTC::UpdateWeatherTC(const WeatherStruct &weather) : BasicServerCommand(SC_UPDATEWEATHER_TC) {
    addUnsignedCharToBuffer(weather.cloud_density);
    addUnsignedCharToBuffer(weather.fog_density);
    addUnsignedCharToBuffer(weather.wind_dir);
    addUnsignedCharToBuffer(weather.gust_strength);
    addUnsignedCharToBuffer(weather.percipitation_strength);
    addUnsignedCharToBuffer(static_cast<unsigned char>(weather.per_type));
    addUnsignedCharToBuffer(weather.thunderstorm);
    addUnsignedCharToBuffer(weather.temperature);
}

IdTC::IdTC(int id) : BasicServerCommand(SC_ID_TC) {
    addIntToBuffer(id);
}

UpdateInventoryPosTC::UpdateInventoryPosTC(unsigned char pos, TYPE_OF_ITEM_ID id, Item::number_type number) : BasicServerCommand(SC_UPDATEINVENTORYPOS_TC) {
    addUnsignedCharToBuffer(pos);
    addShortIntToBuffer(id);
    addShortIntToBuffer(number);
}

SetCoordinateTC::SetCoordinateTC(const position &pos) : BasicServerCommand(SC_SETCOORDINATE_TC) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
}

PlayerSpinTC::PlayerSpinTC(unsigned char faceto, TYPE_OF_CHARACTER_ID id) : BasicServerCommand(SC_PLAYERSPIN_TC) {
    addUnsignedCharToBuffer(faceto);
    addIntToBuffer(id);
}

