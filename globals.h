#ifndef globals_HH
#define globals_HH

#include <map>
#include <fstream>
#include <iostream>

#include "types.h"

/**
* @ingroup Scriptclasses
* defines a 3d coordinate in the area
* exportet to lua as <b>position</b>
*/
struct position 
{
    /**
    *@name position Lua Variables:
    * to lua exportet variables
    */
    
    //@{
    /**
    *=============================start group Lua Variables===================
    *@ingroup Scriptvariables
    */
        
    /** 
    * x-coordinate of the position
    * <b>Lua: (r/w) [x]</b>
    */
	short int x;
    
    /** 
    * y-coordinate of the position
    * <b>Lua: (r/w) [y]</b>
    */    
	short int y; 
    
    /**
    * z-coordinate of the position
    * <b>Lua: (r/w) [z]</b>
    */    
	short int z;

    /**
    *=============================end group Lua Variables===================
    */
    //@}
    
    
    /**
    * @name position Lua Functions:
    * to Lua exported Functions
    */
    
    //@{
    /**
    *=============================start group Lua functions===================
    *@ingroup Scriptfunctions
    */        
        
    /** 
    * standard constructor for creating a new position
    * @param _x starting x position of the coordinate
    * @param _y starting y position of the coordinate
    * @param _z starting z position of the coordinate
    */
	position(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
    
    /** 
    * standard constructor which creates a position and initializes x,y,z with 0
    */
	position() : x(0), y(0), z(0) {}
    
    /**
    *=============================end Lua Variables===================
    */
    
    //@}
    
    
    /**
    * overloaded == operator which determines if one position is equal to another one
    */
	bool operator == (const position pos) 
    {
		return ( x == pos.x && y == pos.y && z == pos.z );
	}
};


/**
* determines a 2d Postion on one single map
* the z coordinate is the map itself
*/
typedef struct 
{
	short int x; /**< x coordinate*/
	short int y; /**< y coordinate*/
}
MAP_POSITION;

// a map storing configuration options from a config file...
extern std::map<std::string, std::string> configOptions;

//! a log file for kills
extern std::ofstream kill_log;

#endif
