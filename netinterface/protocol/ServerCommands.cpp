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

extern RaceSizeTable *RaceSizes;

InputDialogTC::InputDialogTC(InputDialog &inputDialog, unsigned int dialogId) : BasicServerCommand(SC_INPUTDIALOG_TC) {
    addStringToBuffer(inputDialog.getTitle());
    addUnsignedCharToBuffer(inputDialog.isMultiline() ? 1 : 0);
    addShortIntToBuffer(inputDialog.getMaxChars());
    addIntToBuffer(dialogId);
}

MessageDialogTC::MessageDialogTC(MessageDialog &messageDialog, unsigned int dialogId) : BasicServerCommand(SC_MESSAGEDIALOG_TC) {
    addStringToBuffer(messageDialog.getTitle());
    addStringToBuffer(messageDialog.getText());
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
        addUnsignedCharToBuffer(it->getNumber());
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
        addUnsignedCharToBuffer(it->getNumber());
        size--;
    }
}

CharDescription::CharDescription(TYPE_OF_CHARACTER_ID id, std::string description) : BasicServerCommand(SC_LOOKATCHARRESULT_TC) {
    addIntToBuffer(id);
    addStringToBuffer(description);
}

AppearanceTC::AppearanceTC(Character *cc) : BasicServerCommand(SC_APPEARANCE_TC) {
    addIntToBuffer(cc->id);
    addShortIntToBuffer(cc->appearance);
    addUnsignedCharToBuffer(RaceSizes->getRelativeSize(cc->race, cc->battrib.body_height));
    addUnsignedCharToBuffer(static_cast<unsigned char>(0));
    addUnsignedCharToBuffer(cc->hair);
    addUnsignedCharToBuffer(cc->beard);
    addUnsignedCharToBuffer(cc->hairred);
    addUnsignedCharToBuffer(cc->hairgreen);
    addUnsignedCharToBuffer(cc->hairblue);
    addUnsignedCharToBuffer(cc->skinred);
    addUnsignedCharToBuffer(cc->skingreen);
    addUnsignedCharToBuffer(cc->skinblue);
    addShortIntToBuffer(cc->GetItemAt(1).getId());
    addShortIntToBuffer(cc->GetItemAt(3).getId());
    addShortIntToBuffer(cc->GetItemAt(11).getId());
    addShortIntToBuffer(cc->GetItemAt(5).getId());
    addShortIntToBuffer(cc->GetItemAt(6).getId());
    addShortIntToBuffer(cc->GetItemAt(9).getId());
    addShortIntToBuffer(cc->GetItemAt(10).getId());
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

NameOfInventoryItemTC::NameOfInventoryItemTC(unsigned char pos, std::string name) : BasicServerCommand(SC_NAMEOFINVENTORYITEM_TC) {
    addUnsignedCharToBuffer(pos);
    addStringToBuffer(name);
}

NameOfShowCaseItemTC::NameOfShowCaseItemTC(unsigned char showcase, unsigned char pos, std::string name) : BasicServerCommand(SC_NAMEOFSHOWCASEITEM_TC) {
    addUnsignedCharToBuffer(showcase);
    addUnsignedCharToBuffer(pos);
    addStringToBuffer(name);
}

NameOfMapItemTC::NameOfMapItemTC(short int x, short int y, short int z, std::string name) : BasicServerCommand(SC_NAMEOFMAPITEM_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addStringToBuffer(name);
}

ItemPutTC::ItemPutTC(short int x, short int y, short int z, Item &item) : BasicServerCommand(SC_ITEMPUT_TC) {
    addShortIntToBuffer(x);
    addShortIntToBuffer(y);
    addShortIntToBuffer(z);
    addShortIntToBuffer(item.getId());

    if (item.isContainer()) {
        addUnsignedCharToBuffer(1);
    } else {
        addUnsignedCharToBuffer(item.getNumber());
    }
}

ItemSwapTC::ItemSwapTC(position pos, unsigned short int id, Item &item) : BasicServerCommand(SC_MAPITEMSWAP) {
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
    addShortIntToBuffer(id);
    addShortIntToBuffer(item.getId());

    if (item.isContainer()) {
        addUnsignedCharToBuffer(1);
    } else {
        addUnsignedCharToBuffer(item.getNumber());
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
        addUnsignedCharToBuffer(item.getNumber());
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

StartPlayerMenuTC::StartPlayerMenuTC(UserMenuStruct menu) : BasicServerCommand(SC_STARTPLAYERMENU_TC) {
    int count = menu.Items.size();

    addUnsignedCharToBuffer(count);
    std::list<TYPE_OF_ITEM_ID>::iterator it;

    for (int i = 0; i < count; ++i) {
        addShortIntToBuffer(menu.Items.front());
        menu.Items.pop_front();
    }

    menu.Items.clear();
}

UpdateShowCaseTC::UpdateShowCaseTC(unsigned char showcase, ITEMVECTOR &items) : BasicServerCommand(SC_UPDATESHOWCASE_TC) {
    addUnsignedCharToBuffer(showcase);

    MAXCOUNTTYPE size = items.size();
    addUnsignedCharToBuffer(size);

    for (auto it = items.begin(); it < items.end(); ++it) {
        Item &item = *it;
        addShortIntToBuffer(item.getId());

        if (item.isContainer()) {
            addUnsignedCharToBuffer(1);
        } else {
            addUnsignedCharToBuffer(item.getNumber());
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
                    addUnsignedCharToBuffer(1);
                } else {
                    addUnsignedCharToBuffer(static_cast<unsigned char>(item.getNumber()));
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

MusicTC::MusicTC(short int title) : BasicServerCommand(SC_MUSIC_TC) {
    addShortIntToBuffer(title);
}

MusicDefaultTC::MusicDefaultTC() : BasicServerCommand(SC_MUSICDEFAULT_TC) {
}

UpdateAttribTC::UpdateAttribTC(std::string name, short int value) : BasicServerCommand(SC_UPDATEATTRIB_TC) {
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

UpdateSkillTC::UpdateSkillTC(std::string name, unsigned char type, unsigned short int major, unsigned short int minor) : BasicServerCommand(SC_UPDATESKILL_TC) {
    addStringToBuffer(name);
    addUnsignedCharToBuffer(type);
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

UpdateInventoryPosTC::UpdateInventoryPosTC(unsigned char pos, TYPE_OF_ITEM_ID id, unsigned char number) : BasicServerCommand(SC_UPDATEINVENTORYPOS_TC) {
    addUnsignedCharToBuffer(pos);
    addShortIntToBuffer(id);
    addUnsignedCharToBuffer(number);
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

