//
// This file is part of DGGA.
// 
// The MIT License (MIT)
// 
// Copyright (c) 2015 Kevin Tierney and Josep Pon Farreny
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.




#include "OutputLog.hpp"

#include <unistd.h>
#include <sys/types.h>

#include <ctime>
#include <iostream>
#include <sstream>

#include "ggatypedefs.hpp"
#include "GGASystem.hpp"
#include "GGAValue.hpp"

OutputLog::Level OutputLog::s_ReportLevel = OutputLog::NORMAL;
pid_t OutputLog::s_masterPid;

OutputLog::OutputLog() {}
OutputLog::~OutputLog() {}

void OutputLog::setMasterPid() {
    s_masterPid = getpid();
}

pid_t OutputLog::masterPid() {
    return s_masterPid;
}

void OutputLog::setReportLevel(OutputLog::Level lvl) {
    s_ReportLevel = lvl;
}

OutputLog::Level OutputLog::reportLevel() {
    return s_ReportLevel;
}


/**
 *
 */
void OutputLog::log(const std::ostringstream& os, Level lvl, bool addPrefix/* = true*/) {
    if(lvl == ERROR && reportLevel() >= ERROR) {
        if(addPrefix) std::cerr << prefix(lvl) << " ";
        std::cerr << os.str() << std::endl;
    } else if(lvl <= reportLevel()) {
        if(addPrefix) std::cout << prefix(lvl) << " ";
        std::cout << os.str() << std::endl;
    }
}


/**
 *
 */
std::string OutputLog::prefix(Level lvl) {
    std::stringstream ss;
    ss.setf(std::ios::fixed,std::ios::floatfield);
    ss.precision(6);
    // CPU time from a child is nonsense
    if(getpid() == masterPid()) {
	time_t rawtime;
        struct tm timeinfo;
        char buffer[80];

        ::time(&rawtime);
        ::localtime_r(&rawtime, &timeinfo);
        ::strftime(buffer, 80, "%d-%m-%Y %H:%M:%S", &timeinfo);
        ss << "[" << buffer << "]";
        //ss << "[" << userTime() << "]";
    } else {
        ss << "[----]";
    }

    return ss.str();
}


/**
 *
 */
std::string OutputLog::genomeMapToString(GenomeMap g)
{
    std::stringstream ss;
    GenomeMap::iterator itr;
    ss << "{";
    unsigned int count = 1;
    for(itr = g.begin(); itr != g.end(); ++itr) {
        ss << itr->first << ": " << itr->second.toString();
        if(count < g.size()) {
            ss << "; ";
        }
        ++count;
    }
    ss << "}";
    return ss.str();
}


/**
 *
 */
std::string OutputLog::stringVectorToString(const StringVector& sv,
                                            const std::string& delim)
{
    std::stringstream ss;

    StringVector::const_iterator itr;
    for(itr = sv.begin(); itr != sv.end(); ++itr) {
        ss << *itr;
        if (std::distance(itr, sv.end()) > 1)
            ss << delim;
    }

    return ss.str();
}


/**
 *
 */
std::string OutputLog::intVectorToString(const IntVector& ivec,
                                         const std::string delim)
{
    std::stringstream ss;

    IntVector::const_iterator it;
    for (it = ivec.begin(); it != ivec.end(); ++it) {
        ss << *it;
        if (std::distance(it, ivec.end()) > 1)
            ss << delim;
    }

    return ss.str();
}


/**
 *
 */
std::string OutputLog::uintVectorToString(const UIntVector& uivec, 
                               const std::string& delim)
{
    std::stringstream ss;
    UIntVector::const_iterator it;
    for (it = uivec.begin(); it != uivec.end(); ++it) {
        ss << *it;
        if (std::distance(it, uivec.end()) > 1)
            ss << delim;
    }

    return ss.str();
}


