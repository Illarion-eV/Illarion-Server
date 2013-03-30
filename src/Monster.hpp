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


#ifndef MONSTER_HPP
#define MONSTER_HPP

#include "Character.hpp"

class SpawnPoint;

/**
* @ingroup Scriptclasses
* a monster in the game world
*/
class Monster : public Character {

public:

    /**
    * an exception which is thrown if a id for the monster is unknown
    */
    class unknownIDException { };

    /**
    * the constructor which creates the monster on the map
    * @param type the monster type of this monster
    * @param newpos the position where the monster should be spawned
    * @param spawnpoint the spawnpoint from which the monster was spawned ( 0 if there is no spawnpoint )
    */
    Monster(const TYPE_OF_CHARACTER_ID &type, const position &newpos, SpawnPoint *spawnpoint=0) throw(unknownIDException);

    virtual unsigned short getType() const override {
        return monster;
    }

    /**
    * sets the type of the monster and changes
    * so the attributes etc for this monster
    * @param type the new type of the monster
    * @throw unknownIDException
    */
    void setMonsterType(const TYPE_OF_CHARACTER_ID &type) throw(unknownIDException);

    /**
    * sets the spawnpoint for thins monster to a new one
    * @param sp the pointer to the new spawnpoint
    */
    void setSpawn(SpawnPoint *sp);

    /**
    * sets the alive state of the monster
    * if a monster is killed it will be removed from his spawnpoint
    * @param t the new lifestate true = alive false = death
    */
    void SetAlive(bool t);

    void remove();

    virtual std::string to_string() const override;

    /**
    * the monster gets a talked text from a character near starts a script entry point
    * @param tt how loudly the text is spoken
    * @param message the message which is spoken
    * @param cc the character who has spoken the message
    */
    void receiveText(talk_type tt, const std::string &message, Character *cc);

    /**
    * returns the spawnpoint of the monster
    * @return the spawnpoint of this monster
    */
    inline SpawnPoint *getSpawn() const {
        return spawn;
    }

    /**
    * returns the type of this monster
    * @return the type
    */
    virtual TYPE_OF_CHARACTER_ID getMonsterType() const override {
        return monstertype;
    }

    /**
    * the monster is attacking another character starts a script entry
    * @param target the character which is attacked
    * @return no usage at this time anything can be returned
    */
    virtual bool attack(Character *target) override;

    void heal();

    /**
    *trys to find a path to the the targetposition and performs a step
    *in the direction
    *@param targetpos targetposition for the move;
    */
    void performStep(position targetpos);

    /**
    * destructor
    */
    virtual ~Monster() override;

    position lastTargetPosition; /**< last position of the last seen target */
    bool lastTargetSeen; /**< if true the monster trys to reach the last targetposition if there is no other enemy*/

    /**
    * checks if the monster can attack onther one or it is a peacefull monster
    * @return true if the monster is aggresive otherwise false
    */
    inline const bool canAttack() {
        return _canAttack;
    }

    std::string nameDe;

private:

    static uint32_t counter;
    SpawnPoint *spawn;
    TYPE_OF_CHARACTER_ID monstertype;
    bool _canAttack;
};

#endif // MONSTER_HPP
