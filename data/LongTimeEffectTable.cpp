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


#include "db/ConnectionManager.hpp"
#include "LongTimeEffectTable.hpp"
#include "TableStructs.hpp"
#include "script/LuaLongTimeEffectScript.hpp"
#include "Logger.hpp"

LongTimeEffectTable::LongTimeEffectTable() : m_dataOK(false)
{
    reload();
}

void LongTimeEffectTable::reload()
{
#ifdef DataConnect_DEBUG
	std::cout << "LongTimeEffectTable: reload" << std::endl;
#endif    
    try
    {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
        std::vector<uint16_t> effectid;
        std::vector<std::string> effectname;
        std::vector<std::string> scriptname;
        
        size_t rows = di::select_all<di::Integer, di::Varchar, di::Varchar>
        (transaction, effectid, effectname, scriptname, "SELECT lte_effectid, lte_effectname, lte_scriptname FROM longtimeeffects");
        
        if ( rows > 0 )
        {
            clearOldTable();
            LongTimeEffectStruct temp;
            for ( size_t i = 0; i < rows; ++i)
            {
                temp.effectid = effectid[i];
                temp.effectname = effectname[i];
                temp.scriptname = scriptname[i];
                if ( scriptname[i] != "" )
                {
                    try
                    {
                        boost::shared_ptr<LuaLongTimeEffectScript> script(new LuaLongTimeEffectScript( scriptname[i], temp ) );
                        temp.script = script;
                    }
                    catch( ScriptException &e )
                    {
                        Logger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
                    }
                }
                m_table[ effectid[i] ] = temp;
            }
            m_dataOK = true;
        }
        else
            m_dataOK = true;
#ifdef DataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into LongTimeEffectTable" << std::endl;
#endif
    }
    catch( std::exception &e )
    {
        std::cerr<<"exception in LongTimeEffect loading: " << e.what() << std::endl;
        m_dataOK = false;
    }
}

bool LongTimeEffectTable::find( uint16_t effectId, LongTimeEffectStruct &ret)
{
    TABLE::iterator it = m_table.find( effectId );
    if ( it != m_table.end() )
    {
        ret = it->second;
        return true;
    }
    else
        return false;
}

bool LongTimeEffectTable::find( std::string effectname, LongTimeEffectStruct &ret)
{
    for ( TABLE::iterator it = m_table.begin(); it != m_table.end(); ++it )
    {
        if ( it->second.scriptname == effectname )
        {
            ret = it->second;
            return true;
        }
    }
    return false;
}

void LongTimeEffectTable::clearOldTable()
{
    m_table.clear();
}

LongTimeEffectTable::~LongTimeEffectTable()
{
    clearOldTable();
}
