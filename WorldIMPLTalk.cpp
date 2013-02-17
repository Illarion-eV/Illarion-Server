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


#include "World.hpp"
#include "data/NamesObjectTable.hpp"
#include "data/TilesTable.hpp"
#include "data/CommonObjectTable.hpp"
#include "script/LuaItemScript.hpp"
#include "script/LuaLookAtItemScript.hpp"
#include "TableStructs.hpp"
#include "Player.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "Logger.hpp"

extern CommonObjectTable *CommonItems;
extern boost::shared_ptr<LuaLookAtItemScript>lookAtItemScript;

bool World::sendTextInFileToPlayer(std::string filename, Player *cp) {
    if (filename.length() == 0) {
        return false;
    }

    FILE *fp;
    fp = fopen(filename.c_str(), "r");

    if (fp != NULL) {
        const unsigned char LINE_LENGTH = 255;
        char line[LINE_LENGTH];

        while (fgets(line, LINE_LENGTH, fp) != NULL) {
            boost::shared_ptr<BasicServerCommand>cmd(new SayTC(cp->pos.x, cp->pos.y, cp->pos.z, line));
            cp->Connection->addCommand(cmd);
        }

        fclose(fp);
        return true;
    } else {
        return false;
    }
}


void World::sendMessageToAdmin(std::string message) {
    PLAYERVECTOR::iterator titerator;

    for (titerator = Players.begin(); titerator < Players.end(); ++titerator) {
        if ((*titerator)->hasGMRight(gmr_getgmcalls)) {
            boost::shared_ptr<BasicServerCommand>cmd(new SayTC((*titerator)->pos.x, (*titerator)->pos.y, (*titerator)->pos.z, message));
            (*titerator)->Connection->addCommand(cmd);
        }
    }
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

void World::sendMessageToAllPlayers(std::string message) {
    PLAYERVECTOR::iterator titerator;

    for (titerator = Players.begin(); titerator < Players.end(); ++titerator) {
        (*titerator)->inform(message, Player::informBroadcast);
    }
}

void World::sendLanguageMessageToAllCharsInRange(std::string message, Character::talk_type tt, unsigned char lang, Character *cc) {
    uint16_t range = 0;

    // how far can we be heard?
    switch (tt) {
    case Character::tt_say:
        range = 14;
        break;

    case Character::tt_whisper:
        range = 2;
        break;

    case Character::tt_yell:
        range = 30;
        break;
    }

    //determine spoken language skill

    // get all Players
    std::vector<Player *> players = Players.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);
    // get all NPCs
    std::vector<NPC *> npcs = Npc.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);
    // get all Monsters
    std::vector<Monster *> monsters = Monsters.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);

    // alter message because of the speakers inability to speak...
    std::string spokenMessage,tempMessage;
    spokenMessage=cc->alterSpokenMessage(message,cc->getLanguageSkill(cc->activeLanguage));

    // tell all OTHER players... (but tell them what they understand due to their inability to do so)
    // tell the player himself what he wanted to say
    //std::cout << "message in WorldIMPLTalk:" << message;
    if ((message[0]=='#') && (message[1]=='m') && (message[2]=='e')) {
        for (PLAYERVECTOR::iterator it = players.begin(); it != players.end(); ++it) {
            if ((*it)->getPlayerLanguage() == static_cast<Language::LanguageType>(lang)) {
                (*it)->receiveText(tt, message, cc);
            }
        }
    } else {
        for (PLAYERVECTOR::iterator it = players.begin(); it != players.end(); ++it) {
            if ((*it)->getPlayerLanguage() == static_cast<Language::LanguageType>(lang)) {
                if ((*it)->id!=cc->id) {
                    tempMessage=languagePrefix(cc->activeLanguage)+(*it)->alterSpokenMessage(spokenMessage,(*it)->getLanguageSkill(cc->activeLanguage));
                    (*it)->receiveText(tt, tempMessage, cc);
                } else {
                    (*it)->receiveText(tt, languagePrefix(cc->activeLanguage)+message, cc);
                }
            }
        }
    }

    // NPCs talking to other NPCs will mess up thisNPC, so only let players talk to NPCs and monsters for now
    if (cc->character == Character::player) {

        // tell all npcs
        for (NPCVECTOR::iterator it = npcs.begin(); it != npcs.end(); ++it) {
            tempMessage=languagePrefix(cc->activeLanguage)+(*it)->alterSpokenMessage(spokenMessage,(*it)->getLanguageSkill(cc->activeLanguage));
            (*it)->receiveText(tt, tempMessage, cc);
        }

        // tell all monsters
        for (MONSTERVECTOR::iterator it = monsters.begin(); it != monsters.end(); ++it) {
            (*it)->receiveText(tt, message, cc);
        }

    }
}


void World::sendMessageToAllCharsInRange(std::string message, Character::talk_type tt, Character *cc) {
    uint16_t range = 0;

    // how far can we be heard?
    switch (tt) {
    case Character::tt_say:
        range = 14;
        break;

    case Character::tt_whisper:
        range = 2;
        break;

    case Character::tt_yell:
        range = 30;
        break;
    }

    //determine spoken language skill

    //int activeLanguageSkill;



    // get all Players
    std::vector<Player *> players = Players.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);

    // get all NPCs
    std::vector<NPC *> npcs = Npc.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);

    // get all Monsters
    std::vector<Monster *> monsters = Monsters.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);

    // alter message because of the speakers inability to speak...
    std::string spokenMessage,tempMessage;
    spokenMessage=cc->alterSpokenMessage(message,cc->getLanguageSkill(cc->activeLanguage));

    // tell all OTHER players... (but tell them what they understand due to their inability to do so)
    // tell the player himself what he wanted to say
    //std::cout << "message in WorldIMPLTalk:" << message;
    if ((message[0]=='#') && (message[1]=='m') && (message[2]=='e')) {
        for (PLAYERVECTOR::iterator it = players.begin(); it != players.end(); ++it) {
            (*it)->receiveText(tt, message, cc);
        }
    } else {
        for (PLAYERVECTOR::iterator it = players.begin(); it != players.end(); ++it) {
            if ((*it)->id!=cc->id) {
                tempMessage=languagePrefix(cc->activeLanguage)+(*it)->alterSpokenMessage(spokenMessage,(*it)->getLanguageSkill(cc->activeLanguage));
                (*it)->receiveText(tt, tempMessage, cc);
            } else {
                (*it)->receiveText(tt, languagePrefix(cc->activeLanguage)+message, cc);
            }
        }
    }


    // tell all npcs
    for (NPCVECTOR::iterator it = npcs.begin(); it != npcs.end(); ++it) {
        tempMessage=languagePrefix(cc->activeLanguage)+(*it)->alterSpokenMessage(spokenMessage,(*it)->getLanguageSkill(cc->activeLanguage));
        (*it)->receiveText(tt, tempMessage, cc);
    }

    // tell all monsters
    for (MONSTERVECTOR::iterator it = monsters.begin(); it != monsters.end(); ++it) {
        (*it)->receiveText(tt, message, cc);
    }
}


void World::makeGFXForAllPlayersInRange(short int xc, short int yc, short int zc, int distancemetric ,unsigned short int gfx) {
    std::vector < Player * > temp = Players.findAllCharactersInRangeOf(xc, yc, zc, distancemetric);
    std::vector < Player * > ::iterator titerator;

    for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
        boost::shared_ptr<BasicServerCommand>cmd(new GraphicEffectTC(xc, yc, zc, gfx));
        (*titerator)->Connection->addCommand(cmd);
    }
}


void World::makeSoundForAllPlayersInRange(short int xc, short int yc, short int zc, int distancemetric, unsigned short int sound) {
    std::vector < Player * > temp = Players.findAllCharactersInRangeOf(xc, yc, zc, distancemetric);
    std::vector < Player * > ::iterator titerator;

    for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
        boost::shared_ptr<BasicServerCommand>cmd(new SoundTC(xc, yc, zc, sound));
        (*titerator)->Connection->addCommand(cmd);
    }
}



void World::lookAtMapItem(Player *cp, short int x, short int y, short int z) {

    Field *field;
    Item titem;

    if (GetPToCFieldAt(field, x, y, z)) {
        // Feld vorhanden
        if (field->ViewTopItem(titem)) {
            boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(titem.getId());
            ScriptItem n_item = titem;
            n_item.type = ScriptItem::it_field;
            n_item.pos = position(x, y, z);
            n_item.owner = cp;

            if (script && script->existsEntrypoint("LookAtItem")) {
                script->LookAtItem(cp, n_item);
                return;
            }

            if (!lookAtItemScript->lookAtItem(cp, n_item)) {
                lookAtTile(cp, field->getTileId(), x, y, z);
            }
        } else {
            lookAtTile(cp, field->getTileId(), x, y, z);
        }
    }
}


void World::lookAtTile(Player *cp, unsigned short int tile, short int x, short int y, short int z) {
    const TilesStruct &tileStruct = Data::Tiles[tile];
    boost::shared_ptr<BasicServerCommand>cmd(new LookAtTileTC(x, y, z, cp->nls(tileStruct.German, tileStruct.English)));
    cp->Connection->addCommand(cmd);
}


void World::lookAtShowcaseItem(Player *cp, uint8_t showcase, unsigned char position) {

    ScriptItem titem;

    if (cp->isShowcaseOpen(showcase)) {
        Container *ps = cp->getShowcaseContainer(showcase);

        if (ps != NULL) {
            Container *tc;

            if (ps->viewItemNr(position, titem, tc)) {
                boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(titem.getId());
                ScriptItem n_item = titem;

                n_item.type = ScriptItem::it_container;
                n_item.pos = cp->pos;
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

        boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(cp->characterItems[ position ].getId());
        ScriptItem n_item = cp->characterItems[ position ];

        if (position < MAX_BODY_ITEMS) {
            n_item.type = ScriptItem::it_inventory;
        } else {
            n_item.type = ScriptItem::it_belt;
        }

        n_item.itempos = position;
        n_item.pos = cp->pos;
        n_item.owner = cp;

        if (script && script->existsEntrypoint("LookAtItem")) {
            script->LookAtItem(cp, n_item);
            return;
        }

        lookAtItemScript->lookAtItem(cp, n_item);
    }
}



void World::message(std::string message[3], Player *cp) {
    if (cp != NULL) {
        std::string out = cp->nls(message[Language::german], message[Language::english]);
        boost::shared_ptr<BasicServerCommand>cmd(new SayTC(cp->pos.x, cp->pos.y, cp->pos.z ,out));
        cp->Connection->addCommand(cmd);
    }
}

void World::forceIntroducePlayer(Player *cp, Player *Admin) {
    Admin->introducePlayer(cp);
}

void World::introduceMyself(Player *cp) {
    std::vector < Player * > temp = Players.findAllCharactersInRangeOf(cp->pos.x, cp->pos.y, cp->pos.z, 2);
    std::vector < Player * > ::iterator titerator;

    for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
        (*titerator)->introducePlayer(cp);
    }
}

void World::sendWeather(Player *cp) {
    cp->sendWeather(weather);
}

void World::sendIGTime(Player *cp) {
    boost::shared_ptr<BasicServerCommand>cmd(new UpdateTimeTC(static_cast<unsigned char>(getTime("hour")),static_cast<unsigned char>(getTime("minute")),static_cast<unsigned char>(getTime("day")),static_cast<unsigned char>(getTime("month")), static_cast<short int>(getTime("year"))));
    cp->Connection->addCommand(cmd);
}

void World::sendIGTimeToAllPlayers() {
    PLAYERVECTOR::iterator titerator;

    for (titerator = Players.begin(); titerator != Players.end(); ++titerator) {
        sendIGTime((*titerator));
    }
}

void World::sendWeatherToAllPlayers() {
    PLAYERVECTOR::iterator titerator;

    for (titerator = Players.begin(); titerator != Players.end(); ++titerator) {
        (*titerator)->sendWeather(weather);
    }
}

