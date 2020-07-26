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

#ifndef BYTE_BUFFER_HPP
#define BYTE_BUFFER_HPP

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>

constexpr int RECV_BUFFERSIZE = 100;
constexpr int NUMBEROFBUFFERS = 12;

/**
 *@ingroup Netinterface
 * a thread safe ring buffer. Holds the data which is received from the socket
 * and stores it in 12 sections of 100 bytes each.
 * it guarantees that the read buffer cannot be in front of the write buffer
 */
class ByteBuffer {
public:
    ByteBuffer();
    ByteBuffer(const ByteBuffer &) = delete;
    auto operator=(const ByteBuffer &) -> ByteBuffer & = delete;
    ByteBuffer(ByteBuffer &&) = delete;
    auto operator=(ByteBuffer &&) -> ByteBuffer & = delete;
    ~ByteBuffer() = default;

    /**
     * struct which represens the internal structur of the buffer
     */
    using t_rbuffer = struct {
        uint16_t fill; /*<how much data is currently in this buffer section*/
        std::array<unsigned char, RECV_BUFFERSIZE> buff;
    };

    /**
     * adds the size of bytes which was written in the current write buff and sets the new write buffer
     * @param size how many bytes are written to the Buffer
     * @return true if the writing was successfull and a new empty write buffer is ready otherwise false
     */
    auto writeToBuf(uint16_t size) -> bool;

    /**
     * returns one byte from the buffer
     * @return the byte from the buffer
     */
    auto getByte() -> unsigned char;

    /**
     * return how much data is available in the buffer
     * @return the number of bytes which are currently in the buffer
     */
    [[nodiscard]] auto dataAvailable() const -> uint16_t;

private:
    /**
     * gets a new and empty readBuffer
     * @return true if there is a new readBuffer available otherwise false so we have to wait for some data to be read
     * before we can try it again
     */
    auto getReadBuffer() -> bool;

    std::mutex vlock;           /*<mutex for thread safety*/
    uint16_t bytesAvailable{0}; /*<stores how much bytes are currently in the buffer*/

    std::atomic_uint8_t rBuff{0};                        /*<number of current read buffer*/
    std::atomic_uint8_t wBuff{1};                        /*<number of the current write buffer*/
    std::atomic_uint16_t readPos{0};                     /*<current reading position inside the read buffer*/
    std::array<t_rbuffer, NUMBEROFBUFFERS> recvBuffer{}; /*<internal buffer collection*/
};

#endif
