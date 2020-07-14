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

#include "Logger.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "TableStructs.hpp"
#include "World.hpp"
#include "data/Data.hpp"
#include "data/TilesTable.hpp"
#include "map/Field.hpp"
#include "netinterface/protocol/ServerCommands.hpp"

void World::sendMessageToAdmin(const std::string &message) const {
    Players.for_each([&message](Player *player) {
        if (player->hasGMRight(gmr_getgmcalls)) {
            ServerCommandPointer cmd = std::make_shared<SayTC>(player->getPosition(), message);
            player->Connection->addCommand(cmd);
        }
    });
}

auto World::languagePrefix(int languageId) -> std::string {
    switch (languageId) {
    case languageHuman:
        return "[hum] ";

    case languageDwarf:
        return "[dwa] ";

    case languageElf:
        return "[elf] ";

    case languageLizard:
        return "[liz] ";

    case languageOrc:
        return "[orc] ";

    case languageHalfling:
        return "[hal] ";

    case languageFairy:
        return "[fai] ";

    case languageGnome:
        return "[gno] ";

    case languageGoblin:
        return "[gob] ";

    case languageAncient:
        return "[anc] ";

    default:
        return "";
    }
}

auto World::languageNumberToSkillName(int languageId) -> std::string {
    switch (languageId) {
    case languageCommon:
        return "common language";

    case languageHuman:
        return "human language";

    case languageDwarf:
        return "dwarf language";

    case languageElf:
        return "elf language";

    case languageLizard:
        return "lizard language";

    case languageOrc:
        return "orc language";

    case languageHalfling:
        return "halfling language";

    case languageFairy:
        return "fairy language";

    case languageGnome:
        return "gnome language";

    case languageGoblin:
        return "goblin language";

    case languageAncient:
        return "ancient language";

    default:
        return "";
    }
}

auto World::getTalkRange(Character::talk_type tt) -> Range {
    Range range;

    switch (tt) {
    case Character::tt_say:
        range.radius = talkRange;
        break;

    case Character::tt_whisper:
        range.radius = whisperRange;
        range.zRadius = 0;
        break;

    case Character::tt_yell:
        range.radius = yellRange;
        break;
    }

    return range;
}

void World::sendMessageToAllPlayers(const std::string &message) const {
    Players.for_each([&message](Player *player) { player->inform(message, Player::informBroadcast); });
}

void World::broadcast(const std::string &german, const std::string &english) const {
    Players.for_each([&german, &english](Player *player) { player->inform(german, english, Player::informBroadcast); });
}

void World::sendMessageToAllCharsInRange(const std::string &german, const std::string &english, Character::talk_type tt,
                                         Character *cc) const {
    auto range = getTalkRange(tt);
    bool is_action = german.substr(0, 3) == "#me";

    std::string prefix = languagePrefix(cc->getActiveLanguage());

    for (const auto &player : Players.findAllCharactersInRangeOf(cc->getPosition(), range)) {
        if (is_action) {
            player->receiveText(tt, player->nls(german, english), cc);
        } else {
            player->receiveText(tt, prefix + player->nls(german, english), cc);
        }
    }

    if (cc->getType() == Character::player) {
        for (const auto &npc : Npc.findAllCharactersInRangeOf(cc->getPosition(), range)) {
            npc->receiveText(tt, prefix + english, cc);
        }

        for (const auto &monster : Monsters.findAllCharactersInRangeOf(cc->getPosition(), range)) {
            monster->receiveText(tt, english, cc);
        }
    }
}

void World::sendLanguageMessageToAllCharsInRange(const std::string &message, Character::talk_type tt, Language lang,
                                                 Character *cc) const {
    auto range = getTalkRange(tt);

    std::vector<Player *> players = Players.findAllCharactersInRangeOf(cc->getPosition(), range);
    std::vector<NPC *> npcs = Npc.findAllCharactersInRangeOf(cc->getPosition(), range);
    std::vector<Monster *> monsters = Monsters.findAllCharactersInRangeOf(cc->getPosition(), range);

    if (message.substr(0, 3) == "#me") {
        for (const auto &player : players) {
            if (player->getPlayerLanguage() == lang) {
                player->receiveText(tt, message, cc);
            }
        }
    } else {
        for (const auto &player : players) {
            if (player->getPlayerLanguage() == lang) {
                player->receiveText(tt, languagePrefix(cc->getActiveLanguage()) + message, cc);
            }
        }
    }

    if (cc->getType() == Character::player) {
        for (const auto &npc : npcs) {
            npc->receiveText(tt, languagePrefix(cc->getActiveLanguage()) + message, cc);
        }

        for (const auto &monster : monsters) {
            monster->receiveText(tt, message, cc);
        }
    }
}

void World::sendMessageToAllCharsInRange(const std::string &message, Character::talk_type tt, Character *cc) const {
    sendMessageToAllCharsInRange(message, message, tt, cc);
}

void World::makeGFXForAllPlayersInRange(const position &pos, int radius, unsigned short int gfx) const {
    Range range;
    range.radius = radius;

    for (const auto &player : Players.findAllCharactersInRangeOf(pos, range)) {
        ServerCommandPointer cmd = std::make_shared<GraphicEffectTC>(pos, gfx);
        player->Connection->addCommand(cmd);
    }
}

void World::makeSoundForAllPlayersInRange(const position &pos, int radius, unsigned short int sound) const {
    Range range;
    range.radius = radius;

    for (const auto &player : Players.findAllCharactersInRangeOf(pos, range)) {
        ServerCommandPointer cmd = std::make_shared<SoundTC>(pos, sound);
        player->Connection->addCommand(cmd);
    }
}

void World::lookAtMapItem(Player *player, const position &pos, uint8_t stackPos) {
    try {
        map::Field &field = fieldAt(pos);
        ScriptItem item = field.getStackItem(stackPos);

        if (item.getId() != 0) {
            item.type = ScriptItem::it_field;
            item.pos = pos;
            item.itempos = stackPos;
            item.owner = player;

            ItemLookAt lookAt = item.getLookAt(player);

            if (lookAt.isValid()) {
                itemInform(player, item, lookAt);
            } else {
                lookAtTile(player, field.getTileId(), pos);
            }
        } else {
            lookAtTile(player, field.getTileId(), pos);
        }
    } catch (FieldNotFound &) {
    }
}

void World::lookAtTile(Player *cp, unsigned short int tile, const position &pos) {
    const TilesStruct &tileStruct = Data::tiles()[tile];
    ServerCommandPointer cmd = std::make_shared<LookAtTileTC>(pos, cp->nls(tileStruct.German, tileStruct.English));
    cp->Connection->addCommand(cmd);
}

void World::lookAtShowcaseItem(Player *cp, uint8_t showcase, unsigned char position) {
    ScriptItem titem;

    if (cp->isShowcaseOpen(showcase)) {
        Container *ps = cp->getShowcaseContainer(showcase);

        if (ps != nullptr) {
            Container *tc = nullptr;

            if (ps->viewItemNr(position, titem, tc)) {
                ScriptItem n_item = titem;

                n_item.type = ScriptItem::it_container;
                n_item.pos = cp->getPosition();
                n_item.owner = cp;
                n_item.itempos = position;
                n_item.inside = ps;

                ItemLookAt lookAt = n_item.getLookAt(cp);

                if (lookAt.isValid()) {
                    itemInform(cp, n_item, lookAt);
                }
            }
        }
    }
}

void World::lookAtInventoryItem(Player *cp, unsigned char position) {
    if (cp->items.at(position).getId() != 0) {
        ScriptItem item(cp->items.at(position));

        if (position < MAX_BODY_ITEMS) {
            item.type = ScriptItem::it_inventory;
        } else {
            item.type = ScriptItem::it_belt;
        }

        item.itempos = position;
        item.pos = cp->getPosition();
        item.owner = cp;

        ItemLookAt lookAt = item.getLookAt(cp);

        if (lookAt.isValid()) {
            itemInform(cp, item, lookAt);
        }
    }
}

void World::forceIntroducePlayer(Player *cp, Player *admin) { admin->introducePlayer(cp); }

void World::introduceMyself(Player *cp) const {
    Range range;
    range.radius = 2;
    range.zRadius = 0;

    for (const auto &player : Players.findAllCharactersInRangeOf(cp->getPosition(), range)) {
        player->introducePlayer(cp);
    }
}

void World::sendWeather(Player *cp) const { cp->sendWeather(weather); }

void World::sendIGTime(Player *cp) const {
    ServerCommandPointer cmd = std::make_shared<UpdateTimeTC>(
            static_cast<unsigned char>(getTime("hour")), static_cast<unsigned char>(getTime("minute")),
            static_cast<unsigned char>(getTime("day")), static_cast<unsigned char>(getTime("month")),
            static_cast<short int>(getTime("year")));
    cp->Connection->addCommand(cmd);
}

void World::sendIGTimeToAllPlayers() {
    Players.for_each([this](Player *player) { sendIGTime(player); });
}

void World::sendWeatherToAllPlayers() {
    Players.for_each([this](Player *player) { player->sendWeather(weather); });
}
