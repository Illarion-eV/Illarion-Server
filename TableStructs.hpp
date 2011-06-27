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


#ifndef TABLESTRUCTS_HPP
#define TABLESTRUCTS_HPP

#define BOOST_HAS_THREADS

#include <string>
#include "types.hpp"
#include <boost/shared_ptr.hpp>
#include <pthread.h>
#include <iostream>

class CLuaTileScript;
class CLuaItemScript;
class CLuaWeaponScript;
class CLuaLongTimeEffectScript;

#define ID                 1

#define WEIGHT             2
#define VOLUME             3
#define AGEINGSPEED        4
#define OBJECTAFTERROT     5

#define GERMAN             7
#define ENGLISH            8
#define FRENCH             9

#define IMAGEFILENAME      10
#define PIXELOFFSETX       11
#define PIXELOFFSETY       12
#define NROFANIMATIONS     13
#define CANOVERLAP         14

#define FOODSHARE          15
#define DRINKSHARE         16

#define ATTACK             17
#define DEFENCE            18
#define ACCURACY           19
#define RANGE              20
#define WEAPONTYPE         21

#define BODYPARTS          23
#define STROKEARMOR        24
#define THRUSTARMOR        25

#define ARTEFACTS          26

#define CONTAINERVOLUME    27

#define ISNOTPASSABLE      28
#define ISNOTTRANSPARENT   29
#define ISNOTPENETRATEABLE 30
#define WALKINGCOST        31

#define AUTOMAPCOLOR       32

#define GROUNDLEVEL        33
#define SPECIALTILE        34
#define SPECIALITEM        35
#define CATEGORY           36

#define AMMUNITIONTYPE     37
#define ACTIONPOINTS       38
#define PUNCTUREARMOR      39


struct AllObjectsTableStruct {
	std::string Id;

	std::string Weight;
	std::string Volume;
	std::string AgeingSpeed;
	std::string ObjectAfterRot;

	std::string German;
	std::string English;
	std::string French;

	std::string ImageFilename;
	std::string PixelOffsetX;
	std::string PixelOffsetY;
	std::string NrOfAnimations;
	std::string canOverlap;

	std::string Attack;
	std::string Defence;
	std::string Accuracy;
	std::string Range;
	std::string WeaponType;
	std::string AmmunitionType;
	std::string ActionPoints;

	std::string ArmorType;
	std::string BodyParts;
	std::string PunctureArmor;
	std::string StrokeArmor;
	std::string ThrustArmor;

	std::string Artefacts;

	std::string ContainerVolume;

	std::string isNotPassable;
	std::string isNotTransparent;
	std::string isNotPenetrateable;
	std::string SpecialItem;
	std::string GroundLevel;
	std::string Category;
};


struct AllTilesTableStruct {
	std::string Id;

	std::string German;
	std::string English;
	std::string French;

	std::string ImageFilename;
	std::string NrOfAnimations;

	std::string AutoMapColor;

	std::string isNotPassable;
	std::string isNotTransparent;
	std::string isNotPenetrateable;

	std::string walkingCost;

	std::string GroundLevel;
	std::string SpecialTile;
};


struct CommonStruct
{
	TYPE_OF_ITEM_ID id;
	TYPE_OF_WEIGHT Weight;
	TYPE_OF_VOLUME Volume;
	TYPE_OF_AGEINGSPEED AgeingSpeed;
	TYPE_OF_ITEM_ID ObjectAfterRot;
    TYPE_OF_ITEM_ID AfterInfiniteRot;
    TYPE_OF_BRIGHTNESS Brightness;
    TYPE_OF_WORTH Worth;
	bool rotsInInventory;
	bool isStackable;
	//pthread_mutex_t it_mutex;
    //boost::shared_ptr<CLuaItemScript> script;
    
    
	CommonStruct()
	{
        id = 0;
		Weight = 0;
		Volume = 0;
		AgeingSpeed = 0;
		ObjectAfterRot = 0;
        AfterInfiniteRot = 0;
        Brightness = 0;
		rotsInInventory = false;
		isStackable = false;
	}
    
    CommonStruct( const CommonStruct & source )
    {
        id = source.id;
        Weight = source.Weight;
        Volume = source.Volume;
        AgeingSpeed = source.AgeingSpeed;
        ObjectAfterRot = source.ObjectAfterRot;
        AfterInfiniteRot = source.AfterInfiniteRot;
        Brightness = source.Brightness;
        rotsInInventory = source.rotsInInventory;
        isStackable = source.isStackable;
        //pthread_mutex_lock( &it_mutex );
        //script = source.script;
        //pthread_mutex_unlock( &it_mutex );
    }
    
    CommonStruct& operator=(const CommonStruct & source)
    {
        if ( this != &source )
        {
             //( ( CommonStruct ) * this ) = ( ( CommonStruct ) source );
            id = source.id;
            Weight = source.Weight;
            Volume = source.Volume;
            AgeingSpeed = source.AgeingSpeed;
            ObjectAfterRot = source.ObjectAfterRot;
            AfterInfiniteRot = source.AfterInfiniteRot;
            Brightness = source.Brightness;
            rotsInInventory = source.rotsInInventory;
            isStackable = source.isStackable;  
            //pthread_mutex_lock( &it_mutex );
            //script = source.script;
            //pthread_mutex_unlock( &it_mutex );        

        }
        return *this;
        
    }
} ;


struct TilesModificatorStruct {
	unsigned char Modificator;
};

struct LongTimeEffectStruct
{
    uint16_t effectid;
    std::string effectname;
    std::string scriptname;
    boost::shared_ptr<CLuaLongTimeEffectScript> script;
    LongTimeEffectStruct()
    {
         effectid = 0;
         effectname = "";
         scriptname = "";
    }
};

struct NamesStruct {
	TYPE_OF_GERMAN German;
	TYPE_OF_ENGLISH English;
	TYPE_OF_FRENCH French;
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
	boost::shared_ptr<CLuaWeaponScript> script;
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


struct ContainerStruct {
	TYPE_OF_CONTAINERVOLUME ContainerVolume;
};


struct TilesStruct {
	unsigned char flags;
	TYPE_OF_WALKINGCOST walkingCost;
	TYPE_OF_GERMAN German;
	TYPE_OF_ENGLISH English;
	TYPE_OF_FRENCH French;
	boost::shared_ptr<CLuaTileScript> script;
};


struct ArtefactStruct {
	TYPE_OF_ARTEFACTS Artefacts;
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

struct MonsterArmor
{
    short int strokeArmor;
    short int punctureArmor;
    short int thrustArmor;
    MonsterArmor() : strokeArmor(0), punctureArmor(0), thrustArmor(0) {}
};

#endif // TABLESTRUCTS_HPP
