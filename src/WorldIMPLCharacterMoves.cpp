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
#include "netinterface/protocol/ServerCommands.hpp"
#include "script/LuaItemScript.hpp"
#include "Logger.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/NetInterface.hpp"
#include "netinterface/BasicServerCommand.hpp"
#include "Player.hpp"
#include "Field.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "data/Data.hpp"

void World::checkFieldAfterMove(Character *cc, Field *cfstart) {
    if (cfstart->HasSpecialItem()) {

        for (const auto &item : cfstart->items) {
            if (Data::TilesModItems.exists(item.getId())) {
                const auto &tmod = Data::TilesModItems[item.getId()];

                if ((tmod.Modificator & FLAG_SPECIALITEM) != 0) {

                    std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(item.getId());

                    if (script) {
                        script->CharacterOnField(cc);
                        return;
                    }
                }
            }
        }
    }

    if (cfstart->IsSpecialField()) {
        s_fieldattrib which;
        FIELDATTRIBHASH::iterator temp = specialfields.find(cc->getPosition());

        if (specialfields.end() != temp) {
            which = (*temp).second;

            switch (which.type) {
            case SOUNDFIELD:
                makeSoundForAllPlayersInRange(cc->getPosition(), 3, which.flags);
                break;

            case MUSICFIELD:

                if (cc->getType() == Character::player) {
                    dynamic_cast<Player *>(cc)->startMusic(which.flags);
                }

                break;
            }
        }
    }

    if (cc && Data::Triggers.exists(cc->getPosition())) {
        const auto &script = Data::Triggers.script(cc->getPosition());

        if (script) {
            script->CharacterOnField(cc);
        }
    }
}

void World::TriggerFieldMove(Character *cc, bool moveto) {
    if (cc && Data::Triggers.exists(cc->getPosition())) {
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

void World::moveTo(Character *cc, const position& to) {
    switch(cc->getType()) {
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


void World::sendSpinToAllVisiblePlayers(Character *cc) {
    for (const auto &p : Players.findAllCharactersInScreen(cc->getPosition())) {
        ServerCommandPointer cmd = std::make_shared<PlayerSpinTC>(cc->getFaceTo(), cc->getId());
        p->Connection->addCommand(cmd);
    }
}


void World::sendPassiveMoveToAllVisiblePlayers(Character *ccp) {
    char xoffs;
    char yoffs;
    char zoffs;
    const auto &charPos = ccp->getPosition();

    for (const auto &p : Players.findAllCharactersInScreen(charPos)) {
        const auto &playerPos = p->getPosition();
        xoffs = charPos.x - playerPos.x;
        yoffs = charPos.y - playerPos.y;
        zoffs = charPos.z - playerPos.z + RANGEDOWN;

        if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
            ServerCommandPointer cmd = std::make_shared<MoveAckTC>(ccp->getId(), charPos, PUSH, 0);
            p->Connection->addCommand(cmd);
        }
    }

}


void World::sendCharacterMoveToAllVisibleChars(Character *cc, unsigned char waitpages) {
    // for now we only send events to players... TODO change this whole command
    sendCharacterMoveToAllVisiblePlayers(cc, NORMALMOVE, waitpages);
}

void World::sendCharacterMoveToAllVisiblePlayers(Character *cc, unsigned char netid, unsigned char waitpages) {
    if (!cc->isInvisible()) {
        char xoffs;
        char yoffs;
        char zoffs;
        const auto &charPos = cc->getPosition();

        for (const auto &p : Players.findAllCharactersInScreen(charPos)) {
            const auto &playerPos = p->getPosition();
            xoffs = charPos.x - playerPos.x;
            yoffs = charPos.y - playerPos.y;
            zoffs = charPos.z - playerPos.z + RANGEDOWN;

            if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
                ServerCommandPointer cmd = std::make_shared<MoveAckTC>(cc->getId(), charPos, netid, waitpages);
                p->Connection->addCommand(cmd);
            }
        }
    }
}


void World::sendCharacterWarpToAllVisiblePlayers(Character *cc, const position &oldpos, unsigned char netid) {
    if (!cc->isInvisible()) {
        sendRemoveCharToVisiblePlayers(cc->getId(), oldpos);

        for (const auto &p : Players.findAllCharactersInScreen(cc->getPosition())) {
            if (cc != p) {
                ServerCommandPointer cmd = std::make_shared<MoveAckTC>(cc->getId(), cc->getPosition(), PUSH, 0);
                p->Connection->addCommand(cmd);
            }
        }
    }
}


void World::sendAllVisibleCharactersToPlayer(Player *cp, bool sendSpin) {
    std::vector < Player * > tempP = Players.findAllCharactersInRangeOf(cp->getPosition(), cp->getScreenRange());
    sendCharsInVector< Player >(tempP, cp, sendSpin);

    std::vector < Monster * > tempM = Monsters.findAllCharactersInRangeOf(cp->getPosition(), cp->getScreenRange());
    sendCharsInVector< Monster >(tempM, cp, sendSpin);

    std::vector < NPC * > tempN = Npc.findAllCharactersInRangeOf(cp->getPosition(), cp->getScreenRange());
    sendCharsInVector< NPC >(tempN, cp, sendSpin);

    cp->sendAvailableQuests();
}


template<class T>
void World::sendCharsInVector(const std::vector<T *> &vec, Player *cp, bool sendSpin) {
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


bool World::addWarpField(const position &where, const position &target, unsigned short int starttilenr, Item::id_type startitemnr) {
#ifdef World_CharMove_DEBUG
    std::cout << "addWarpField: " << where << "\n";
#endif
    Field *cfstart;

    // Startfeld vorhanden
    if (GetPToCFieldAt(cfstart, where)) {

        if (starttilenr != 0) {
            cfstart->setTileId(starttilenr);
        }

        if (startitemnr != 0) {
            Item warpfi;
            warpfi.setId(startitemnr);
            warpfi.setNumber(1);
            warpfi.makePermanent();
            cfstart->PutTopItem(warpfi);
        }

        cfstart->SetWarpField(target);
        cfstart->updateFlags();

        return true;
    } else {
        return false;
    }

}


bool World::makeSpecialField(const position &where, s_fieldattrib which) {
    std::cout << "addSpecialField: " << where << "\n";
    Field *cfstart;

    // Startfeld vorhanden
    if (GetPToCFieldAt(cfstart, where)) {
        cfstart->SetSpecialField(true);
        specialfields.insert(FIELDATTRIBHASH::value_type(where, which));

        return true;
    } else {
        return false;
    }
}


bool World::makeSpecialField(short int x, short int y, short int z, unsigned char type, unsigned long int value) {

    position where;
    s_fieldattrib which;
    where.x = x;
    where.y = y;
    where.z = z;
    which.type = type;
    which.flags = value;

    return makeSpecialField(where, which);

}


bool World::addWarpField(const position &where, const position &target, unsigned short int starttilenr, Item::id_type startitemnr, unsigned short int targettilenr, Item::id_type targetitemnr) {

    if (addWarpField(where, target, starttilenr, startitemnr)) {

        std::cout << "addWarp Unten -> Oben ist ok" << std::endl;

        if (addWarpField(target, where, targettilenr, targetitemnr)) {
            std::cout << "adwarp Oben -> Unten ist ok" << std::endl;
            return true;
        } else {
            removeWarpField(where);
        }
    }

    return false;

}


bool World::removeWarpField(const position &where) {

#ifdef World_CharMove_DEBUG
    std::cout << "removeWarpField: " << where << "\n";
#endif
    Field *cfstart;

    if (GetPToCFieldAt(cfstart, where)) {     // Startfeld vorhanden
        cfstart->UnsetWarpField();
        return true;
    }

    return false;
}

