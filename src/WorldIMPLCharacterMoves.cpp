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
#include "World.hpp"
#include "data/Data.hpp"
#include "map/Field.hpp"
#include "netinterface/BasicServerCommand.hpp"
#include "netinterface/NetInterface.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "script/LuaItemScript.hpp"

void World::checkFieldAfterMove(Character *character, const map::Field &field) {
    if (character == nullptr) {
        return;
    }

    if (character->isAlive() && field.hasSpecialItem()) {
        for (const auto &item : field.getItemStack()) {
            if (Data::TilesModItems.exists(item.getId())) {
                const auto &tmod = Data::TilesModItems[item.getId()];

                if ((tmod.Modificator & FLAG_SPECIALITEM) != 0) {
                    std::shared_ptr<LuaItemScript> script = Data::Items.script(item.getId());

                    if (script) {
                        script->CharacterOnField(character);
                        return;
                    }
                }
            }
        }
    }

    if (character->isAlive() && Data::Triggers.exists(character->getPosition())) {
        const auto &script = Data::Triggers.script(character->getPosition());

        if (script) {
            script->CharacterOnField(character);
        }
    }
}

void World::TriggerFieldMove(Character *cc, bool moveto) {
    if ((cc != nullptr) && cc->isAlive() && Data::Triggers.exists(cc->getPosition())) {
        const auto &script = Data::Triggers.script(cc->getPosition());

        if (script) {
            if (moveto) {
                script->MoveToField(cc);
            } else {
                script->MoveFromField(cc);
            }
        }
    }
}

void World::moveTo(Character *cc, const position &to) {
    switch (cc->getType()) {
    case Character::player:
        Players.update(dynamic_cast<Player *>(cc), to);
        break;
    case Character::monster:
        Monsters.update(dynamic_cast<Monster *>(cc), to);
        break;
    case Character::npc:
        Npc.update(dynamic_cast<NPC *>(cc), to);
        break;
    }
}

void World::sendSpinToAllVisiblePlayers(Character *cc) const {
    for (const auto &p : Players.findAllCharactersInScreen(cc->getPosition())) {
        ServerCommandPointer cmd = std::make_shared<PlayerSpinTC>(cc->getFaceTo(), cc->getId());
        p->Connection->addCommand(cmd);
    }
}

void World::sendPassiveMoveToAllVisiblePlayers(Character *ccp) const {
    const auto &charPos = ccp->getPosition();

    for (const auto &p : Players.findAllCharactersInScreen(charPos)) {
        const auto &playerPos = p->getPosition();
        char xoffs = charPos.x - playerPos.x;
        char yoffs = charPos.y - playerPos.y;
        char zoffs = charPos.z - playerPos.z + RANGEDOWN;

        if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
            ServerCommandPointer cmd = std::make_shared<MoveAckTC>(ccp->getId(), charPos, PUSH, 0);
            p->Connection->addCommand(cmd);
        }
    }
}

void World::sendCharacterMoveToAllVisibleChars(Character *cc, TYPE_OF_WALKINGCOST duration) const {
    sendCharacterMoveToAllVisiblePlayers(cc, NORMALMOVE, duration);
}

void World::sendCharacterMoveToAllVisiblePlayers(Character *cc, unsigned char moveType,
                                                 TYPE_OF_WALKINGCOST duration) const {
    if (!cc->isInvisible()) {
        const auto &charPos = cc->getPosition();

        for (const auto &p : Players.findAllCharactersInScreen(charPos)) {
            const auto &playerPos = p->getPosition();
            char xoffs = charPos.x - playerPos.x;
            char yoffs = charPos.y - playerPos.y;
            char zoffs = charPos.z - playerPos.z + RANGEDOWN;

            if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
                ServerCommandPointer cmd = std::make_shared<MoveAckTC>(cc->getId(), charPos, moveType, duration);
                p->Connection->addCommand(cmd);
            }
        }
    }
}

void World::sendCharacterWarpToAllVisiblePlayers(Character *cc, const position &oldpos, unsigned char moveType) const {
    if (!cc->isInvisible()) {
        {
            ServerCommandPointer cmd = std::make_shared<RemoveCharTC>(cc->getId());

            for (const auto &player : Players.findAllCharactersInScreen(oldpos)) {
                if (!player->isInScreen(cc->getPosition())) {
                    player->sendCharRemove(cc->getId(), cmd);
                }
            }
        }

        for (const auto &p : Players.findAllCharactersInScreen(cc->getPosition())) {
            if (cc != p) {
                ServerCommandPointer cmd = std::make_shared<MoveAckTC>(cc->getId(), cc->getPosition(), PUSH, 0);
                p->Connection->addCommand(cmd);
            }
        }
    }
}

void World::sendAllVisibleCharactersToPlayer(Player *cp, bool sendSpin) {
    Range range;
    range.radius = cp->getScreenRange();

    std::vector<Player *> tempP = Players.findAllCharactersInRangeOf(cp->getPosition(), range);
    sendCharsInVector<Player>(tempP, cp, sendSpin);

    std::vector<Monster *> tempM = Monsters.findAllCharactersInRangeOf(cp->getPosition(), range);
    sendCharsInVector<Monster>(tempM, cp, sendSpin);

    std::vector<NPC *> tempN = Npc.findAllCharactersInRangeOf(cp->getPosition(), range);
    sendCharsInVector<NPC>(tempN, cp, sendSpin);

    cp->sendAvailableQuests();
}

template <class T> void World::sendCharsInVector(const std::vector<T *> &vec, Player *cp, bool sendSpin) {
    char xoffs;
    char yoffs;
    char zoffs;
    const auto &playerPos = cp->getPosition();

    for (const auto &cc : vec) {
        if (!cc->isInvisible()) {
            const auto &charPos = cc->getPosition();
            xoffs = charPos.x - playerPos.x;
            yoffs = charPos.y - playerPos.y;
            zoffs = charPos.z - playerPos.z + RANGEDOWN;

            if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
                ServerCommandPointer cmd = std::make_shared<MoveAckTC>(cc->getId(), charPos, PUSH, 0);
                cp->Connection->addCommand(cmd);
                cmd = std::make_shared<PlayerSpinTC>(cc->getFaceTo(), cc->getId());

                if (sendSpin) {
                    cp->Connection->addCommand(cmd);
                }
            }
        }
    }
}

auto World::addWarpField(const position &where, const position &target, unsigned short int starttilenr,
                         Item::id_type startitemnr) -> bool {
    try {
        map::Field &field = fieldAt(where);

        if (starttilenr != 0) {
            field.setTileId(starttilenr);
        }

        if (startitemnr != 0) {
            Item warpfi;
            warpfi.setId(startitemnr);
            warpfi.setNumber(1);
            warpfi.makePermanent();
            field.addItemOnStack(warpfi);
        }

        field.setWarp(target);

        return true;
    } catch (FieldNotFound &) {
        return false;
    }
}

auto World::addWarpField(const position &where, const position &target, unsigned short int starttilenr,
                         Item::id_type startitemnr, unsigned short int targettilenr, Item::id_type targetitemnr)
        -> bool {
    if (addWarpField(where, target, starttilenr, startitemnr)) {
        if (addWarpField(target, where, targettilenr, targetitemnr)) {
            return true;
        }
        removeWarpField(where);
    }

    return false;
}

auto World::removeWarpField(const position &pos) -> bool {
    try {
        fieldAt(pos).removeWarp();
        return true;
    } catch (FieldNotFound &) {
        return false;
    }
}
