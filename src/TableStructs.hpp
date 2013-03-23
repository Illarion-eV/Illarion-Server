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


#ifndef _TABLESTRUCTS_HPP_
#define _TABLESTRUCTS_HPP_

#define BOOST_HAS_THREADS

#include <string>
#include "types.hpp"
#include <boost/shared_ptr.hpp>
#include <pthread.h>
#include <iostream>

class LuaTileScript;
class LuaItemScript;
class LuaWeaponScript;
class LuaLongTimeEffectScript;

struct CommonStruct {
    TYPE_OF_ITEM_ID id;
    TYPE_OF_WEIGHT Weight;
    TYPE_OF_AGINGSPEED AgeingSpeed;
    TYPE_OF_ITEM_ID ObjectAfterRot;
    TYPE_OF_ITEM_ID AfterInfiniteRot;
    TYPE_OF_BRIGHTNESS Brightness;
    TYPE_OF_WORTH Worth;
    TYPE_OF_MAX_STACK MaxStack;
    TYPE_OF_BUY_STACK BuyStack;
    bool rotsInInventory;

    CommonStruct() {
        id = 0;
        Weight = 0;
        AgeingSpeed = 0;
        ObjectAfterRot = 0;
        AfterInfiniteRot = 0;
        Brightness = 0;
        Worth = 0;
        MaxStack = 1;
        BuyStack = 1;
        rotsInInventory = false;
    }

    bool isValid() const {
        return id != 0;
    }
};

struct TilesModificatorStruct {
    unsigned char Modificator;
};

struct LongTimeEffectStruct {
    uint16_t effectid;
    std::string effectname;
    std::string scriptname;
    std::shared_ptr<LuaLongTimeEffectScript> script;
    LongTimeEffectStruct(): effectname(""), scriptname("") {
        effectid = 0;
    }
};

struct NamesStruct {
    TYPE_OF_GERMAN German;
    TYPE_OF_ENGLISH English;
};


struct WeaponStruct {
    TYPE_OF_ATTACK Attack;
    TYPE_OF_DEFENCE Defence;
    TYPE_OF_ACCURACY Accuracy;
    TYPE_OF_RANGE Range;
    TYPE_OF_WEAPONTYPE WeaponType;
    TYPE_OF_AMMUNITIONTYPE AmmunitionType;
    TYPE_OF_ACTIONPOINTS ActionPoints;
    TYPE_OF_MAGICDISTURBANCE MagicDisturbance;
    TYPE_OF_POISONSTRENGTH PoisonStrength;
    //Constructor
    WeaponStruct() : Attack(0) , Defence(0) , Accuracy(0) , Range(0) , WeaponType(0) , AmmunitionType(0) , ActionPoints(0) , MagicDisturbance(0) , PoisonStrength(0) {}
};


struct ArmorStruct {
    TYPE_OF_BODYPARTS BodyParts;
    TYPE_OF_PUNCTUREARMOR PunctureArmor;
    TYPE_OF_STROKEARMOR StrokeArmor;
    TYPE_OF_THRUSTARMOR ThrustArmor;
    TYPE_OF_MAGICDISTURBANCE MagicDisturbance;
    int16_t Absorb;
    int16_t Stiffness;
    ArmorStruct() : BodyParts(0) , PunctureArmor(0), StrokeArmor(0), ThrustArmor(0),MagicDisturbance(0),Absorb(0), Stiffness(0) {}
};


struct SkillStruct {
    std::string serverName;
    TYPE_OF_ENGLISH englishName;
    TYPE_OF_GERMAN germanName;
};


struct TilesStruct {
    unsigned char flags;
    TYPE_OF_WALKINGCOST walkingCost;
    TYPE_OF_GERMAN German;
    TYPE_OF_ENGLISH English;
};


struct PlayerraceStruct {
    std::string racename;
    short int points;
    short int minage;
    short int maxage;
    short int minweight;
    short int maxweight;
    unsigned char minbodyheight;
    unsigned char maxbodyheight;
    unsigned char minagility;
    unsigned char maxagility;
    unsigned char minconstitution;
    unsigned char maxconstitution;
    unsigned char mindexterity;
    unsigned char maxdexterity;
    unsigned char minessence;
    unsigned char maxessence;
    unsigned char minintelligence;
    unsigned char maxintelligence;
    unsigned char minperception;
    unsigned char maxperception;
    unsigned char minstrength;
    unsigned char maxstrength;
    unsigned char minwillpower;
    unsigned char maxwillpower;
};

struct MonsterArmor {
    short int strokeArmor;
    short int punctureArmor;
    short int thrustArmor;
    MonsterArmor() : strokeArmor(0), punctureArmor(0), thrustArmor(0) {}
};

#endif // TABLESTRUCTS_HPP
