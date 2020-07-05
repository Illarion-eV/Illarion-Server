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

#include "BasicClientCommand.hpp"

#include "BasicCommand.hpp"

#include <climits>

BasicClientCommand::BasicClientCommand(unsigned char defByte, uint16_t minAP) : BasicCommand(defByte), minAP(minAP) {}

void BasicClientCommand::setHeaderData(uint16_t mlength, uint16_t mcheckSum) {
    length = mlength;
    checkSum = mcheckSum;
    msg_buffer.resize(length);
}

auto BasicClientCommand::msg_data() -> std::vector<unsigned char> & { return msg_buffer; }

auto BasicClientCommand::getUnsignedCharFromBuffer() -> unsigned char {
    unsigned char ret = 0;

    // no buffer available but we want to read from it
    if (length == 0) {
        dataOk = false;
    }
    // we want to read more data than there is in the buffer
    else if (bytesRetrieved > length) {
        dataOk = false;
        throw OverflowException();
    }
    // all went well
    else {
        ret = msg_buffer.at(bytesRetrieved++);
    }

    crc += ret;
    return ret;
}

auto BasicClientCommand::getStringFromBuffer() -> std::string {
    unsigned short int len = getShortIntFromBuffer();

    std::string ret;

    for (int i = 0; i < len; ++i) {
        ret.append(1, getUnsignedCharFromBuffer());
    }

    return ret;
}

auto BasicClientCommand::getIntFromBuffer() -> int {
    int ret = getUnsignedCharFromBuffer() << 3 * CHAR_BIT;
    ret = ret | (getUnsignedCharFromBuffer() << 2 * CHAR_BIT);
    ret = ret | (getUnsignedCharFromBuffer() << CHAR_BIT);
    ret = ret | getUnsignedCharFromBuffer();
    return ret;
}

auto BasicClientCommand::getShortIntFromBuffer() -> short int {
    auto ret = static_cast<short int>(getUnsignedCharFromBuffer() << CHAR_BIT);
    ret = static_cast<short int>(ret | getUnsignedCharFromBuffer());
    return ret;
}

auto BasicClientCommand::isDataOk() const -> bool {
    constexpr auto allBitsSet = 0xFFFF;
    auto crcCheck = static_cast<uint16_t>(crc % allBitsSet);
    return (dataOk && (length == bytesRetrieved) && (crcCheck == checkSum));
}
