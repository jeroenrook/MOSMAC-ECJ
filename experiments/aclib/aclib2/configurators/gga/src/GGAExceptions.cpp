//
// This file is part of DGGA.
// 
// The MIT License (MIT)
// 
// Copyright (c) 2015 Kevin Tierney and Josep Pon
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

#include <cerrno>
#include <cstring>
#include <sstream>

#include "GGAExceptions.hpp"
#include "OutputLog.hpp"

//
// === GGAException ===
//

/**
 * @brief Initializes an instance of GGAException with an empty error message.
 */
GGAException::GGAException()
    : m_message()
{ }


/**
 * @brief Initializes an instance of GGAException as a copy of another one.
 */
GGAException::GGAException(const GGAException& other)
    : m_message(other.m_message)
{ }


/**
 * @brief Initializes an instance of GGAException with the given error message.
 */
GGAException::GGAException(const std::string& err_msg)
    : m_message(err_msg)
{ }


/**
 *
 */
GGAException::~GGAException() throw()
{ }


/**
 * @return The c-string representation of the excpetion's message.
 */
const char* GGAException::what() const throw()
{ return m_message.c_str(); }


/**
 * @return The exception's error message.
 */
const std::string& GGAException::message() const
{ return m_message; }


/**
 * @brief Sets the exception's error message.
 */
void GGAException::message(const std::string& message)
{ m_message = message; }


//
// === GGAMalformedInstanceException ===
//

GGAMalformedInstanceException::GGAMalformedInstanceException(
        const std::string& inst)
    : GGAException(inst)
{ }


GGAMalformedInstanceException::GGAMalformedInstanceException(
        const std::string& inst,
        const std::string& file,
        int line) 
    : GGAException()
{
    std::stringstream ss;

    ss << "Malformed Instance";
    if (line > -1)
        ss << " at line " << line;
    if (!file.empty())
        ss << " in " << file;
    ss << ": " << inst << std::endl; 
    ss << "Proper formatting is: [seed] <instance> [extra parameter 1 ... n]";

    message(ss.str());
}

//
// === GGAFileNotFoundException ===
//

GGAFileNotFoundException::GGAFileNotFoundException(const std::string& file) 
        : GGAException("File not found: " + file)
{ }


//
// === GGAParameterException ===
//

GGAParameterException::GGAParameterException(const std::string& prob)
    : GGAException(prob)
{ }


//
// === GGAOptionsException ===
//

GGAOptionsException::GGAOptionsException(const std::string& prob) 
    : GGAException(prob)
{ }


//
// === GGAPopulationException ===
//

GGAPopulationException::GGAPopulationException(const std::string& prob) 
    : GGAException("Population exception: " + prob) 
{ }


//
// === GGAXMLParseException ===
//

GGAXMLParseException::GGAXMLParseException(const std::string& msg)
    : GGAException(msg)
{ }


//
// === GGACommandException ===
//

GGACommandException::GGACommandException(const std::string& msg)
    : GGAException(msg)
{ }


//
// === GGAScenarioFileException ===
//

GGAScenarioFileException::GGAScenarioFileException(const std::string& msg)
  : GGAException(msg)
{ }


//
// === GGAInterruptedException ===
//

GGAInterruptedException::GGAInterruptedException(const std::string& msg)
  : GGAException(msg)
{ }


//
// === GGANullPointerException ===
//

GGANullPointerException::GGANullPointerException(const std::string& msg)
    : GGAException(msg)
{ }
