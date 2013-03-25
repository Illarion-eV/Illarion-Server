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

bool World::warpMonster(Monster *cm, Field *cfstart) {
    if (cfstart->IsWarpField()) {
        const position oldpos = cm->pos;
        cfstart->GetWarpField(cm->pos);
        Field *cfend;

        if (findEmptyCFieldNear(cfend, cm->pos)) {
            cfstart->SetMonsterOnField(false);
            cfend->SetMonsterOnField(true);

            sendCharacterWarpToAllVisiblePlayers(cm, oldpos, PUSH);
            return true;
        } else {
            cm->pos = oldpos;
        }
    }

    return false;
}

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
        FIELDATTRIBHASH::iterator temp = specialfields.find(cc->pos);

        if (specialfields.end() != temp) {
            which = (*temp).second;

            switch (which.type) {
            case SOUNDFIELD:
                makeSoundForAllPlayersInRange(cc->pos, 3, which.flags);
                break;

            case MUSICFIELD:

                if (cc->character == Character::player) {
                    dynamic_cast<Player *>(cc)->startMusic(which.flags);
                }

                break;
            }
        }
    }

    if (cc && Data::Triggers.exists(cc->pos)) {
        const auto &script = Data::Triggers.script(cc->pos);

        if (script) {
            script->CharacterOnField(cc);
        }
    }
}

void World::TriggerFieldMove(Character *cc, bool moveto) {
    if (cc && Data::Triggers.exists(cc->pos)) {
        const auto &script = Data::Triggers.script(cc->pos);

        if (script) {
            if (moveto) {
                script->MoveToField(cc);
            } else {
                script->MoveFromField(cc);
            }
        }
    }
}

bool World::pushCharacter(Player *cp, TYPE_OF_CHARACTER_ID pushedCharId, direction dir) {
    Character *pushedChar = Players.findID(pushedCharId);

    if (!pushedChar) {
        Monsters.findID(pushedCharId);
    }

    if (pushedChar && pushedChar->pos.z == cp->pos.z && abs(pushedChar->pos.x - cp->pos.x)<=1 && abs(pushedChar->pos.y - cp->pos.y)<=1) {
        return pushedChar->move(dir,false);
    }

    return false;
}

bool World::spinPlayer(Player *cp, unsigned char d) {

    // die Blickrichtung �ndern
    switch (d) {
    case 0 :
        cp->faceto = Character::north;
        break;

    case 1 :
        cp->faceto = Character::northeast;
        break;

    case 2 :
        cp->faceto = Character::east;
        break;

    case 3 :
        cp->faceto = Character::southeast;
        break;

    case 4 :
        cp->faceto = Character::south;
        break;

    case 5 :
        cp->faceto = Character::southwest;
        break;

    case 6 :
        cp->faceto = Character::west;
        break;

    case 7 :
        cp->faceto = Character::northwest;
        break;
    }

    // allen sichtbaren Spielern die Drehung �bermitteln
    sendSpinToAllVisiblePlayers(cp);

    return true;

}


void World::sendSpinToAllVisiblePlayers(Character *cc) {
    for (const auto &p : Players.findAllCharactersInScreen(cc->pos)) {
        ServerCommandPointer cmd(new PlayerSpinTC(cc->faceto, cc->getId()));
        p->Connection->addCommand(cmd);
    }
}


void World::sendPassiveMoveToAllVisiblePlayers(Character *ccp) {
    char xoffs;
    char yoffs;
    char zoffs;

    for (const auto &p : Players.findAllCharactersInScreen(ccp->pos)) {
        xoffs = ccp->pos.x - p->pos.x;
        yoffs = ccp->pos.y - p->pos.y;
        zoffs = ccp->pos.z - p->pos.z + RANGEDOWN;

        if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
            ServerCommandPointer cmd(new MoveAckTC(ccp->getId(), ccp->pos, PUSH, 0));
            p->Connection->addCommand(cmd);
        }
    }

}


void World::sendCharacterMoveToAllVisibleChars(Character *cc, unsigned char waitpages) {
    // for now we only send events to players... TODO change this whole command
    sendCharacterMoveToAllVisiblePlayers(cc, NORMALMOVE, waitpages);
}

void World::sendCharacterMoveToAllVisiblePlayers(Character *cc, unsigned char netid, unsigned char waitpages) {
    if (!cc->isinvisible) {
        char xoffs;
        char yoffs;
        char zoffs;

        for (const auto &p : Players.findAllCharactersInScreen(cc->pos)) {
            xoffs = cc->pos.x - p->pos.x;
            yoffs = cc->pos.y - p->pos.y;
            zoffs = cc->pos.z - p->pos.z + RANGEDOWN;

            if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
                ServerCommandPointer cmd(new MoveAckTC(cc->getId(), cc->pos, netid, waitpages));
                p->Connection->addCommand(cmd);
            }
        }
    }
}


void World::sendCharacterWarpToAllVisiblePlayers(Character *cc, const position &oldpos, unsigned char netid) {
    if (!cc->isinvisible) {
        sendRemoveCharToVisiblePlayers(cc->getId(), oldpos);

        for (const auto &p : Players.findAllCharactersInScreen(cc->pos)) {
            if (cc != p) {
                ServerCommandPointer cmd(new MoveAckTC(cc->getId(), cc->pos, PUSH, 0));
                p->Connection->addCommand(cmd);
            }
        }
    }
}


void World::sendAllVisibleCharactersToPlayer(Player *cp, bool sendSpin) {
    std::vector < Player * > tempP = Players.findAllCharactersInRangeOf(cp->pos, cp->getScreenRange());
    sendCharsInVector< Player >(tempP, cp, sendSpin);

    std::vector < Monster * > tempM = Monsters.findAllCharactersInRangeOf(cp->pos, cp->getScreenRange());
    sendCharsInVector< Monster >(tempM, cp, sendSpin);

    std::vector < NPC * > tempN = Npc.findAllCharactersInRangeOf(cp->pos, cp->getScreenRange());
    sendCharsInVector< NPC >(tempN, cp, sendSpin);
}


template<class T>
void World::sendCharsInVector(const std::vector<T *> &vec, Player *cp, bool sendSpin) {
    char xoffs;
    char yoffs;
    char zoffs;

    for (const auto &cc : vec) {
        if (!cc->isinvisible) {
            xoffs = cc->pos.x - cp->pos.x;
            yoffs = cc->pos.y - cp->pos.y;
            zoffs = cc->pos.z - cp->pos.z + RANGEDOWN;

            if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
                ServerCommandPointer cmd(new MoveAckTC(cc->getId(), cc->pos, PUSH, 0));
                cp->Connection->addCommand(cmd);
                cmd.reset(new PlayerSpinTC(cc->faceto, cc->getId()));

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

