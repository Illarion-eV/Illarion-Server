//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#include "BasicServerCommand.hpp"

#include "BasicCommand.hpp"
#include "Connection.hpp"
#include "Logger.hpp"
#include "netinterface/NetInterface.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sys/socket.h>

BasicServerCommand::BasicServerCommand(unsigned char defByte) : BasicCommand(defByte) {
    buffer.resize(STDBUFFERSIZE);
    this->addUnsignedCharToBuffer(getDefinitionByte());
    this->addUnsignedCharToBuffer(getDefinitionByte() xor static_cast<unsigned char>(255));
    this->addShortIntToBuffer(0); //<- dummy for the length
    this->addShortIntToBuffer(0); //<- dummy for the checksum
    checkSum = 0;
}

BasicServerCommand::BasicServerCommand(unsigned char defByte, uint16_t bsize) : BasicCommand(defByte) {
    STDBUFFERSIZE = bsize;
    buffer.resize(STDBUFFERSIZE);
    this->addUnsignedCharToBuffer(getDefinitionByte());
    this->addUnsignedCharToBuffer(getDefinitionByte() xor static_cast<unsigned char>(255));
    this->addShortIntToBuffer(0); //<- dummy for the length
    this->addShortIntToBuffer(0); //<- dummy for the checksum
    checkSum = 0;
}

void BasicServerCommand::addHeader() {
    // at place 2 and 3 add the length
    if (bufferPos >= 6) { // check if the buffer is large enough to add the data
        auto crc = static_cast<int16_t>(checkSum % 0xFFFF);
        buffer.at(2) = ((bufferPos - 6) >> 8);
        buffer.at(3) = ((bufferPos - 6) & 255);
        buffer.at(4) = (crc >> 8);
        buffer.at(5) = (crc & 255);
    }
}

auto BasicServerCommand::getLength() const -> int { return bufferPos; }

auto BasicServerCommand::cmdData() const -> const std::vector<char> & { return buffer; }

void BasicServerCommand::addStringToBuffer(const std::string &data) {
    unsigned short int count = data.length();
    addShortIntToBuffer(count);
    std::for_each(data.cbegin(), data.cend(), [this](auto c) { addUnsignedCharToBuffer(c); });
}

void BasicServerCommand::addIntToBuffer(int data) {
    addUnsignedCharToBuffer((data >> 24));
    addUnsignedCharToBuffer(((data >> 16) & 255));
    addUnsignedCharToBuffer(((data >> 8) & 255));
    addUnsignedCharToBuffer((data & 255));
}

void BasicServerCommand::addShortIntToBuffer(short int data) {
    addUnsignedCharToBuffer((data >> 8));
    addUnsignedCharToBuffer((data & 255));
}

void BasicServerCommand::addUnsignedCharToBuffer(unsigned char data) {
    // resize the buffer if there is not enough place to store
    if ((bufferPos + 1) >= (bufferSizeMod * STDBUFFERSIZE)) {
        resizeBuffer();
    }

    assert(bufferPos < (bufferSizeMod * STDBUFFERSIZE));
    buffer.at(bufferPos) = data;
    checkSum += data; // add the data to the checksum
    bufferPos++;
}

void BasicServerCommand::resizeBuffer() {
    Logger::info(LogFacility::Other) << "Not enough memory. Resizing the send buffer. Current size: "
                                     << bufferSizeMod * STDBUFFERSIZE << " bytes." << Log::end;
    bufferSizeMod++;
    buffer.resize(bufferSizeMod * STDBUFFERSIZE);
    Logger::info(LogFacility::Other) << "Resizing the send buffer successful. New size: "
                                     << bufferSizeMod * STDBUFFERSIZE << " bytes." << Log::end;
}

void BasicServerCommand::addColourToBuffer(const Colour &c) {
    addUnsignedCharToBuffer(c.red);
    addUnsignedCharToBuffer(c.green);
    addUnsignedCharToBuffer(c.blue);
    addUnsignedCharToBuffer(c.alpha);
}
