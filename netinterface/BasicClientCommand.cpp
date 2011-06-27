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


#include "BasicClientCommand.hpp"
#include "BasicCommand.hpp"

BasicClientCommand::BasicClientCommand( unsigned char defByte ) : BasicCommand( defByte ), dataOk( true ), length(0), bytesRetrieved(0), checkSum(0), crc(0)
{
    msg_buffer = NULL;
}


void BasicClientCommand::setHeaderData(uint16_t mlength, uint16_t mcheckSum)
{
    length = mlength;
    checkSum = mcheckSum;
    msg_buffer = new unsigned char[length];
}

BasicClientCommand::~BasicClientCommand()
{
    delete[] msg_buffer;
}

unsigned char * BasicClientCommand::msg_data()
{
    return msg_buffer;
}



/*
volatile bool BasicClientCommand::getData( ByteBuffer * recvBuffer )
{
    if ( !headerComplete )
    {
        //try to get the header
        if ( recvBuffer->dataAvailable() >= 4 )
        {
            length = recvBuffer->getByte() << 8;
            length = length | recvBuffer->getByte();
            checkSum = recvBuffer->getByte() << 8;
            checkSum = checkSum | recvBuffer->getByte();
            //create the new buffer
            buffer = new unsigned char[length];
            headerComplete = true;
        }
        else
        {
            //not enough data for reading the complete header so return false and wait
            return false;
        }
    }
    // not else because in the first step we can get the header 
    if ( headerComplete )
    {
        //try to get the data
        //loop until we got enough bytes
        while ( bytesReceived < length )
        {
            if ( recvBuffer->dataAvailable() <= 0 ) return false;
            buffer[ bytesReceived ] = recvBuffer->getByte();
            bytesReceived++;
        }
        return true;
    }
    return false;
}*/

unsigned char BasicClientCommand::getUnsignedCharFromBuffer() throw (OverflowException)
{
    unsigned char ret = 0;
    //no buffer available but we want to read from it
    if ( !msg_buffer )dataOk = false;
    //we want to read more data than there is in the buffer
    else if ( bytesRetrieved > length )
    {
        dataOk = false;
        std::cout<<"try to read more data then in buffer for command:"<<std::hex<<static_cast<int>(getDefinitionByte())<<std::dec<<std::endl;
        throw OverflowException();
    }
    //all went well
    else
    {
        ret = msg_buffer[ bytesRetrieved++ ];
    }
    crc += ret;
    return ret;
}

std::string BasicClientCommand::getStringFromBuffer()
{
    unsigned char len = getUnsignedCharFromBuffer();
    if (len>250) len = 250;
    std::string ret="";
    unsigned char resolved;
    for ( int i = 0; i < len; ++i)
    {
        resolved = getUnsignedCharFromBuffer();
        ret.append( 1, resolved);
    }
    return ret;
}

int BasicClientCommand::getIntFromBuffer()
{
    int ret = getUnsignedCharFromBuffer() << 24;
    ret = ret | (getUnsignedCharFromBuffer() << 16);
    ret = ret | (getUnsignedCharFromBuffer() << 8);
    ret = ret | getUnsignedCharFromBuffer();
    return ret;
}

short int BasicClientCommand::getShortIntFromBuffer()
{
    short int ret = getUnsignedCharFromBuffer() << 8;
    ret = ret | getUnsignedCharFromBuffer();
    return ret;
}

bool BasicClientCommand::isDataOk()
{ 
    uint16_t crcCheck = static_cast<uint16_t>( crc % 0xFFFF);
    return (dataOk && (length == bytesRetrieved) && (crcCheck==checkSum)); 
}
