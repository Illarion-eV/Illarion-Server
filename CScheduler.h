#ifndef CSHEDULER_H
#define CSHEDULER_H

#include <list>

class CWorld;

class CSchedulerObject {

	public:
		//überladung für den Operator ()
		virtual bool operator() (CWorld * world) = 0;
		inline unsigned long int GetNextCycle() { return nextCycle; }
		inline void SetNextCycle(unsigned long int nCycle) { nextCycle = nCycle; }
		inline short int GetCount() { return count; }
		inline void SetCount(unsigned short int nCount) { count = nCount; }
		inline unsigned long int GetCycleTime() { return CycleTime; }
		inline void SetCycleTime( unsigned long int nCycleTime) { CycleTime = nCycleTime; }
		virtual ~CSchedulerObject() {}

	protected:

		unsigned long int nextCycle ;
		unsigned long int CycleTime ;
		unsigned short int count;

	private:
		//! no copy operator for pure virtual classes
		CSchedulerObject& operator=(const CSchedulerObject&);
};


class CScheduler {

	public:
		//constructor welcher den Thread erstellt
		// \param Zeiger auf das World objekt in dem der Scheduler läuft
		CScheduler(CWorld * pworld);
		//Destructor welcher den Thread beendet.
		~CScheduler();
		//Funktion um einen Task in die Liste hinzu zu fügen
		void AddTask(CSchedulerObject* sobject);
		//Funktion welche den nächsten Zyklus einleitet.
		void NextCycle();
		//Funktion welche den Aktuellen Zyklus zurück liefert.
		inline unsigned long int GetCurrentCycle() { return cycle; }

	private:
		//Liste für die Sheduler objekte
		std::list<CSchedulerObject*> Tasks;
		//Aktueller Zyklus
		unsigned long int cycle;
		//verbindung zur Spielwelt
		CWorld * world;
};

#endif
