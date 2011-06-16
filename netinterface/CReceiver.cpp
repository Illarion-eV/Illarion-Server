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


#include "netinterface/CReceiver.hpp"
#include "netinterface/CBasicClientCommand.hpp"
#include "netinterface/CBasicServerCommand.hpp"
#include "netinterface/protocol/ClientCommands.hpp"

CReceiver::CReceiver( CNetInterface * conn, boost::shared_ptr<CNetInterface::CLIENTCOMMANDLIST> rQueue ) : connection( conn ), receivedQueue( rQueue )
{
    numtrys = 0;
    lastReadedByte = -1;
    boost::shared_ptr<CByteBuffer>tmpBuf(new CByteBuffer());
    buffer = tmpBuf;
    //buffer = new CByteBuffer();
    conn->online = false;
    rlrunning = false;
    rlOK = false;
    //create the two mutexes for secure access to the buffer
    receivedQueue->clear();
    cmd.reset();
}

CReceiver::~CReceiver()
{
    receivedQueue->clear();
    try
    {
        if ( rlOK )
        {
            pthread_cancel( receive_thread );
            std::cout<<"canceled receive thread"<<std::endl;
        }
        //delete buffer;
        //buffer = NULL;
        buffer.reset();
    }
    catch (std::exception const& e)
    {
        //std::cerr << e.what () << std::endl;
    }
    catch ( ... )
    {
        throw;
    }

}

bool CReceiver::start()
{
    pthread_attr_t pattr;
    pthread_attr_init( &pattr);
    pthread_attr_setdetachstate( &pattr, PTHREAD_CREATE_DETACHED );
    bool ok = true;
    
    if ( pthread_create( &receive_thread, &pattr, (void*(*)(void*))&receive_loop, this ) != 0 )
    {
        std::cout << "Read-S-Nummer: " << connection->m_read_socket << " error on receive thread creation (receiver loop): " << strerror( errno ) << std::endl;
        ok = false;
    }
    rlOK = true;
    return ok; 
}

volatile void CReceiver::checkData()
{
    while ( connection->online && (buffer != NULL ) && (buffer->dataAvailable() > 0 ))
    {
        try
        {
            //we got no command so we try to find a new one
            if  ( !cmd )
            {
                while( !cmd && buffer->dataAvailable() > 0 )
                {
                    unsigned char curByte = buffer->getByte();
                    if ( lastReadedByte != -1 && ((static_cast<unsigned char>(lastReadedByte) xor 255) == curByte) )
                    {
                        //std::cout<<"found command signature: "<<std::hex<<lastReadedByte<<std::endl;
                        //we found a command
                        cmd = commandFactory.getCommand( lastReadedByte );
                        //std::cout<<"command: "<<cmd<<std::endl;
                        lastReadedByte = curByte; //command was not in list so the byte was wrong
                    }
                    else
                    {
                        lastReadedByte = curByte; //command was not in list so the byte was wrong
                    }
                }
            }
            if ( cmd )
            {
                    //we have a command
                    if ( cmd->getData( buffer ) )
                    {
                        //std::cout<<"got data for cmd"<<std::endl;
                        numtrys = 0;
                        //the command was fully red   
                        cmd->decodeData();
                        //std::cout<<"data decoded"<<std::endl;
                        if ( cmd->isDataOk() )
                        {
                            //std::cout<<"data was ok"<<std::endl;
                            receivedQueue->push_back( cmd );
                        }
                        //std::cout<<"cmd resetted!"<<std::endl;
                        cmd.reset();
                        if (buffer)
                        {
                            lastReadedByte = buffer->getByte(); //try to get the command id from the necht command
                        }
                    }
                    else if ( numtrys > 250 )
                    {
                        cmd.reset();
                        lastReadedByte = -1;
                        std::cout<<"timeout for receiving command"<<std::endl;
                        numtrys=0;
                    }
                    else numtrys++;
            }
        }
        catch( OverflowException &e )
        {
            std::cout<<"overflow while reading from buffer"<<std::endl;
            cmd.reset();
            lastReadedByte = -1;
        }
    }
}

void * CReceiver::receive_loop( CReceiver * receiver )
{
    try
    {
            //create waittime struct so the thread waits for 20 ms 
            timespec waittime; 
            waittime.tv_sec = 0;
            waittime.tv_nsec = 3000000; //10 ms;
            timespec lwaittime; 
            waittime.tv_sec = 0;
            waittime.tv_nsec = 50000000; //50 ms;            
            
            //stores how often the data can't be stored in the buffer
            //if this number to large the server seems to be crashed and the 
            //connection will be canceled
            receiver->rlrunning = true;
            int recvinactive = 0;
            while ( receiver->connection->online && receiver->rlrunning )
            {
                //there is space in the buffer
                if ( receiver->buffer )
                {
                    recvinactive = 0;
                    //last buffer saved so we can receive new data
                    int count = recv( receiver->connection->m_read_socket, receiver->buffer->writeBuff(), RECV_BUFFERSIZE,MSG_NOSIGNAL);
                    if ( count < 1 )
                    {
                        std::cout<<"error on receiveing data close Connection"<<std::endl;
                        if (receiver->connection->online)receiver->connection->closeConnection();
                    }
                    while ( (receiver->buffer) && !receiver->buffer->writeToBuf( count ) )
                    {
                        nanosleep( &lwaittime , NULL);
                    }
                    //we read the data now try to do something with it
                    receiver->checkData();
                    
                    
                    
                }
                else
                {
                    if ( recvinactive > 100 )
                    {
                        std::cout<<"receiver inactive for more than 100 cycles, closing connection!"<<std::endl;
                        receiver->connection->closeConnection();
                    }
                    recvinactive++;
                    nanosleep(&lwaittime,NULL);
                }
                nanosleep( &waittime, NULL);
            }
            std::cout<<"receiveThread ended!"<<std::endl;
            receiver->rlOK = false;
            pthread_exit( 0 );
            return NULL;
    }
    catch ( std::exception e)
    {
        //std::cerr<<e.what()<<std::endl;
    }
    catch ( ... )
    {
        throw;
    }
    return NULL;
    
}
