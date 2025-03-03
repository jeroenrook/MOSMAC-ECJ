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

/*
 * Class: OutputLog
 * Author: Kevin Tierney
 * Provides simple std out / std err logging capabilities
 */

#ifndef GA_PARAM_CONFIG_OUTPUT_LOG
#define GA_PARAM_CONFIG_OUTPUT_LOG

#include <iostream>
#include <sstream>
#include <string>

#include "ggatypedefs.hpp"

// TODO: Replace with something typesafe?
#define DO_LOG(text, level) {std::ostringstream oss; OutputLog::log((std::ostringstream&)(oss<<text), level);}
#define LOG_NOPRE(text) {std::ostringstream oss; OutputLog::log((std::ostringstream&)(oss<<text), OutputLog::NORMAL, false);}
#define LOG(text) DO_LOG(text, OutputLog::NORMAL)
#define LOG_ERROR(text) DO_LOG(text, OutputLog::ERROR)
#define LOG_VERBOSE(text) DO_LOG(text, OutputLog::VERBOSE)
#define LOG_VERY_VERBOSE(text) DO_LOG(text, OutputLog::VERY_VERBOSE)
#define LOG_DEBUG(text) DO_LOG(text, OutputLog::DEBUG)
#define LOG_ERROR_NOP(text) {std::ostringstream oss; OutputLog::log((std::ostringstream&)(oss<<text), OutputLog::ERROR, false);}

// TODO Thread safety!

class OutputLog {
public:
    
    enum Level {
        NOTHING         = 0,
        ERROR           = 1,
        NORMAL          = 2,
        VERBOSE         = 3,
        VERY_VERBOSE    = 4,
        DEBUG           = 5
    };
    
    OutputLog();
    virtual ~OutputLog();

    static void setReportLevel(Level lvl);
    static Level reportLevel();

    static void setMasterPid();
    static pid_t masterPid();
    
    static void log(const std::ostringstream& os, Level lvl, bool addPrefix = true);
    
    /*
     * Gets the logging output prefix formatted by:
     * [OUTPUT_LEVEL][CPU TIME] msg
     */
    static std::string prefix(Level lvl);

    static std::string genomeMapToString(GenomeMap g);
    static std::string stringVectorToString(const StringVector& sv, 
                                            const std::string& delim = ", ");
    static std::string intVectorToString(const IntVector& ivec,
                                         const std::string delim = ", ");
    static std::string uintVectorToString(const UIntVector& uivec,
                                          const std::string& delim = ", ");

    static Level s_ReportLevel;
    static pid_t s_masterPid;
};

#endif
