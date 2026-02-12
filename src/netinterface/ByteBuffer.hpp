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

constexpr int RECV_BUFFERSIZE = 100; ///< Size of each buffer segment in bytes
constexpr int NUMBEROFBUFFERS = 12; ///< Total number of ring buffer segments

/**
 * @brief Thread-safe ring buffer for network data reception.
 * @ingroup Netinterface
 *
 * Provides lock-free concurrent access for network reading and command processing:
 * - Network thread writes incoming socket data to write buffer
 * - Command processing thread reads from read buffer
 * - Ring structure with 12 segments of 100 bytes each (1200 bytes total)
 * - Atomic operations ensure read buffer never overtakes write buffer
 *
 * Buffer lifecycle:
 * 1. Network thread fills current write buffer
 * 2. Calls writeToBuf() to commit and advance write position
 * 3. Processing thread calls getByte() to read sequentially
 * 4. When read buffer exhausted, automatically advances to next segment
 *
 * @note Lock-free design minimizes contention between network I/O and game logic
 * @note Buffer full condition (write catches up to read) causes writeToBuf() to fail
 */
class ByteBuffer {
public:
    /**
     * @brief Constructs buffer with all segments marked empty.
     */
    ByteBuffer();
    ByteBuffer(const ByteBuffer &) = delete;
    auto operator=(const ByteBuffer &) -> ByteBuffer & = delete;
    ByteBuffer(ByteBuffer &&) = delete;
    auto operator=(ByteBuffer &&) -> ByteBuffer & = delete;
    ~ByteBuffer() = default;

    /**
     * @brief Internal structure representing one buffer segment.
     */
    using t_rbuffer = struct {
        uint16_t fill; ///< Number of valid bytes currently in this segment
        std::array<unsigned char, RECV_BUFFERSIZE> buff; ///< Raw data storage
    };

    /**
     * @brief Commits data to current write buffer and advances to next segment.
     * @param size Number of bytes written to the buffer (must be <= RECV_BUFFERSIZE)
     * @return True if commit successful and new write buffer available, false if buffer full
     *
     * Atomically updates buffer metadata and advances write position. Returns false
     * if the next write buffer would collide with the current read buffer.
     *
     * @note Called by network thread after filling buffer with socket data
     */
    auto writeToBuf(uint16_t size) -> bool;

    /**
     * @brief Reads and removes one byte from the buffer.
     * @return The next byte in sequence, or 0 if buffer empty
     *
     * Automatically advances to next read buffer segment when current exhausted.
     * Thread-safe for concurrent access with writeToBuf().
     */
    auto getByte() -> unsigned char;

    /**
     * @brief Returns total unread bytes across all buffer segments.
     * @return Number of bytes available for reading
     */
    [[nodiscard]] auto dataAvailable() const -> uint16_t;

private:
    /**
     * @brief Advances to next buffer segment for reading.
     * @return True if new read buffer available, false if caught up to write buffer
     *
     * Uses try_lock to avoid blocking if write operation in progress.
     */
    auto getReadBuffer() -> bool;

    std::mutex vlock; ///< Mutex protecting buffer structure modifications
    uint16_t bytesAvailable{0}; ///< Cached count of total readable bytes

    std::atomic_uint8_t rBuff{0}; ///< Current read buffer index (0-11)
    std::atomic_uint8_t wBuff{1}; ///< Current write buffer index (0-11)
    std::atomic_uint16_t readPos{0}; ///< Byte position within current read buffer
    std::array<t_rbuffer, NUMBEROFBUFFERS> recvBuffer{}; ///< Ring buffer segments
};

#endif
