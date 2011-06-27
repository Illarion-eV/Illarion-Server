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


#include "Logger.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


extern std::map<std::string, std::string> configOptions;

CLogger::LOGMAP CLogger::logs;
CLogger::LOGACTIVATEDMAP CLogger::logact;
CLogger::CLogger * CLogger::theLoggerInstance;

CLogger::CLogger()
{
}

CLogger::~CLogger()
{
}

CLogger * CLogger::get()
{
    //proof if we have a logger instance
    if ( theLoggerInstance == NULL )
    {
        //if there is no instance create on instance
        theLoggerInstance = new CLogger();
    }
    
    //return the instance
    return theLoggerInstance;
}

std::string CLogger::getLogDate()
{
    //Load current timestamp
    time_t acttime = time(NULL);
    //save the timestamp as string
    std::string logtime = ctime(&acttime);
    //Add a : to the string
    logtime[logtime.size()-1] = ':';
    return logtime;
}

void CLogger::writeError(std::string LogType, std::string Message, bool saveTime)
{
    //if ( isLogActivated(LogType) )
    {
        boost::shared_ptr<std::ofstream> theLog;
        //try to find a open log from the current error logtype
        if (!findLog("_" + LogType + "_err", theLog))
        {
            
            if (!createLog("_" + LogType + "_err", theLog))
            {
                std::cerr<<"Cant create " << + "_" + LogType+"_err.log" << " for writing!"<<std::endl;
                return;
            }
        }
        if (theLog)
        {
            if (saveTime)
                (*theLog) << getLogDate() << " " << Message << std::endl;
            else
                (*theLog) << Message << std::endl;
        }
    }
}

void CLogger::writeMessage(std::string LogType, std::string Message, bool saveTime)
{
    if (isLogActivated(LogType))
    {
        boost::shared_ptr<std::ofstream> theLog;
        //!try to find a open log from the current messagetype
        if (!findLog("_" + LogType + "_msg", theLog))
        {
            if (!createLog("_" + LogType + "_msg", theLog))
            {
                std::cerr<<"Cant create " << + "_" + LogType + "_msg.log" << " for writing!"<<std::endl;
                return;
            }
        }
        if (theLog)
        {
            if (saveTime)
                (*theLog) << getLogDate() << " " << Message << std::endl;
            else
                (*theLog) << Message << std::endl;
        }
    }
}

bool CLogger::createLog(std::string LogType,/*std::ofstream * pLog*/ boost::shared_ptr<std::ofstream> &pLog)
{
    boost::shared_ptr<std::ofstream> ptheLog(new std::ofstream() );
    std::string file = configOptions["logdir"] + configOptions["starttime"] + LogType + ".log"; 
    if ( ptheLog )
    {
        ptheLog->open(file.c_str());
        if ( !ptheLog->good() )
        {
            std::cerr << "Could not open " << configOptions["logdir"] + configOptions["starttime"] << LogType << ".log" << " for writing!" << std::endl;
            return false;
        }
        else
        {
            logs[LogType] = ptheLog;
            pLog = ptheLog;
            if ( !isLogActivated(LogType) )logact[LogType] = false;
            return true;
        }
    }
    else
    {
        std::cerr << "pointer to ofstream is null!" <<std::endl;
    }
    return false;
}

void CLogger::activateLog(std::string LogType)
{
    logact[LogType] = true;
}

void CLogger::deactivateLog(std::string LogType)
{
    logact[LogType] = false;
}

bool CLogger::isLogActivated(std::string LogType)
{
    LOGACTIVATEDMAP::iterator theIterator;
    theIterator = logact.find( LogType );
    if ( theIterator == logact.end() )
    {
        return false;
    }
    else
    {
        return theIterator->second;
    }
    return false;    
}

bool CLogger::findLog(std::string LogType, /*std::ofstream * pLog*/ boost::shared_ptr<std::ofstream> &pLog)
{
    LOGMAP::iterator theIterator;
    theIterator = logs.find( LogType );
    if ( theIterator == logs.end() )
    {
        return false;
    }
    else
    {
        pLog = ( *theIterator ).second;
        return true;
    }
    return false;
}


