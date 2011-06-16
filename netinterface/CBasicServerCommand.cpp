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


#include "CBasicServerCommand.hpp"
#include "CBasicCommand.hpp"
#include <sys/socket.h>
#include <iostream>
#include <assert.h>
#include <malloc.h>
#include "Connection.h"
#include "netinterface/CNetInterface.hpp"

CBasicServerCommand::CBasicServerCommand( unsigned char defByte ) : CBasicCommand(defByte)
{
    STDBUFFERSIZE = 1000;
    bufferPos=0;
    bufferSizeMod=1;
    checkSum = 0;
    buffer = new char[STDBUFFERSIZE];
    this->addUnsignedCharToBuffer(getDefinitionByte());
    this->addUnsignedCharToBuffer(getDefinitionByte() xor static_cast<unsigned char>(255));
    this->addShortIntToBuffer( 0 ); //<- dummy for the length
    this->addShortIntToBuffer( 0 ); //<- dummy for the checksum
    checkSum = 0;
}


CBasicServerCommand::CBasicServerCommand( unsigned char defByte , uint16_t bsize) : CBasicCommand(defByte)
{
    STDBUFFERSIZE = bsize;
    bufferPos=0;
    bufferSizeMod=1;
    checkSum = 0;
    buffer = new char[STDBUFFERSIZE];
    this->addUnsignedCharToBuffer(getDefinitionByte());
    this->addUnsignedCharToBuffer(getDefinitionByte() xor static_cast<unsigned char>(255));
    this->addShortIntToBuffer( 0 ); //<- dummy for the length
    this->addShortIntToBuffer( 0 ); //<- dummy for the checksum  
    checkSum = 0;
}

CBasicServerCommand::~CBasicServerCommand()
{
    delete[] buffer;
    buffer = NULL;
}

void CBasicServerCommand::addHeader()
{
    //at place 2 and 3 add the length
    if ( bufferPos >= 6 ) //check if the buffer is large enough to add the data
    {
        int16_t crc = static_cast<int16_t>( checkSum % 0xFFFF );
        buffer[2] = ( (bufferPos-6) >> 8 );
        buffer[3] = ( (bufferPos-6) & 255 );
        buffer[4] = ( crc >> 8 );
        buffer[5] = ( crc & 255 );
    }
}

int CBasicServerCommand::getLength()
{
    return bufferPos;
}

char * CBasicServerCommand::cmdData()
{
    return buffer;

}

void CBasicServerCommand::addStringToBuffer( std::string data )
{
    unsigned short int count = data.length();
    
    if ( count > 254 ) count = 254; 
    addUnsignedCharToBuffer( static_cast<unsigned char>(count) );
    
    for ( unsigned short int i = 0; i < count; ++i)
    {
        addUnsignedCharToBuffer( data.at(i) );
    }
}

void CBasicServerCommand::addIntToBuffer( int data )
{
    addUnsignedCharToBuffer( (data >> 24) );
    addUnsignedCharToBuffer( ((data >> 16) & 255) );
    addUnsignedCharToBuffer( ((data >> 8) & 255) );
    addUnsignedCharToBuffer( (data & 255) );    
}
    
void CBasicServerCommand::addShortIntToBuffer( short int data )
{
    addUnsignedCharToBuffer( (data >> 8) );
    addUnsignedCharToBuffer( (data & 255) );
}

void CBasicServerCommand::addUnsignedCharToBuffer( unsigned char data )
{
    //resize the buffer if there is not enough place to store
    if ( (bufferPos+1) >= (bufferSizeMod*STDBUFFERSIZE) ) resizeBuffer();
    assert( bufferPos < (bufferSizeMod*STDBUFFERSIZE) );
    buffer[ bufferPos ] = data;
    checkSum+=data; //add the data to the checksum
    bufferPos++;
}

void CBasicServerCommand::resizeBuffer()
{
    std::cout<<"not enough memory resizing the sendbuffer"<<std::endl;
    //increase the buffer size modifikator
    bufferSizeMod++;
    //store old data in temp
    char * temp = buffer;
    //resize buffer
    buffer = new char[bufferSizeMod*STDBUFFERSIZE];
    //save data back to the buffer
    for ( uint32_t i = 0; i<bufferPos; ++i)
    {
        buffer[i] = temp[i];
    }
    //delete the temp buffer;
    delete[] temp;
    temp = NULL;
    std::cout<<"resizing the sendBuffer successfully"<<std::endl;
}

