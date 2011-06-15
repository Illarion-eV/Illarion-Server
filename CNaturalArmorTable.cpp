#include "db/ConnectionManager.h"
#include "CNaturalArmorTable.hpp"
#include <iostream>

CNaturalArmorTable::CNaturalArmorTable() : m_dataOK(false)
{
    reload();
}

CNaturalArmorTable::~CNaturalArmorTable()
{
    clearOldTable();
}

void CNaturalArmorTable::reload()
{
#ifdef CDataConnect_DEBUG
    std::cout<<"Trying to reload NaturalArmorTable!"<<std::endl;
#endif

    try
    {
        //LAden einer Connection    
        ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
        
        std::vector<uint16_t> raceTyp;
        std::vector<int16_t> strokeArmor;
        std::vector<int16_t> punctureArmor;
        std::vector<int16_t> thrustArmor;
        
        //Laden der Daten in die Vectoren
        size_t rows = di::select_all<di::Integer, di::Integer, di::Integer, di::Integer>
        (transaction, raceTyp, strokeArmor, punctureArmor, thrustArmor, "SELECT nar_race, nar_strokeArmor, nar_punctureArmor, nar_thrustArmor FROM NaturalArmor");
        
        //Löschen der alten Daten
        clearOldTable();
        
        //Schleife durch Tuples        
        for ( size_t i = 0; i < rows; ++i )
        {
             MonsterArmor Armor;
             Armor.strokeArmor = strokeArmor[i];
             Armor.punctureArmor = punctureArmor[i];
             Armor.thrustArmor = thrustArmor[i];        
             m_ArmorTable[ raceTyp[i] ] = Armor;          
        }   
        
        m_dataOK = true;
           
#ifdef CDataConnect_DEBUG
	std::cout << "loaded " << rows << " rows into CNaturalArmorTable" << std::endl;
#endif         
    }
    catch ( ... )
    {
        m_dataOK = false;
    }

}

bool CNaturalArmorTable::find( CCharacter::race_type race, MonsterArmor &ret)
{
    TABLE::iterator iterat;
    iterat = m_ArmorTable.find(static_cast<uint16_t>(race) );
    
    if ( iterat == m_ArmorTable.end() )
        return false;
    else
    {
        ret = (*iterat).second;
        return true;
    }
    return false;
}

void CNaturalArmorTable::clearOldTable()
{
    m_ArmorTable.clear();
}


