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


#include "ScriptVariablesTable.hpp"
#include "db/ConnectionManager.hpp"
#include <boost/lexical_cast.hpp>

ScriptVariablesTable::ScriptVariablesTable() : m_dataOK( false )
{
    reload();
}

ScriptVariablesTable::~ScriptVariablesTable()
{
    clearOldTable();
}

void ScriptVariablesTable::clearOldTable()
{
    values_table.clear();
}

void ScriptVariablesTable::reload()
{
	try 
    {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
        //Loading strings
		std::vector<std::string> id;
		std::vector<std::string> sv_strings;
        size_t rows = di::select_all<
        di::Varchar, di::Varchar>(transaction, id, sv_strings,"SELECT svt_ids, svt_string FROM scriptvariables");
 		if (rows > 0) 
        {
            values_table.clear();            
			for (size_t i = 0; i < rows; ++i) 
            {
                char * vname = new char[ id[i].length() + 1 ];
                strcpy( vname, id[i].c_str() );
                vname[ id[i].length() ] = 0;
				values_table[ vname ] = sv_strings[i];
			}
		}
        m_dataOK = true;
        
#ifdef DataConnect_DEBUG
		std::cout << "loaded " << rows << " values for scripts" << std::endl;
#endif   
         return;
	} 
    catch ( std::exception &e ) 
    {
        std::cerr<<"catched error as reading scriptvariables: "<<e.what()<<std::endl;
		m_dataOK = false;
	}    
}

bool ScriptVariablesTable::find(std::string id, std::string &ret)
{
    STRINGTABLE::iterator it;
    it = values_table.find( id.c_str() );
    if ( it != values_table.end() )
    {
        ret = it->second;
        return true;
    }
    else
    {
        ret = "";
        return false;
    }
}

void ScriptVariablesTable::set( std::string id, luabind::object o)
{
    char * vname = new char[ id.length() + 1 ];
    strcpy( vname, id.c_str() );
    vname[ id.length() ] = 0;
    std::string str = boost::lexical_cast<std::string>(o);
    values_table[ vname ] = str;
}

bool ScriptVariablesTable::remove( std::string id )
{
    STRINGTABLE::iterator it = values_table.find( id.c_str() );
    if ( it != values_table.end() )
    {
        values_table.erase( it );
        return true;
    }
    else
    {
        return false;
    }
}

void ScriptVariablesTable::save()
{
    //Laden einer Transaktion
    ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
    try
    {
        //Deleting old values
        std::stringstream query;
        query << "DELETE FROM scriptvariables";
        di::exec(transaction, query.str());
        //Inserting new ones
        for ( STRINGTABLE::iterator it = values_table.begin(); it != values_table.end(); ++it )
        {
            std::string id = std::string( it->first );
            di::insert(transaction, id, it->second, "INSERT INTO scriptvariables (svt_ids, svt_string)");
        }
        transaction.commit();
    }
    catch ( std::exception &e )
    {
        std::cerr<<"exception: "<<e.what()<<" while saving Scriptvariables!"<<std::endl;
        transaction.rollback();
    }
}
    
    
