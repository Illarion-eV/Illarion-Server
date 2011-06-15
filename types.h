#ifndef ILLA_TYPES_H
#define ILLA_TYPES_H

#include <stdint.h>

typedef uint16_t TYPE_OF_ITEM_ID;
typedef uint32_t TYPE_OF_CHARACTER_ID;

typedef unsigned short TYPE_OF_WEIGHT;
typedef unsigned short TYPE_OF_VOLUME;
typedef unsigned char  TYPE_OF_AGEINGSPEED;
typedef unsigned char  TYPE_OF_BRIGHTNESS;
typedef uint32_t TYPE_OF_WORTH;

typedef std::string TYPE_OF_GERMAN;
typedef std::string TYPE_OF_ENGLISH;
typedef std::string TYPE_OF_FRENCH;

#define TYPE_OF_ATTACK                 unsigned char
#define TYPE_OF_DEFENCE                unsigned char
#define TYPE_OF_ACCURACY               unsigned char
#define TYPE_OF_RANGE                  unsigned char
#define TYPE_OF_WEAPONTYPE             unsigned char
#define TYPE_OF_POISONSTRENGTH         unsigned char

#define TYPE_OF_BODYPARTS              unsigned char
#define TYPE_OF_STROKEARMOR            unsigned char
#define TYPE_OF_THRUSTARMOR            unsigned char
#define TYPE_OF_MAGICDISTURBANCE       unsigned short int

#define TYPE_OF_ARTEFACTS              unsigned char

#define TYPE_OF_CONTAINERVOLUME        unsigned short int

#define TYPE_OF_WALKINGCOST            unsigned char

#define TYPE_OF_GROUNDLEVEL            unsigned char
#define TYPE_OF_SPECIALTILE            unsigned char
#define TYPE_OF_SPECIALITEM            unsigned char
#define TYPE_OF_CATEGORY               unsigned short
#define TYPE_OF_AMMUNITIONTYPE         unsigned char
#define TYPE_OF_ACTIONPOINTS           unsigned char
#define TYPE_OF_PUNCTUREARMOR          unsigned char

#endif
