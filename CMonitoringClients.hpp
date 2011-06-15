#ifndef _CMONITORINGCLIENTS_
#define _CMONITORINGCLIENTS_

#include <list>
#include <iostream>
#include <boost/shared_ptr.hpp>

class CBasicServerCommand;
class CWorld;
class CPlayer;

/**
 * class which holds all the monitoring clients on the gameserver
 * and sends all the important server informations to them
 */
class CMonitoringClients 
{
	public:
		CMonitoringClients( CWorld * world);
		~CMonitoringClients();
		
		/**
		 * a new Monitoring client connects to the server
		 * @param player a pointer to the player which connects with the monitoring client
		 */
		void clientConnect( CPlayer * player );
		
		/**
		 * sends a new Command to all the connected clients
		 * @param command the command which should be sended 
		 */
		void sendCommand( boost::shared_ptr<CBasicServerCommand> command );
		
		/**
		 * function which checks if new commands from clients are arrived and handels them
		 */
		void CheckClients();
	
	private:
		std::list<CPlayer*> client_list;
		CWorld * _world; /*< pointer to the gameworld*/
};
#endif //_CMONITORINGCLIENTS_
