#ifndef CNPCTABLE_H
#define CNPCTABLE_H

#include <string>
#include <list>

#include "globals.h"
#include "CWorld.hpp"
#include "CNPC.hpp"
#include "CField.hpp"

class CWorld;

struct NPCStruct {

	position NPCPos; //Pos des NPC
	std::string Name; //Name des NPC
	CCharacter::race_type type; //type des NPC's
	CCharacter::face_to faceto; //richtung des Chars 0 Nord, 2 Ost, 4 Süd, 6 West
	std::vector<struct NPCTalk> speechTexts; //Texte welche der NPC sagen kann;
	unsigned short int walk_range; //Bereich in dem sich der NPC bewegen darf. 0 für keine Bewegung.
	bool ishealer;
};


class CNPCTable {
	public:

		CNPCTable(); //Constructor
		~CNPCTable(); //Destructor

		bool LoadData(); //Function welche die Daten aus der Datenbank lädt liefert false wenn das laden nicht klappt

	private:
        CWorld * _world;
		std::list<struct NPCStruct> NPCList; //List zum aufnehmen aller Werte aus der DB
};

#endif
