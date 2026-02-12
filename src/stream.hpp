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

#ifndef STREAM_HPP
#define STREAM_HPP

#include <cstring>
#include <fstream>
#include <string>
#include <type_traits>

/**
 * @brief Reads raw bytes from an input stream into a buffer.
 * @param stream The input file stream to read from.
 * @param output The buffer to write data into.
 * @param size The number of bytes to read.
 */
inline void readFromStream(std::ifstream &stream, char *output, std::size_t size) { stream.read(output, size); }

/**
 * @brief Writes raw bytes from a buffer to an output stream.
 * @param stream The output file stream to write to.
 * @param input The buffer containing data to write.
 * @param size The number of bytes to write.
 */
inline void writeToStream(std::ofstream &stream, const char *input, std::size_t size) { stream.write(input, size); }

/**
 * @brief Reads a trivially copyable object from a stream.
 * @tparam T A trivially copyable type (POD types, simple structs).
 * @param stream The input file stream to read from.
 * @param output The object to read data into.
 * @note This uses memcpy and requires T to be trivially copyable to avoid undefined behavior.
 */
template <typename T> void readFromStream(std::ifstream &stream, T &output) {
    static_assert(std::is_trivially_copyable_v<T>); // avoid UB with memcpy
    std::string buffer(sizeof(T), '\0');
    readFromStream(stream, buffer.data(), sizeof(T));
    std::memcpy(&output, buffer.data(), sizeof(T));
}

/**
 * @brief Writes a trivially copyable object to a stream.
 * @tparam T A trivially copyable type (POD types, simple structs).
 * @param stream The output file stream to write to.
 * @param input The object to write to the stream.
 * @note This uses memcpy and requires T to be trivially copyable to avoid undefined behavior.
 */
template <typename T> void writeToStream(std::ofstream &stream, const T &input) {
    static_assert(std::is_trivially_copyable_v<T>); // avoid UB with memcpy
    std::string buffer(sizeof(T), '\0');
    std::memcpy(buffer.data(), &input, sizeof(T));
    writeToStream(stream, buffer.data(), sizeof(T));
}

#endif
