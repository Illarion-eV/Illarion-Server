#include "db/ConnectionManager.h"
#include "CTriggerTable.hpp"
#include "CLogger.hpp"

template<class from>
const std::string toString(const from& convert) {
	std::stringstream stream;
	stream << convert;
	return stream.str();
}


CTriggerTable::CTriggerTable():  _dataOK(false) {
	reload();
}

void CTriggerTable::reload() {
#ifdef CDataConnect_DEBUG
	std::cout<<"CTriggerTable: reload!" <<std::endl;
#endif
	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
		std::vector<int16_t> posx,posy,posz;
		std::vector<std::string> scriptname;
		di::isnull_vector<std::vector<std::string> > n_scriptname(scriptname);
		size_t rows = di::select_all<di::Integer, di::Integer, di::Integer, di::Varchar>(transaction,posx,posy,posz,n_scriptname,
					  "SELECT tgf_posx, tgf_posy, tgf_posz, tgf_script FROM triggerfields");
		for ( size_t i = 0; i < rows; ++i) {
			TriggerStruct Trigger; //new Trigger
			Trigger.pos = position(posx[i],posy[i],posz[i]);
			if (!n_scriptname.var[i]) {
				try {
					// we got a script... load it
					boost::shared_ptr<CLuaTriggerScript> script( new CLuaTriggerScript( scriptname[i], Trigger.pos ) );
					Trigger.script = script;
				} catch (ScriptException &e) {
                    CLogger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
				}
			}
			Triggers.insert(std::pair<position, TriggerStruct>(Trigger.pos,Trigger)); //Zuweisen des Spells
		}
		std::cout << " loadet " << rows << " Triggerfields! " << std::endl;
		_dataOK = true;
	} catch (std::exception &e) {

		std::cerr << "exception: " << e.what() << std::endl;
		_dataOK = false;
	}
}

bool CTriggerTable::find(position pos, TriggerStruct &data) {
	TriggerMap::iterator iterator;
	iterator = Triggers.find(pos);
	if ( iterator == Triggers.end() ) {
		return false;
	} else {
		data = (*iterator).second;
		return true;
	}

}

void CTriggerTable::clearOldTable() {
	Triggers.clear();
}

CTriggerTable::~CTriggerTable() {
	clearOldTable();
}

