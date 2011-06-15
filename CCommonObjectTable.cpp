#include "db/ConnectionManager.h"
#include "CCommonObjectTable.h"
#include <iostream>
#include "CWorld.hpp"
#include "script/CLuaItemScript.hpp"
#include <boost/shared_ptr.hpp>
#include "CLogger.hpp"

/*template<class from>
const std::string toString(const from& convert) {
	std::stringstream stream;
	stream << convert;
	return stream.str();
}*/

CCommonObjectTable::CCommonObjectTable() : m_dataOK(false) 
{
	reload();
}

TYPE_OF_ITEM_ID CCommonObjectTable::calcInfiniteRot( TYPE_OF_ITEM_ID id, std::map<TYPE_OF_ITEM_ID, bool> & visited, std::map<TYPE_OF_ITEM_ID, bool> & assigned )
{
    if (visited[ id ])
    {
        if (assigned[ id ]) return m_table[ id ].AfterInfiniteRot;
        return 0;
    }
    visited[ id ] = true;
    if (id == m_table[ id ].ObjectAfterRot)
    {
        if (m_table[ id ].AgeingSpeed >= 255)
        {
            m_table[ id ].AfterInfiniteRot = id;
            assigned[ id ] = true;
            return id;
        }
        m_table[ id ].AfterInfiniteRot = 0;
        assigned[ id ] = true;
        return 0;
    }
    m_table[ id ].AfterInfiniteRot = calcInfiniteRot( m_table[ id ].ObjectAfterRot, visited, assigned );
    assigned[ id ] = true;
    return m_table[ id ].AfterInfiniteRot;
}

void CCommonObjectTable::reload() {

#ifdef CDataConnect_DEBUG
	std::cout << "CCommonObjectTable: reload" << std::endl;
#endif

	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

		std::vector<TYPE_OF_ITEM_ID> ids;
		std::vector<TYPE_OF_WEIGHT> weights;
		std::vector<TYPE_OF_VOLUME> volumes;
		std::vector<TYPE_OF_AGEINGSPEED> agingspeeds;
		std::vector<TYPE_OF_ITEM_ID> objectafterrot;
		std::vector<bool> isStackable;
        std::vector<bool> rotsininventory;
        std::vector<TYPE_OF_BRIGHTNESS> brightnesses;
        std::vector<TYPE_OF_WORTH> worth;
		std::vector<std::string> scriptname;
		di::isnull_vector<std::vector<std::string> > n_scriptname(scriptname);

        std::map<TYPE_OF_ITEM_ID, bool> assigned; // map item id to whether an infinite rot has been assigned
        std::map<TYPE_OF_ITEM_ID, bool> visited;  // map item id to whether it has been visited already in calcInfiniteRot

		size_t rows = di::select_all<
                        di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Varchar, di::Boolean, di::Boolean, di::Integer, di::Integer
					  >(transaction, ids, volumes, weights, agingspeeds, objectafterrot, n_scriptname, isStackable, rotsininventory, brightnesses, worth, 
						"SELECT com_itemid, com_volume, com_weight, com_agingspeed, com_objectafterrot, com_script, com_stackable, com_rotsininventory, com_brightness, com_worth FROM common");
		if (rows > 0) {
			clearOldTable();
			for (size_t i = 0; i < rows; ++i) 
            {
				CommonStruct temprecord;
				temprecord.id = ids[i];
				temprecord.Weight = weights[i];
				temprecord.Volume = volumes[i];
				temprecord.AgeingSpeed = agingspeeds[i];
				temprecord.ObjectAfterRot = objectafterrot[i];
				temprecord.isStackable = isStackable[i];
                temprecord.rotsInInventory = rotsininventory[i];
                temprecord.Brightness = brightnesses[i];
                temprecord.Worth = worth[i];
				if (!n_scriptname.var[i]) {
					try {
						boost::shared_ptr<CLuaItemScript> tmpScript(new CLuaItemScript( scriptname[i] , temprecord));
                        m_scripttable[ ids[i] ] = tmpScript;
					} catch(ScriptException &e) {
                        CLogger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
					}
				}
				m_table[ ids[i] ] = temprecord;
                visited.insert( std::pair<TYPE_OF_ITEM_ID, bool>(ids[i],false) );
                assigned.insert( std::pair<TYPE_OF_ITEM_ID, bool>(ids[i],false) );
			}

            // calculate infinite rot for map export
            for (size_t i = 0; i < rows; ++i)
            {
                if (!assigned[ ids[i] ])
                    calcInfiniteRot( ids[i], visited, assigned );
            }

			m_dataOK = true;
		} else m_dataOK = false;


#ifdef CDataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into CCommonObjectTable" << std::endl;
#endif

	} catch (std::exception e) {
		std::cerr << "exception: " << e.what() << std::endl;
		m_dataOK = false;
	}

}

bool CCommonObjectTable::find( TYPE_OF_ITEM_ID Id, CommonStruct &ret ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		ret = ( *iterator ).second;
		return true;
	}
}

boost::shared_ptr<CLuaItemScript> CCommonObjectTable::findScript( TYPE_OF_ITEM_ID Id)
{
    SCRIPTTABLE::iterator iterator;
	iterator = m_scripttable.find( Id );

	if ( iterator != m_scripttable.end() ) 
    {
		return iterator->second;
	} 
    else 
    {
		return boost::shared_ptr<CLuaItemScript>();
	}    
}

bool CCommonObjectTable::find( TYPE_OF_ITEM_ID Id) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		return true;
	}
}



void CCommonObjectTable::clearOldTable() {
	m_table.clear();
}


CCommonObjectTable::~CCommonObjectTable() {
	clearOldTable();
}


