#ifndef CNATURALARMORTABLE
#define CNATURALARMORTABLE

#include <string>
#include "CCharacter.hpp"

#if __GNUC__ < 3
#include <hash_map>
#else
#include <ext/hash_map>

#if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
using __gnu_cxx::hash_map;
#endif

#if (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
using std::hash_map;
#endif

#endif

class CNaturalArmorTable
{
    public:
        
        //sucht einen Eintrag aus der Tabelle nach Rassentyp
        //liefert true oder false je nachdem ob ein eintrag gefunden wurde.
        bool find(CCharacter::race_type race, MonsterArmor &ret);

        //Constructor        
        CNaturalArmorTable();
        
        //Destructor
        ~CNaturalArmorTable();
        
        //Lädt die Tabelle neu
        void reload();
        
        //liefert den Wert der Privat Variable m_dataOK zurück
        inline bool isDataOk(){ return m_dataOK; }
        
     protected:
     
        //Tabelle zum aufnehmen der eigentlichen Daten
        typedef hash_map<uint16_t, MonsterArmor> TABLE;
        TABLE m_ArmorTable;
        
        //Bool Wert der enthält ob das Laden der Daten geklappt hat bzw alles IO mit der Tabelle ist
        bool m_dataOK;
        
        //leer die alte Tabelle (löscht deren Inhalt)
        void clearOldTable();
        
};
#endif
