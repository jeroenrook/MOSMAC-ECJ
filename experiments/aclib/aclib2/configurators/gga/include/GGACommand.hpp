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

#ifndef _GGA_COMMAND_HPP_
#define _GGA_COMMAND_HPP_

#include <string>
#include <vector>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

#include "ggatypedefs.hpp"
#include "GGAInstance.hpp"
#include "GGAParameter.hpp"


class GGACommandArg
{
public:
    enum ArgType {VARIABLE, FIXED};

    // construct/copy/destruct
    GGACommandArg();
    GGACommandArg(const GGACommandArg&);
    GGACommandArg(ArgType type, const std::string& val);
    ~GGACommandArg();

    ArgType type() const;
    void type(ArgType type);

    const std::string& name() const;
    void name(const std::string& name);

    bool isProtected() const;

    const std::string& fixedValue() const;
    
    std::string str() const;

private:
    // serialization
    friend class boost::serialization::access;
    template<class Archiver>void serialize(Archiver&, const unsigned int);

    //
    ArgType m_type;
    std::string m_name;
    std::string m_fixedValue;
    bool m_protected;
};

//==============================================================================
// GGACommandArg typedefs

typedef std::vector<GGACommandArg> CommandArgVector;


/*
 * Represents the command to call the target algorithm from GGA.
 * Variables are set based on their names using a BASH style, 
 * e.g. $this or ${this}
 * Special variables: $seed, $instance
 */
class GGACommand {
public:
    // Static variables
    static StringVector protectedNames();
    
    // construct/destruct
    GGACommand(const std::string& cmd);
    ~GGACommand();
    
    const std::string& rawCommand() const;
    const CommandArgVector& arguments() const;

    void outputTrajectory(const GenomeMap& genome, double utime, double wtime);

    bool isOrPathOk(const GGAParameter::pointer param, 
                    const GenomeMap& genome) const;

    bool validateVarNames(const ParameterMap& params);

    std::string str() const;

private:
    static StringVector* s_protectedNames;

    GGACommand(); // Implemented for serialization
    GGACommand(const GGACommand&); // Intentionally unimplemented

    void parseCommand(const std::string& cmd);

    // serialization
    friend class boost::serialization::access;
    template<class Archiver>void serialize(Archiver&, const unsigned int);

    //
    std::string m_execCommand;
    std::string m_rawCommand;
    CommandArgVector m_commandArgs;
    bool m_parseSuccessful;    
};


//==============================================================================
// GGACommandArg public inline methods

/**
 *
 */
inline GGACommandArg::ArgType GGACommandArg::type() const 
{    return m_type;    }

/**
 *
 */
inline void GGACommandArg::type(GGACommandArg::ArgType type)
{    m_type = type;    }

/**
 *
 */
inline const std::string& GGACommandArg::name() const 
{    return m_name;    }

/**
 *
 */
inline void GGACommandArg::name(const std::string& name)
{    m_name = name;    }

/**
 *
 */
inline bool GGACommandArg::isProtected() const
{    return m_protected;    }

/**
 *
 */
inline const std::string& GGACommandArg::fixedValue() const
{    return m_fixedValue;    }


//==============================================================================
// GGACommandArg private inline/template methods

template<typename Archiver>
void GGACommandArg::serialize(Archiver& ar, const unsigned int version)
{
    ar & m_type;
    ar & m_name;
    ar & m_fixedValue;
    ar & m_protected;
}


//==============================================================================
// GGACommand public inline methods

/**
 *
 */
inline const std::string& GGACommand::rawCommand() const
{    return m_rawCommand;    }

/**
 *
 */
inline const CommandArgVector& GGACommand::arguments() const
{    return m_commandArgs;    }


//==============================================================================
// GGACommand private inline/template methods

template<typename Archiver>
void GGACommand::serialize(Archiver& ar, const unsigned int version)
{
    ar & m_execCommand;
    ar & m_rawCommand;
    ar & m_commandArgs;
    ar & m_parseSuccessful; 
}


#endif // _GGA_COMMAND_HPP_
