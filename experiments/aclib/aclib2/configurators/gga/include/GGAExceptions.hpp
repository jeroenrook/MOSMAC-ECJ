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


#ifndef _GGA_EXCEPTIONS_HPP_
#define _GGA_EXCEPTIONS_HPP_

#include <exception>
#include <string>

/**
 * @brief Base GGAException
 */
class GGAException : public std::exception 
{
public:
    GGAException();
    GGAException(const GGAException&);
    GGAException(const std::string&);
    virtual ~GGAException() throw();

    virtual const char* what() const throw();

    const std::string& message() const;
protected:
    void message(const std::string&);
private:
    std::string m_message;
};

/**
 *
 */
class GGAMalformedInstanceException : public GGAException
{
public:
    GGAMalformedInstanceException(const std::string& inst);
    GGAMalformedInstanceException(const std::string& inst, const std::string& file,
                               int line);
};

/**
 *
 */
class GGAFileNotFoundException : public GGAException
{
public:
    GGAFileNotFoundException(const std::string& file);
};

/**
 *
 */
class GGAOptionsException : public GGAException
{
public:
    GGAOptionsException(const std::string& prob);
};

/**
 *
 */
class GGAParameterException : public GGAException
{
public:
    GGAParameterException(const std::string& prob);
};

/**
 *
 */
class GGAPopulationException : public GGAException
{
public:
    GGAPopulationException(const std::string& prob);
};

/**
 *
 */
class GGAXMLParseException : public GGAException
{
public:
    GGAXMLParseException(const std::string& msg);
};

/**
 *
 */
class GGACommandException : public GGAException
{
public:
    GGACommandException(const std::string& msg);
};

/**
 *
 */
class GGAScenarioFileException : public GGAException
{
public:
    GGAScenarioFileException(const std::string& msg);
};

/**
 *
 */
class GGAInterruptedException : public GGAException
{
public:
    GGAInterruptedException(const std::string& msg);
};

/**
 *
 */
class GGANullPointerException : public GGAException
{
public:
    GGANullPointerException(const std::string& msg);
};

#endif // _GGA_EXCEPTIONS_HPP_
