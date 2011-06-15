#ifndef USER_MENU_HPP
#define USER_MENU_HPP


/*
extern "C" 
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
 
#include <luabind/object.hpp>
*/
#include <list>
#include <stdarg.h>
#include "types.h"

/**
*@ingroup Scriptclasses
* holds the list of itemid which are shown in a menu
*/
struct UserMenuStruct 
{
	std::list<TYPE_OF_ITEM_ID>Items;
    
    /**
    *@name UserMenuStruct Lua functions:
    *functions which are exported to lua like enums
    */
        
    //@{
    /**
    *======================start grouping Lua functions===================
    *@ingroup Scriptfunctions
    */    
    
    /**
    * Standard Constructor
    * <b>Lua: UserMenuStruct()</b>
    */
	UserMenuStruct() 
    {
		Items.clear();
	}

    /**
    * adds an itemid to the list
    * @param Item the id of the item which is added
    */
    void AddItem(TYPE_OF_ITEM_ID Item) 
    {
		Items.push_back(Item);
	}
    
    /**
    *=====================End of grouping Lua functions=======================
    */
    //@}
    
	~UserMenuStruct() 
    {
		Items.clear();
	}


	/*
	void AddItems( luabind::object list )
	{
	    if ( list.isvalid() )
	    {
	          if ( list.type() == luabind::LUA_LIST )
	          {
	                for (luabind::object::iterator it = liste.begin(); it != liste.end(); ++it) 
	                {
	                     TYPE_OF_ITEM_ID Item = luabind::object_cast<TYPE_OF_ITEM_ID>(*it);
	                     Items.push_back(Item);
	                }  
	          }        
	    }
	}*/
};
#endif
