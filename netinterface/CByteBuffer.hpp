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


#ifndef _CBYTE_BUFFER_HPP_
#define _CBYTE_BUFFER_HPP_

#include <stdint.h>
#include <pthread.h>

#define RECV_BUFFERSIZE 100
#define NUMBEROFBUFFERS 12

/**
*@ingroup Netinterface
* a thread save ring buffer. Holds the date which are received from the socket
* and stores them in 12 section a 100 bytes.
* it guaranties that the read buffer cant be in front of the write buffer
*/
class CByteBuffer
{
    public:
        /**
        * standard constructor
        */
        CByteBuffer();
        
        /**
        * standard destructor
        */
        ~CByteBuffer();
    
        /**
        * struct which represens the internal structur of the buffer
        */
        typedef struct 
        {
            uint16_t fill; /*<how much data is currently in this buffer section*/
            unsigned char buff[ RECV_BUFFERSIZE ]; /*<a pointer to this buffer section*/
        } t_rbuffer;
        
        /**
        * gives the current writing buffer so that data can be added to it
        * @return a pointer to the current write buffer
        */
        unsigned char * writeBuff();
        
        /**
        * adds the size of bytes which was written in the current write buff and sets the new write buffer
        * @param size how many bytes are written to the Buffer
        * @return true if the writing was successfull and a new empty write buffer is ready otherwise false
        */
        bool writeToBuf(uint16_t size);
        
        /**
        * returns one byte from the buffer
        * @return the byte from the buffer
        */
        unsigned char getByte();
        
        /**
        * return how much data is available in the buffer
        * @return the number of bytes which are currently in the buffer
        */
        uint16_t dataAvailable();
           
    private:
    
        /**
        * gets a new and empty readBuffer
        * @return true if there is a new readBuffer available otherwise false so we have to wait for some data to be read before we can try it again
        */
        bool getReadBuffer(); 
    
        pthread_mutex_t *vlock; /*<mutex for thread safety*/
        uint16_t bytesAvailable; /*<stores how much bytes are currently in the buffer*/
        
        volatile uint8_t rBuff; /*<number of current read buffer*/
        volatile uint8_t wBuff; /*<number of the current write buffer*/
        volatile uint16_t readPos; /*<current reading position inside the read buffer*/
        t_rbuffer * recvBuffer; /*<pointer to a internal buffer struct which holds the buffer in a array*/

};

#endif
