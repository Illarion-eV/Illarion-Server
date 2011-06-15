#include "db/ConnectionManager.h"
#include "CLongTimeEffectTable.hpp"
#include "TableStructs.hpp"
#include "script/CLuaLongTimeEffectScript.hpp"
#include "CLogger.hpp"

CLongTimeEffectTable::CLongTimeEffectTable() : m_dataOK(false)
{
    reload();
}

void CLongTimeEffectTable::reload()
{
#ifdef CDataConnect_DEBUG
	std::cout << "CLongTimeEffectTable: reload" << std::endl;
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
                        boost::shared_ptr<CLuaLongTimeEffectScript> script(new CLuaLongTimeEffectScript( scriptname[i], temp ) );
                        temp.script = script;
                    }
                    catch( ScriptException &e )
                    {
                        CLogger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
                    }
                }
                m_table[ effectid[i] ] = temp;
            }
            m_dataOK = true;
        }
        else
            m_dataOK = true;
#ifdef CDataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into CLongTimeEffectTable" << std::endl;
#endif
    }
    catch( std::exception e )
    {
        std::cerr<<"exception in LongTimeEffect loading: " << e.what() << std::endl;
        m_dataOK = false;
    }
}

bool CLongTimeEffectTable::find( uint16_t effectId, LongTimeEffectStruct &ret)
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

bool CLongTimeEffectTable::find( std::string effectname, LongTimeEffectStruct &ret)
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

void CLongTimeEffectTable::clearOldTable()
{
    m_table.clear();
}

CLongTimeEffectTable::~CLongTimeEffectTable()
{
    clearOldTable();
}
