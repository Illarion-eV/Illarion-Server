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

#include "netinterface/NetInterface.hpp"

#include "CommandFactory.hpp"
#include "Player.hpp"
#include "netinterface/BasicClientCommand.hpp"
#include "netinterface/protocol/ClientCommands.hpp"

#include <climits>
#include <iomanip>

NetInterface::NetInterface(boost::asio::io_service &io_servicen)
        : online(false), headerBuffer{0}, socket(io_servicen), inactive(0), owner(nullptr) {
    cmd.reset();
}

auto NetInterface::getIPAdress() -> std::string { return ipadress; }

NetInterface::~NetInterface() {
    try {
        online = false;
        sendQueue.clear();
        socket.close();
    } catch (std::exception &e) {
        Logger::error(LogFacility::Other) << "Error in NetInterface destructor: " << e.what() << Log::end;
    } catch (...) {
    }
}

void NetInterface::closeConnection() { online = false; }

auto NetInterface::activate(Player *player) -> bool {
    try {
        owner = player;
        boost::asio::async_read(socket, boost::asio::buffer(headerBuffer),
                                [shared_this = shared_from_this()](const auto &error, auto bytes_transferred) {
                                    shared_this->handle_read_header(error);
                                });
        ipadress = socket.remote_endpoint().address().to_string();
        online = true;
        return true;
    } catch (std::exception &e) {
        if (player != nullptr) {
            Logger::error(LogFacility::Other)
                    << "Error in NetInterface::activate for " << player->to_string() << ": " << e.what() << Log::end;
        } else {
            Logger::error(LogFacility::Other) << "Error in NetInterface::activate: " << e.what() << Log::end;
        }

        return false;
    }
}

void NetInterface::handle_read_data(const boost::system::error_code &error) {
    if (!error) {
        if (online) {
            try {
                cmd->decodeData();

                if (cmd->isDataOk()) {
                    cmd->setReceivedTime();

                    if (owner == nullptr) {
                        auto login = std::dynamic_pointer_cast<LoginCommandTS>(cmd);

                        if (!login) {
                            closeConnection();
                            return;
                        }

                        loginData = login;
                        return;
                    }
                    owner->receiveCommand(cmd);
                }
            } catch (OverflowException &e) {
                std::ostringstream message;
                message << "Overflow while reading from buffer from ";
                message << getIPAdress() << ": ";

                const auto &data = cmd->msg_data();
                message << std::hex << std::uppercase << std::setfill('0');

                for (int i = 0; i < cmd->getLength(); ++i) {
                    message << std::setw(2) << (int)data.at(i) << " ";
                }

                message << std::dec << std::nouppercase;
                Logger::error(LogFacility::Other) << message.str() << Log::end;

                closeConnection();
            }

            cmd.reset();
            boost::asio::async_read(socket, boost::asio::buffer(headerBuffer),
                                    [shared_this = shared_from_this()](const auto &error, auto bytes_transferred) {
                                        shared_this->handle_read_header(error);
                                    });
        }
    } else {
        closeConnection();
        boost::asio::async_read(socket, boost::asio::buffer(headerBuffer),
                                [shared_this = shared_from_this()](const auto &error, auto bytes_transferred) {
                                    shared_this->handle_read_header(error);
                                });
    }
}

auto NetInterface::nextInactive() -> bool {
    inactive++;
    return (inactive > maxInactive);
}

void NetInterface::handle_read_header(const boost::system::error_code &error) {
    if (!error) {
        if ((headerBuffer[commandPosition] xor UCHAR_MAX) == headerBuffer[commandPosition + 1]) {
            // Correcter header decodieren und comand empfangen
            uint16_t length = headerBuffer[lengthPosition] << CHAR_BIT;
            length = length | headerBuffer[lengthPosition + 1];
            uint16_t checkSum = headerBuffer[crcPosition] << CHAR_BIT;
            checkSum = checkSum | headerBuffer[crcPosition + 1];
            cmd = commandFactory.getCommand(headerBuffer[commandPosition]);

            if (cmd) {
                cmd->setHeaderData(length, checkSum);
                boost::asio::async_read(socket, boost::asio::buffer(cmd->msg_data(), cmd->getLength()),
                                        [shared_this = shared_from_this()](const auto &error, auto bytes_transferred) {
                                            shared_this->handle_read_data(error);
                                        });

                return;
            }
        }

        // no correct header

        // look for command id in header
        for (int i = 1; i < headerSize - 1; ++i) {
            // found correct command id
            if ((headerBuffer.at(i) xor UCHAR_MAX) == headerBuffer.at(i + 1)) {
                // copy the rest of the correct message to the start of the buffer
                int start = 0;

                while (i < headerSize) {
                    headerBuffer.at(start++) = headerBuffer.at(i++);
                }

                // restheader empfangen
                boost::asio::async_read(socket, boost::asio::buffer(&headerBuffer.at(start), headerSize - start),
                                        [shared_this = shared_from_this()](const auto &error, auto bytes_transferred) {
                                            shared_this->handle_read_header(error);
                                        });

                return;
            }
        }

        // Keine Command Signature gefunden wieder 6 Byte Header auslesen
        boost::asio::async_read(socket, boost::asio::buffer(headerBuffer),
                                [shared_this = shared_from_this()](const auto &error, auto bytes_transferred) {
                                    shared_this->handle_read_header(error);
                                });

    } else {
        if (online) {
            std::string errorMsg = error.message();

            // Handle specific errors
            if (error == boost::asio::error::eof) {
                errorMsg = "Connection closed by remote peer (EOF)";
            } else if (error == boost::asio::error::operation_aborted) {
                errorMsg = "Operation aborted (possible shutdown or timeout)";
            } else if (error == boost::asio::error::connection_reset) {
                errorMsg = "Connection reset by peer";
            }

            // Log the error with additional context
            if (owner != nullptr) {
                Logger::error(LogFacility::Other)
                    << "Error in NetInterface::handle_read_header for " << owner->to_string()
                    << " from " << getIPAdress() << ": " << errorMsg << Log::end;
            } else {
                Logger::error(LogFacility::Other)
                    << "Error in NetInterface::handle_read_header from " << getIPAdress()
                    << ": " << errorMsg << Log::end;
            }
        }

    // Close the connection regardless of the error type
    closeConnection();
    }
}

void NetInterface::addCommand(const ServerCommandPointer &command) {
    if (online) {
        command->addHeader();
        std::lock_guard<std::mutex> lock(sendQueueMutex);
        bool write_in_progress = !sendQueue.empty();
        sendQueue.push_back(command);

        try {
            if (!write_in_progress && online) {
                boost::asio::async_write(
                        socket, boost::asio::buffer(sendQueue.front()->cmdData(), sendQueue.front()->getLength()),
                        [shared_this = shared_from_this()](const auto &error, auto bytes_transferred) {
                            shared_this->handle_write(error);
                        });
            }
        } catch (std::exception &e) {
            Logger::error(LogFacility::Other) << "Exception in NetInterface::addCommand: " << e.what() << Log::end;
            closeConnection();
        }
    }
}

void NetInterface::shutdownSend(const ServerCommandPointer &command) {
    try {
        command->addHeader();
        shutdownCmd = command;
        boost::asio::async_write(socket, boost::asio::buffer(shutdownCmd->cmdData(), shutdownCmd->getLength()),
                                 [shared_this = shared_from_this()](const auto &error, auto bytes_transferred) {
                                     shared_this->handle_write_shutdown(error);
                                 });
    } catch (std::exception &e) {
        Logger::error(LogFacility::Other) << "Exception in NetInterface::shutownSend: " << e.what() << Log::end;
        closeConnection();
    }
}

void NetInterface::handle_write(const boost::system::error_code &error) {
    try {
        if (!error) {
            if (online) {
                std::lock_guard<std::mutex> lock(sendQueueMutex);
                sendQueue.pop_front();

                if (!sendQueue.empty() && online) {
                    boost::asio::async_write(
                            socket, boost::asio::buffer(sendQueue.front()->cmdData(), sendQueue.front()->getLength()),
                            [shared_this = shared_from_this()](const auto &error, auto bytes_transferred) {
                                shared_this->handle_write(error);
                            });
                }
            }
        } else {
            Logger::error(LogFacility::Other) << "Error in NetInterface::handle_write: " << error.message() << Log::end;
            closeConnection();
        }
    } catch (std::exception &e) {
        Logger::error(LogFacility::Other) << "Exception in NetInterface::handle_write: " << e.what() << Log::end;
        closeConnection();
    }
}

void NetInterface::handle_write_shutdown(const boost::system::error_code &error) {
    if (!error) {
        closeConnection();
        shutdownCmd.reset();
    } else {
        if (online) {
            Logger::error(LogFacility::Other)
                    << "Error in NetInterface::handle_write_shutdown: " << error.message() << Log::end;
        }

        closeConnection();
    }
}
