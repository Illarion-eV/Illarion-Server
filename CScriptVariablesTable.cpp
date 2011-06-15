#include "CScriptVariablesTable.hpp"
#include "db/ConnectionManager.h"
#include <boost/lexical_cast.hpp>

CScriptVariablesTable::CScriptVariablesTable() : m_dataOK( false )
{
    reload();
}

CScriptVariablesTable::~CScriptVariablesTable()
{
    clearOldTable();
}

void CScriptVariablesTable::clearOldTable()
{
    values_table.clear();
}

void CScriptVariablesTable::reload()
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
        
#ifdef CDataConnect_DEBUG
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

bool CScriptVariablesTable::find(std::string id, std::string &ret)
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

void CScriptVariablesTable::set( std::string id, luabind::object o)
{
    char * vname = new char[ id.length() + 1 ];
    strcpy( vname, id.c_str() );
    vname[ id.length() ] = 0;
    std::string str = boost::lexical_cast<std::string>(o);
    values_table[ vname ] = str;
}

bool CScriptVariablesTable::remove( std::string id )
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

void CScriptVariablesTable::save()
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
    
    
