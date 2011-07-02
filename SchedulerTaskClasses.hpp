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


#ifndef SCHEDULERTASKCLASSES
#define SCHEDULEDTASKCLASSES

#include "Scheduler.hpp"
#include "types.hpp"
#include "TableStructs.hpp"
#include "data/CommonObjectTable.hpp"
#include "script/LuaLearnScript.hpp"

extern CommonObjectTable *CommonItems;
extern boost::shared_ptr<LuaLearnScript>learnScript;

class STalk : public SchedulerObject {

public:

    STalk(TYPE_OF_CHARACTER_ID id, std::string ntext, unsigned short int ncount, unsigned long int ncycles, unsigned long int cycle_time) {
        playerid = id;
        text = ntext;
        CycleTime = cycle_time;
        nextCycle = ncycles;
        count = ncount;
    }
    virtual ~STalk() {}

    bool operator()(World *world) {
        ccharactervector < Player * >::iterator playerIterator;

        for (playerIterator = world->Players.begin(); playerIterator < world->Players.end(); ++playerIterator) {
            if ((*playerIterator)->id == playerid) {
                //spieler noch online
                (*playerIterator)->sendMessage(text);
            }
        }

        count--;

        if (count > 0) {
            nextCycle += CycleTime;
            return true;
        } else {
            return false;
        }
    }

private:

    TYPE_OF_CHARACTER_ID playerid;
    std::string text;

};

//====================================================================================================
class SIncreaseHealtPoints : public SchedulerObject {

public:

    SIncreaseHealtPoints(TYPE_OF_CHARACTER_ID id, int nvalue, unsigned short int ncount, unsigned long int first_cycle, unsigned long int cycle_time) {
        cid = id; //Characterid für spätere Überprüfung
        value = nvalue;
        CycleTime = cycle_time;
        nextCycle = first_cycle;
        count = ncount;
    }

    virtual ~SIncreaseHealtPoints() {}

    bool operator()(World *world) {
        Player *cp=NULL;
        ccharactervector < Player * >::iterator playerIterator;

        for (playerIterator = world->Players.begin(); playerIterator < world->Players.end(); ++playerIterator) {
            if ((*playerIterator)->id == cid) {
                cp = (*playerIterator); //Spieler gefunden
                break;
            }
        }

        if (cp!=NULL) {
            if (cp->battrib.hitpoints + value >= MAXHPS) {
                //Wenn Heilung mehr als max heilen würde dann max heilen und Task entfernen
                cp->battrib.hitpoints = MAXHPS;
                //std::cout<<"Zeiger nicht mehr gültig Task gelöscht \n";
                return false;
            } else if (!cp->IsAlive()) {
                return false;    //Wenn Char Tod ist, Task entfernen
            } else {
                //std::cout<<"HP erhöhen um : "<<value<<" Cycle: "<<nextCycle<<" nächste Ausführung: ";
                cp->increaseAttrib("hitpoints",value); //Hp erhöhen
                nextCycle += CycleTime ; //neuen Ausführungszylkus berechnen
                --count; //Ausführung um eins dezimieren
                //std::cout<<nextCycle<<" Count: "<<count<<"\n";
                if (count <= 0) {
                    return false; //Wenn langzeiteffekt zuende nicht wieder einfügen
                } else {
                    return true; //Wenn noch ausführungen offen sind Task wieder einfügen
                }

            }
        } else {
            return false; //spieler nicht mehr online Task löschen
        }

        //Client updaten
        //cp->sendAttrib("hitpoints",cp->increaseAttrib("hitpoints", 0 ) );

    }

private:

    TYPE_OF_CHARACTER_ID cid;
    int value;

};
//================================================================================================================
class SIncreaseManaPoints : public SchedulerObject {

public:

    SIncreaseManaPoints(TYPE_OF_CHARACTER_ID id, int nvalue, unsigned short int ncount, unsigned long int first_cycle, unsigned long int cycle_time) {
        cid = id; //Characterid für spätere Überprüfung
        value = nvalue;
        CycleTime = cycle_time;
        nextCycle = first_cycle;
        count = ncount;
    }
    virtual ~SIncreaseManaPoints() {}

    bool operator()(World *world) {
        ccharactervector < Player * >::iterator playerIterator;
        Player *cp=NULL;

        for (playerIterator = world->Players.begin(); playerIterator < world->Players.end(); ++playerIterator) {
            if ((*playerIterator)->id == cid) {
                cp = (*playerIterator); //Spieler gefunden
                break;
            }
        }

        if (cp !=NULL) {
            if (cp->battrib.mana + value >= MAXMANA) {
                //Wenn erhöhung höher wäre als Maxmana dann maxmana und Task entfernen
                cp->battrib.mana = MAXMANA;
                return false;
            } else if (!cp->IsAlive()) {
                return false;    //Wenn Char Tod ist, Task entfernen
            } else {
                cp->increaseAttrib("mana",value); //Mana erhöhen
                nextCycle += CycleTime ; //neuen Ausführungszylkus berechnen
                --count; //Ausführung um eins dezimieren

                if (count <= 0) {
                    return false;    //Wenn langzeiteffekt zuende nicht wieder einfügen
                } else {
                    return true;    //Wenn noch ausführungen offen sind Task wieder einfügen
                }


            }
        } else {
            return false; //spieler hat ausgeloggt Task entfernen
        }

        //Client updaten
        cp->sendAttrib("mana",cp->increaseAttrib("mana", 0));
    }

private:

    TYPE_OF_CHARACTER_ID cid;
    Player *ch;
    int value;

};

//=========================================================================================
class SGlobalPoison : public SchedulerObject {

public:

    SGlobalPoison(unsigned long int first_cycle) {
        nextCycle = first_cycle;
    }
    virtual ~SGlobalPoison() {}

    bool operator()(World *world) {
        bool alivebefore=true;
        ccharactervector < Monster * >::iterator monsterIterator;

        for (monsterIterator = world->Monsters.begin(); monsterIterator < world->Monsters.end(); ++monsterIterator) {
            if ((*monsterIterator)->getPoisonValue() > 0) {   //Prüfen ob aktueller Spieler vergiftet ist
                alivebefore = (*monsterIterator)->IsAlive();
                short int decreasevalue = 1 + ((*monsterIterator)->increaseAttrib("willpower",0) / 20) *  rnd(0,5);
                (*monsterIterator)->increasePoisonValue(-decreasevalue);   //Giftwert absenken
                short int schaden = (*monsterIterator)->getPoisonValue() * (80 + rnd(0,40))/(7*(*monsterIterator)->increaseAttrib("constitution",0));
                (*monsterIterator)->increaseAttrib("hitpoints", -schaden);     //Giftwert * 2 an schaden machen

                if (!(*monsterIterator)->IsAlive()) {    //Wenn Monster nicht mehr lebt
                    (*monsterIterator)->setPoisonValue(0);   //Vergiftung auf 0 setzen

                    if (alivebefore != (*monsterIterator)->IsAlive()) {   //Wenn er vorher noch gelebt hat
                        //Beim Tod durch gift einen Spin anzeigen (Wolke)
                        world->sendSpinToAllVisiblePlayers((*monsterIterator));

                        if (alivebefore) {
                            //Killtime und grund ins Log schreiben
                            time_t acttime = time(NULL);
                            std::string killtime = ctime(&acttime);
                            killtime[killtime.size()-1] = ':';
                            kill_log << killtime << " ";
                            kill_log << " Monster " << (*monsterIterator)->name << "(" << (*monsterIterator)->id << ") killed by Poison effect \n";
                        }
                    }
                }
            }
        }

        ccharactervector < Player * >::iterator playerIterator;

        for (playerIterator = world->Players.begin(); playerIterator < world->Players.end(); ++playerIterator) {
            if ((*playerIterator)->getPoisonValue() > 0) {   //Prüfen ob aktueller Spieler vergiftet ist
                alivebefore = (*playerIterator)->IsAlive();
                short int decreasevalue = 1 + ((*playerIterator)->increaseAttrib("willpower",0) / 20) *  rnd(0,5);
                (*playerIterator)->increasePoisonValue(-decreasevalue);   //Giftwert absenken
                short int schaden = (*playerIterator)->getPoisonValue() * (80 + rnd(0,40))/(7*(*playerIterator)->increaseAttrib("constitution",0));
                (*playerIterator)->increaseAttrib("hitpoints", -schaden);     //Giftwert * 2 an schaden machen

                if (!(*playerIterator)->IsAlive()) {    //Wenn Spieler nicht mehr lebt
                    (*playerIterator)->setPoisonValue(0);   //Vergiftung auf 0 setzen

                    if (alivebefore != (*playerIterator)->IsAlive()) {   //Wenn er vorher noch gelebt hat
                        //Beim Tod durch gift einen Spin anzeigen (Wolke)
                        world->sendSpinToAllVisiblePlayers((*playerIterator));

                        if (alivebefore) {
                            //Killtime und grund ins Log schreiben
                            time_t acttime = time(NULL);
                            std::string killtime = ctime(&acttime);
                            killtime[killtime.size()-1] = ':';
                            kill_log << killtime << " ";
                            kill_log << " Player " << (*playerIterator)->name << "(" << (*playerIterator)->id << ") killed by Poison effect \n";
                        }
                    }
                }
            }
        }

        /* Giftschaden bei NPC's deaktiviert
        ccharactervector < NPC* >::iterator npcIterator;
        for ( npcIterator = world->Npc.begin(); npcIterator < world->Npc.end(); ++npcIterator )
        {
              //ToDo- Schleife einfügen um Vergifteten Spielern Schaden zu machen
        }
        */
        nextCycle += 2; //Neue Zykluszeit berechnen (aller 2 Zyklen);
        return true; //Task erneut einfügen
    }

};


class SGlobalPlayerLearnrate : public SchedulerObject {

public:

    SGlobalPlayerLearnrate(unsigned long int first_cycle) {
        nextCycle = first_cycle;
    }
    virtual ~SGlobalPlayerLearnrate() {}

    bool operator()(World *world) {
        ccharactervector < Player * >::iterator playerIterator;

        for (playerIterator = world->Players.begin(); playerIterator < world->Players.end(); ++playerIterator) {
            if ((*playerIterator)->getMentalCapacity() > 0) {   //Prüfen ob aktueller Spieler MC > 0 hat
                learnScript->reduceMC(*playerIterator);
            }
        }

        nextCycle += 10; //Neue Zykluszeit berechnen (aller 10 Zyklen);
        return true; //Task erneut einfügen
    }

};

class SGlobalMonsterLearnrate : public SchedulerObject {

public:

    SGlobalMonsterLearnrate(unsigned long int first_cycle) {
        nextCycle = first_cycle;
    }
    virtual ~SGlobalMonsterLearnrate() {}
    bool operator()(World *world) {
        ccharactervector < Monster * >::iterator monsterIterator;

        for (monsterIterator = world->Monsters.begin(); monsterIterator < world->Monsters.end(); ++monsterIterator) {
            if ((*monsterIterator)->getMentalCapacity() > 0) {   //Prüfen ob aktuelles Monster MC > 0 hat
                learnScript->reduceMC(*monsterIterator);
            }
        }

        ccharactervector < NPC * >::iterator npcIterator;

        for (npcIterator = world->Npc.begin(); npcIterator < world->Npc.end(); ++npcIterator) {
            if ((*npcIterator)->getMentalCapacity() > 0) {
                learnScript->reduceMC(*npcIterator);
            }
        }

        nextCycle += 10; //Neue Zykluszeit berechnen (aller 30 Zyklen) zur Serverentlastung dafür die dreifache Senkung;
        return true; //Task erneut einfügen
    }
};

/*
class SItemScriptCycle : public SchedulerObject
{
    public:

        SItemScriptCycle(unsigned long int first_cycle) { nextCycle = first_cycle; }

        virtual ~SItemScriptCycle() {}

        bool operator() ( World* world )
        {
                CommonItems->NextCycle();
                nextCycle += 10;
                return true;
        }
};
*/

class STempAttribCycle : public SchedulerObject {
public:

    STempAttribCycle(unsigned long int first_cycle) {
        nextCycle = first_cycle;
    }

    virtual ~STempAttribCycle() {}

    bool operator()(World *world) {
        ccharactervector< Monster * >::iterator monsterIterator;

        for (monsterIterator = world->Monsters.begin(); monsterIterator < world->Monsters.end(); ++monsterIterator) {
            (*monsterIterator)->tempAttribCheck();
        }

        ccharactervector< NPC * >::iterator npcIterator;

        for (npcIterator = world->Npc.begin(); npcIterator < world->Npc.end(); ++npcIterator) {
            (*npcIterator)->tempAttribCheck();
        }

        ccharactervector < Player * >::iterator playerIterator;

        for (playerIterator = world->Players.begin(); playerIterator < world->Players.end(); ++playerIterator) {
            (*playerIterator)->tempAttribCheck();
        }

        nextCycle += 1;
        return true;

    }
};
#endif
