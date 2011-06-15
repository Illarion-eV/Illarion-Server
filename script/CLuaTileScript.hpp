#ifndef _CLUATILESCRIPT_HPP
#define _CLUATILESCRIPT_HPP

#include "CLuaScript.hpp"
#include "TableStructs.hpp"
#include "globals.h"
#include "Item.hpp"

class CWorld;
class CCharacter;

class CLuaTileScript : public CLuaScript {
	public:
		CLuaTileScript(std::string filename,TilesStruct tile) throw(ScriptException);
		virtual ~CLuaTileScript() throw();

		//Character uses an Tile
		//\param user: the Character which is using the tile
		//\param pos: the Position at which the tile is used
		void useTile(CCharacter * user, position pos, unsigned short int counter, unsigned int param, unsigned char ltastate);

		//Character uses a tile with a item
		//\param user: The Character which is using the tile
		//\param pos: the position at which the tile is used
		//\param item: the ScriptItem whith which the field is used
		void useTileWithItem(CCharacter * user, position pos, ScriptItem item, unsigned short int counter, unsigned int param, unsigned char ltastate);

		//Character uses a tile with another tile
		//\param user: the Character which is using the tile
		//\param pos: the position at which the tile is used
		//\param posnew: the other position
		void useTileWithField(CCharacter * user, position pos, position posnew, unsigned short int counter, unsigned int param, unsigned char ltastate);

		//Character uses a tile with another character
		//\param user: the Character which is using the tile
		//\param pos: the position at which the tile is used
		//\param targetchar: the character whith which the field is used.
		void useTileWithCharacter(CCharacter * user, position pos, CCharacter * character, unsigned short int counter, unsigned int param, unsigned char ltastate);
        
        /**
        *a longtime action is disturbed by another person
        */
        bool actionDisturbed(CCharacter * performer, CCharacter * disturber);

	private:

		CLuaTileScript(const CLuaTileScript&);
		CLuaTileScript& operator=(const CLuaTileScript&);
		TilesStruct thisTile;
		void init_functions();
};
#endif
