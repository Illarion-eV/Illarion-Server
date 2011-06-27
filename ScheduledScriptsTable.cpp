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
#include "ScheduledScriptsTable.hpp"
#include <iostream>
#include "Logger.hpp"
#include "Random.hpp"

CScheduledScriptsTable::CScheduledScriptsTable() : currentCycle(0), m_dataOk(false)
{
    reload();
}

CScheduledScriptsTable::~CScheduledScriptsTable()
{
    /**@todo destructor implementatiotn*/
    m_table.clear();
}

bool CScheduledScriptsTable::nextCycle()
{
    currentCycle++;
    ScriptData data; /**< holds the current task*/
    int emexit = 0; /**< emergency counter which breaks the loop if to much execution*/ 
    while ( !m_table.empty() && (emexit < 200) && ( m_table.front().nextCycleTime <= currentCycle) )
    {
        emexit++;
        if ( emexit>=200 ) break; /**< emergency exit so we can't create an endless loop */
        
        if ( !m_table.empty() )
        {
            data = m_table.front(); /**< copy the first data in data*/
            m_table.pop_front(); /**< deletes the first task */
            if ( data.scriptptr ) 
            {
                /**calculate the next time where the script is invoked */
                data.nextCycleTime = currentCycle + rnd(data.minCycleTime,data.maxCycleTime);
            
                /**call the script function */
                data.scriptptr->callFunction(data.functionName,currentCycle,data.lastCycleTime,data.nextCycleTime);
            
                data.lastCycleTime = currentCycle; /**< script was runned and so we can change the lastCycleTime*/
            
                addData(data); /**< insert the script in the list again*/
                return true;
            }
        }
    }
    return false;
}

bool CScheduledScriptsTable::addData(ScriptData data)
{
    CLogger::writeMessage("schedscripts","insert new Task task.nextCycle: " + CLogger::toString(data.nextCycleTime)  + " current Cycle: " + CLogger::toString(currentCycle));
    std::list<ScriptData>::iterator it;
    bool inserted = false;
    if ( data.nextCycleTime <= currentCycle )
        data.nextCycleTime = currentCycle + 1;
    for ( it = m_table.begin(); it != m_table.end(); ++it)
    {
        if ( (*it).nextCycleTime > data.nextCycleTime )
        {
            m_table.insert( it, data);
            inserted = true;
            break;
        }
    }
    if (!inserted)
        m_table.push_back(data);
    return true;
}

void CScheduledScriptsTable::reload()
{
    try
    {
        ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
        
        std::vector<std::string> scriptname;        
        di::isnull_vector<std::vector<std::string> > n_scriptname(scriptname);

        size_t rows = di::select_all<di::Varchar>(transaction, n_scriptname, "SELECT DISTINCT sc_scriptname FROM scheduledscripts");
        for ( size_t i = 0; i < rows; ++i)
        {
            if (!n_scriptname.var[i] )
            {
                ScriptData tmpRecord;
                std::vector<uint32_t> min_cycle_time;
                std::vector<uint32_t> max_cycle_time;
                std::vector<std::string> functionname;
                di::isnull_vector<std::vector<std::string> > n_functionname(functionname);
                size_t rows2 = di::select_all<di::Integer, di::Integer,di::Varchar>(transaction,min_cycle_time, max_cycle_time, n_functionname, "SELECT sc_mincycletime, sc_maxcycletime, sc_functionname FROM scheduledscripts WHERE sc_scriptname = '" + scriptname[i] + "'");
                try
                {
                    boost::shared_ptr<CLuaScheduledScript> script(new CLuaScheduledScript( scriptname[i] ) );
                    for ( size_t j = 0; j < rows2; ++j)
                    {
                        tmpRecord.minCycleTime = min_cycle_time[j];
                        tmpRecord.maxCycleTime = max_cycle_time[j];
                        tmpRecord.nextCycleTime = rnd(currentCycle + min_cycle_time[j], currentCycle + max_cycle_time[j] );
                        tmpRecord.functionName = functionname[j];
                        tmpRecord.scriptName = scriptname[i];
                        tmpRecord.scriptptr = script;
                        addData(tmpRecord);
                    }
                }
                catch (ScriptException &e)
                {
                    CLogger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
                }
            }
        }
        m_dataOk = true;
    }
    catch ( std::exception e )
    {
        std::cerr << "exception: " << e.what() << std::endl;
        m_dataOk = false;
    }
}
