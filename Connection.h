#ifndef CONNECTION_HH
#define CONNECTION_HH

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


//! Grundelemente einer Verbindung
class CConnection {
	public:
		//! die ID des Empfangssockets
		int m_read_socket;

		//! Empfangs - Adresse innerhalb des Kommunikations-Adressraumes
		struct sockaddr_in m_read_addr;

		//! Sende - Adresse innerhalb des Kommunikations-Adressraumes
		struct sockaddr_in m_write_addr;
};

#endif
