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

#include "ByteBuffer.hpp"

#include <iostream>
#include <string>

ByteBuffer::ByteBuffer() {
    recvBuffer = new t_rbuffer[NUMBEROFBUFFERS];

    for (int i = 0; i < NUMBEROFBUFFERS; ++i) {
        recvBuffer[i].fill = 0;
    }
}

ByteBuffer::~ByteBuffer() { delete[] recvBuffer; }

auto ByteBuffer::dataAvailable() const -> uint16_t { return (bytesAvailable); }

auto ByteBuffer::getByte() -> unsigned char {
    if (bytesAvailable > 0) {
        if (recvBuffer[rBuff].fill > readPos) {
            bytesAvailable--;
            unsigned char ret = recvBuffer[rBuff].buff[readPos++];
            return ret;

        } // end of the current readbuffer

        if (getReadBuffer()) {
            readPos = 0;

            if (recvBuffer[rBuff].fill > 0) {
                bytesAvailable--;

                unsigned char ret = recvBuffer[rBuff].buff[readPos++];

                return ret;
            }
        }
    }

    return 0;
}

auto ByteBuffer::getReadBuffer() -> bool {
    if (vlock.try_lock()) {
        uint8_t nr = (rBuff + 1) % NUMBEROFBUFFERS; // get next buffer number

        if (wBuff != nr) {
            rBuff = nr;
            vlock.unlock();
            return true;
        }
    }

    vlock.unlock();
    return false;
}

auto ByteBuffer::writeToBuf(uint16_t size) -> bool {
    std::lock_guard<std::mutex> lock(vlock);
    uint8_t nr = (wBuff + 1) % NUMBEROFBUFFERS;

    if (nr != rBuff) {
        recvBuffer[wBuff].fill = size;
        wBuff = nr;
        bytesAvailable += size;
        return true;
    }
    return false;
}
