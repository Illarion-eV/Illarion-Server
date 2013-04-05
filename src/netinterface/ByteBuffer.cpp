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


#include <string>
#include <malloc.h>
#include <iostream>
#include "ByteBuffer.hpp"
#include <assert.h>

ByteBuffer::ByteBuffer() : bytesAvailable(0), rBuff(0), wBuff(1), readPos(0) {
    vlock = new pthread_mutex_t;

    if (pthread_mutex_init(vlock,nullptr)) {
        std::cout << "Mutex couldn't get initialized... throwing exception!" << std::endl;
        throw std::exception();
    }

    recvBuffer = new t_rbuffer[ NUMBEROFBUFFERS ];

    for (int i = 0 ; i < NUMBEROFBUFFERS; ++i) {
        recvBuffer[ i ].fill = 0;
    }

}

ByteBuffer::~ByteBuffer() {
    pthread_mutex_destroy(vlock);
    delete [] recvBuffer;
    delete vlock;
}


uint16_t ByteBuffer::dataAvailable() {
    return (bytesAvailable);
}

unsigned char ByteBuffer::getByte() {
    if (bytesAvailable > 0) {
        if (recvBuffer[rBuff].fill > readPos) {
            bytesAvailable--;
            unsigned char ret  = recvBuffer[ rBuff ].buff[ readPos++ ];
            return ret;

        } else {
            //end of the current readbuffer
            if (getReadBuffer()) {
                readPos = 0;

                if (recvBuffer[ rBuff ].fill > 0) {
                    bytesAvailable--;
                    unsigned char ret = recvBuffer[ rBuff ].buff[ readPos++ ];
                    return ret;

                }
            }
        }
    }

    return 0;
}

unsigned char *ByteBuffer::writeBuff() {
    return recvBuffer[ wBuff ].buff;
}

bool ByteBuffer::getReadBuffer() {
    int error = pthread_mutex_trylock(vlock);

    if (!error) {
        uint8_t nr = (rBuff + 1) % NUMBEROFBUFFERS;   //get next buffer number

        if (wBuff != nr) {
            rBuff = nr;
            pthread_mutex_unlock(vlock);
            return true;
        }
    }

    pthread_mutex_unlock(vlock);
    return false;
}

bool ByteBuffer::writeToBuf(uint16_t size) {
    pthread_mutex_lock(vlock);
    /*
    std::cout<<"received "<<size<<" bytes"<<std::endl;
    for( int i = 0; i < size; ++i)
    {
        std::cout<<"[ "<<static_cast<int>( recvBuffer[ wBuff ].buff[i] )<<" ]";
    }
    std::cout<<std::endl;
    */
    uint8_t nr = (wBuff + 1) % NUMBEROFBUFFERS;

    if (nr != rBuff) {
        recvBuffer[ wBuff ].fill = size;
        wBuff = nr;
        bytesAvailable += size;
        pthread_mutex_unlock(vlock);
        return true;
    } else {
        pthread_mutex_unlock(vlock);
        return false;
    }

}





