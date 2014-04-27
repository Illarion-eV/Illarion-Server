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


#include "World.hpp"
#include "data/NamesObjectTable.hpp"
#include "data/TilesTable.hpp"
#include "script/LuaItemScript.hpp"
#include "script/LuaLookAtItemScript.hpp"
#include "TableStructs.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "Monster.hpp"
#include "Field.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "Logger.hpp"
#include "data/Data.hpp"

extern std::shared_ptr<LuaLookAtItemScript>lookAtItemScript;

void World::sendMessageToAdmin(const std::string &message) {
    Players.for_each([&message](Player *player) {
        if (player->hasGMRight(gmr_getgmcalls)) {
            ServerCommandPointer cmd = std::make_shared<SayTC>(player->getPosition(), message);
            player->Connection->addCommand(cmd);
        }
    });
}


std::string World::languagePrefix(int Language) {
    if (Language==0) {
        return "";
    } else if (Language==1) {
        return "[hum] ";
    } else if (Language==2) {
        return "[dwa] ";
    } else if (Language==3) {
        return "[elf] ";
    } else if (Language==4) {
        return "[liz] ";
    } else if (Language==5) {
        return "[orc] ";
    } else if (Language==6) {
        return "[hal] ";
    } else if (Language==7) {
        return "[fai] ";
    } else if (Language==8) {
        return "[gno] ";
    } else if (Language==9) {
        return "[gob] ";
    } else if (Language==10) {
        return "[anc] ";
    } else {
        return "";
    }
}

std::string World::languageNumberToSkillName(int languageNumber) {
    switch (languageNumber) {
    case 0:
        return "common language";

    case 1:
        return "human language";

    case 2:
        return "dwarf language";

    case 3:
        return "elf language";

    case 4:
        return "lizard language";

    case 5:
        return "orc language";

    case 6:
        return "halfling language";

    case 7:
        return "fairy language";

    case 8:
        return "gnome language";

    case 9:
        return "goblin language";

    case 10:
        return "ancient language";

    default:
        return "";

    }
}

Range World::getTalkRange(Character::talk_type tt) const {
    Range range;

    switch (tt) {
    case Character::tt_say:
        range.radius = 14;
        break;

    case Character::tt_whisper:
        range.radius = 2;
        range.zRadius = 0;
        break;

    case Character::tt_yell:
        range.radius = 30;
        break;
    }

    return range;
}

void World::sendMessageToAllPlayers(const std::string &message) {
    Players.for_each([&message](Player *player) {
        player->inform(message, Player::informBroadcast);
    });
}

void World::sendMessageToAllCharsInRange(const std::string &german, const std::string &english, Character::talk_type tt, Character *cc) {
    auto range = getTalkRange(tt);
    std::string spokenMessage_german, spokenMessage_english, tempMessage;
    bool is_action = german.substr(0, 3) == "#me";

    if (!is_action) {
        // alter message because of the speakers inability to speak...
        spokenMessage_german = cc->alterSpokenMessage(german, cc->getLanguageSkill(cc->getActiveLanguage()));
        spokenMessage_english = cc->alterSpokenMessage(english, cc->getLanguageSkill(cc->getActiveLanguage()));
    }

    // tell all OTHER players... (but tell them what they understand due to their inability to do so)
    // tell the player himself what he wanted to say
    std::string prefix = languagePrefix(cc->getActiveLanguage());

    for (const auto &player : Players.findAllCharactersInRangeOf(cc->getPosition(), range)) {
        if (!is_action && player->getId() != cc->getId()) {
            tempMessage = prefix + player->alterSpokenMessage(player->nls(spokenMessage_german, spokenMessage_english), player->getLanguageSkill(cc->getActiveLanguage()));
            player->receiveText(tt, tempMessage, cc);
        } else {
            if (is_action) {
                player->receiveText(tt, player->nls(german, english), cc);
            } else {
                player->receiveText(tt, prefix + player->nls(german, english), cc);
            }
        }
    }

    if (cc->getType() == Character::player) {
        // tell all npcs
        for (const auto &npc : Npc.findAllCharactersInRangeOf(cc->getPosition(), range)) {
            tempMessage=prefix + npc->alterSpokenMessage(english, npc->getLanguageSkill(cc->getActiveLanguage()));
            npc->receiveText(tt, tempMessage, cc);
        }

        // tell all monsters
        for (const auto &monster : Monsters.findAllCharactersInRangeOf(cc->getPosition(), range)) {
            monster->receiveText(tt, english, cc);
        }
    }
}

void World::sendLanguageMessageToAllCharsInRange(const std::string &message, Character::talk_type tt, Language lang, Character *cc) {
    auto range = getTalkRange(tt);

    // get all Players
    std::vector<Player *> players = Players.findAllCharactersInRangeOf(cc->getPosition(), range);
    // get all NPCs
    std::vector<NPC *> npcs = Npc.findAllCharactersInRangeOf(cc->getPosition(), range);
    // get all Monsters
    std::vector<Monster *> monsters = Monsters.findAllCharactersInRangeOf(cc->getPosition(), range);

    // alter message because of the speakers inability to speak...
    std::string spokenMessage,tempMessage;
    spokenMessage=cc->alterSpokenMessage(message,cc->getLanguageSkill(cc->getActiveLanguage()));

    // tell all OTHER players... (but tell them what they understand due to their inability to do so)
    // tell the player himself what he wanted to say
    if (message.substr(0, 3) == "#me") {
        for (const auto &player : players) {
            if (player->getPlayerLanguage() == lang) {
                player->receiveText(tt, message, cc);
            }
        }
    } else {
        for (const auto &player : players) {
            if (player->getPlayerLanguage() == lang) {
                if (player->getId() != cc->getId()) {
                    tempMessage=languagePrefix(cc->getActiveLanguage()) + player->alterSpokenMessage(spokenMessage, player->getLanguageSkill(cc->getActiveLanguage()));
                    player->receiveText(tt, tempMessage, cc);
                } else {
                    player->receiveText(tt, languagePrefix(cc->getActiveLanguage())+message, cc);
                }
            }
        }
    }

    if (cc->getType() == Character::player) {
        // tell all npcs
        for (const auto &npc : npcs) {
            tempMessage=languagePrefix(cc->getActiveLanguage()) + npc->alterSpokenMessage(spokenMessage, npc->getLanguageSkill(cc->getActiveLanguage()));
            npc->receiveText(tt, tempMessage, cc);
        }

        // tell all monsters
        for (const auto &monster : monsters) {
            monster->receiveText(tt, message, cc);
        }
    }
}


void World::sendMessageToAllCharsInRange(const std::string &message, Character::talk_type tt, Character *cc) {
    sendMessageToAllCharsInRange(message, message, tt, cc);
}

void World::makeGFXForAllPlayersInRange(const position &pos, int radius ,unsigned short int gfx) {
    Range range;
    range.radius = radius;

    for (const auto &player : Players.findAllCharactersInRangeOf(pos, range)) {
        ServerCommandPointer cmd = std::make_shared<GraphicEffectTC>(pos, gfx);
        player->Connection->addCommand(cmd);
    }
}


void World::makeSoundForAllPlayersInRange(const position &pos, int radius, unsigned short int sound) {
    Range range;
    range.radius = radius;

    for (const auto &player : Players.findAllCharactersInRangeOf(pos, range)) {
        ServerCommandPointer cmd = std::make_shared<SoundTC>(pos, sound);
        player->Connection->addCommand(cmd);
    }
}



void World::lookAtMapItem(Player *cp, const position &pos) {

    Field *field;
    Item titem;

    if (GetPToCFieldAt(field, pos)) {
        // Feld vorhanden
        if (field->ViewTopItem(titem)) {
            std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(titem.getId());
            ScriptItem n_item = titem;
            n_item.type = ScriptItem::it_field;
            n_item.pos = pos;
            n_item.owner = cp;

            if (script && script->existsEntrypoint("LookAtItem")) {
                script->LookAtItem(cp, n_item);
                return;
            }

            if (!lookAtItemScript->lookAtItem(cp, n_item)) {
                lookAtTile(cp, field->getTileId(), pos);
            }
        } else {
            lookAtTile(cp, field->getTileId(), pos);
        }
    }
}


void World::lookAtTile(Player *cp, unsigned short int tile, const position &pos) {
    const TilesStruct &tileStruct = Data::Tiles[tile];
    ServerCommandPointer cmd = std::make_shared<LookAtTileTC>(pos, cp->nls(tileStruct.German, tileStruct.English));
    cp->Connection->addCommand(cmd);
}


void World::lookAtShowcaseItem(Player *cp, uint8_t showcase, unsigned char position) {

    ScriptItem titem;

    if (cp->isShowcaseOpen(showcase)) {
        Container *ps = cp->getShowcaseContainer(showcase);

        if (ps != nullptr) {
            Container *tc;

            if (ps->viewItemNr(position, titem, tc)) {
                std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(titem.getId());
                ScriptItem n_item = titem;

                n_item.type = ScriptItem::it_container;
                n_item.pos = cp->getPosition();
                n_item.owner = cp;
                n_item.itempos = position;
                n_item.inside = ps;

                if (script && script->existsEntrypoint("LookAtItem")) {
                    script->LookAtItem(cp, n_item);
                    return;
                }

                lookAtItemScript->lookAtItem(cp, n_item);
            }
        }
    }
}



void World::lookAtInventoryItem(Player *cp, unsigned char position) {
    if (cp->characterItems[ position ].getId() != 0) {

        Item titem = cp->characterItems[ position ];

        std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(cp->characterItems[ position ].getId());
        ScriptItem n_item = cp->characterItems[ position ];

        if (position < MAX_BODY_ITEMS) {
            n_item.type = ScriptItem::it_inventory;
        } else {
            n_item.type = ScriptItem::it_belt;
        }

        n_item.itempos = position;
        n_item.pos = cp->getPosition();
        n_item.owner = cp;

        if (script && script->existsEntrypoint("LookAtItem")) {
            script->LookAtItem(cp, n_item);
            return;
        }

        lookAtItemScript->lookAtItem(cp, n_item);
    }
}

void World::forceIntroducePlayer(Player *cp, Player *Admin) {
    Admin->introducePlayer(cp);
}

void World::introduceMyself(Player *cp) {
    Range range;
    range.radius = 2;
    range.zRadius = 0;

    for (const auto &player : Players.findAllCharactersInRangeOf(cp->getPosition(), range)) {
        player->introducePlayer(cp);
    }
}

void World::sendWeather(Player *cp) {
    cp->sendWeather(weather);
}

void World::sendIGTime(Player *cp) {
    ServerCommandPointer cmd = std::make_shared<UpdateTimeTC>(static_cast<unsigned char>(getTime("hour")),static_cast<unsigned char>(getTime("minute")),static_cast<unsigned char>(getTime("day")),static_cast<unsigned char>(getTime("month")), static_cast<short int>(getTime("year")));
    cp->Connection->addCommand(cmd);
}

void World::sendIGTimeToAllPlayers() {
    Players.for_each([this](Player *player) {
        sendIGTime(player);
    });
}

void World::sendWeatherToAllPlayers() {
    Players.for_each([this](Player *player) {
        player->sendWeather(weather);
    });
}

