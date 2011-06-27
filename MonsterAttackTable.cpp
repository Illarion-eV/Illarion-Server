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
#include "MonsterAttackTable.hpp"
#include <iostream>

CMonsterAttackTable::CMonsterAttackTable() : m_dataOk(false)
{
    reload();
}

CMonsterAttackTable::~CMonsterAttackTable()
{
    clearOldTable();
}

void CMonsterAttackTable::reload()
{
#ifdef CDataConnect_DEBUG
    std::cout<<"Trying to reload MonsterAttackTable!"<<std::endl;
#endif
    
    try
    {
       std::vector<int16_t>raceType; 
       std::vector<uint8_t>attackType; 
       std::vector<int16_t>attackValue;
       std::vector<int16_t>actionPointsLost;
       
       ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
       size_t rows = di::select_all<di::Integer, di::Integer, di::Integer, di::Integer>
       ( transaction, raceType, attackType, attackValue, actionPointsLost,
           "SELECT mat_race_type, mat_attack_type, mat_attack_value, mat_actionpointslost FROM MonsterAttack");
       
       //Zeilenweises laden der Daten
       for ( size_t i = 0; i < rows; ++i )
       {
            AttackBoni data;
            data.attackType = attackType[i];
            data.attackValue = attackValue[i];
            data.actionPointsLost = actionPointsLost[i];
            raceAttackBoni[ raceType[i] ] = data;
       }       
    
    
       m_dataOk = true;
                   
#ifdef CDataConnect_DEBUG
	   std::cout << "loaded " << rows << " rows into MonsterAttackTable" << std::endl;
#endif
    }
    catch ( ... )
    {
        m_dataOk = false;
    }
}

bool CMonsterAttackTable::find(CCharacter::race_type race, AttackBoni &ret)
{
    TABLE::iterator iterat;
    iterat = raceAttackBoni.find( (int16_t)race );
    
    if ( iterat == raceAttackBoni.end() )
        return false;
    else
    {
        ret = (*iterat).second;
        return true;
    }
    return false;
}

void CMonsterAttackTable::clearOldTable()
{
    raceAttackBoni.clear();
}
