#include <string>
#include <malloc.h>
#include <iostream>
#include "CByteBuffer.hpp"
#include <assert.h>

CByteBuffer::CByteBuffer() : bytesAvailable(0), rBuff(0), wBuff(1), readPos(0)
{
    vlock = new pthread_mutex_t;
    if (pthread_mutex_init(vlock,NULL)) 
    {
		std::cout << "Mutex couldn't get initialized... throwing exception!" << std::endl;
		throw std::exception();
	}    
    
    recvBuffer = new t_rbuffer[ NUMBEROFBUFFERS ];
    for ( int i = 0 ; i < NUMBEROFBUFFERS; ++i)
    {
        recvBuffer[ i ].fill = 0;
    }

}

CByteBuffer::~CByteBuffer()
{
    pthread_mutex_destroy(vlock);
    delete [] recvBuffer;
    delete vlock;
}


uint16_t CByteBuffer::dataAvailable()
{
    return ( bytesAvailable );    
}

unsigned char CByteBuffer::getByte()
{
    if ( bytesAvailable > 0 )
    {
        if ( recvBuffer[rBuff].fill > readPos )
        {
            bytesAvailable--;
            unsigned char ret  = recvBuffer[ rBuff ].buff[ readPos++ ];
            return ret;
            
        }
        else
        {
            //end of the current readbuffer
            if ( getReadBuffer() )
            {
                readPos = 0;
                if ( recvBuffer[ rBuff ].fill > 0 )
                {
                    bytesAvailable--;
                    unsigned char ret = recvBuffer[ rBuff ].buff[ readPos++ ];
                    return ret;
                    
                }
            }
        }
    }
    return 0;
}

unsigned char * CByteBuffer::writeBuff()
{
     return recvBuffer[ wBuff ].buff;
}

bool CByteBuffer::getReadBuffer() 
{
    int error = pthread_mutex_trylock( vlock );
    if ( !error )
    {
        uint8_t nr = ( rBuff + 1 ) % NUMBEROFBUFFERS; //get next buffer number
        if ( wBuff != nr )
        {
            rBuff = nr;
            pthread_mutex_unlock( vlock);
            return true;
        }
    }
    pthread_mutex_unlock( vlock);
    return false;
}

bool CByteBuffer::writeToBuf(uint16_t size)
{
    pthread_mutex_lock( vlock );
    /*
    std::cout<<"received "<<size<<" bytes"<<std::endl;
    for( int i = 0; i < size; ++i)
    {
        std::cout<<"[ "<<static_cast<int>( recvBuffer[ wBuff ].buff[i] )<<" ]";
    }
    std::cout<<std::endl;
    */    
    uint8_t nr = ( wBuff + 1 ) % NUMBEROFBUFFERS;
    if ( nr != rBuff )
    {
        recvBuffer[ wBuff ].fill = size;
        wBuff = nr;
        bytesAvailable += size;
        pthread_mutex_unlock( vlock );
        return true;
    } 
    else
    {
        pthread_mutex_unlock( vlock );
        return false;
    }
    
}

    
    


