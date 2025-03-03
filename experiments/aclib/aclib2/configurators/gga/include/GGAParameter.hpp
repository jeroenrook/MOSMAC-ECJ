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


#ifndef _GGA_PARAMETER_HPP_
#define _GGA_PARAMETER_HPP_

#include <iostream>
#include <string>
#include <vector>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/shared_ptr.hpp>

#include "ggatypedefs.hpp"
#include "GGAValue.hpp"
#include "OutputLog.hpp"


/*
 * GGAParameter
 * This class represents a parameter accepted by the target algorithm.
 * Parameters can be of type: DISCRETE, CONTINUOUS, or CATEGORICAL
 * and have a valid domain. Note that the domain is inclusive.
 */
class GGAParameter
{
public:
    typedef boost::shared_ptr<GGAParameter> pointer;
    typedef boost::shared_ptr<const GGAParameter> const_pointer;

    // TODO Support bool input. For now, this is just used internally
    enum Type {DISCRETE, CONTINUOUS, CATEGORICAL, BOOL, UNKNOWN};

    // static methods
    static std::string genUniqueVarName();
    // This isn't used for parameters anymore, but maybe it will be of use somewhere else...
    static std::string genRandomName();

    // construct/copy/destruct
    GGAParameter(Type t);    
    GGAParameter(const StringVector& domain, const std::string& name,
                 const std::string& prefix);
    GGAParameter(GGAParameter::Type t, const GGAValue& start, 
                 const GGAValue& end, const std::string& name,
                 const std::string& prefix);
    virtual ~GGAParameter();

    // Attributes access/modification
    void type(GGAParameter::Type t);
    Type type() const;

    void rangeStart(const GGAValue& start);
    const GGAValue& rangeStart();

    void rangeEnd(const GGAValue& end);
    const GGAValue& rangeEnd();

    void categoricalDomain(const StringVector& domain);
    const StringVector& categoricalDomain() const;

    bool isValueInRange(const GGAValue& val) const;

    const std::string& name() const;
    void name(const std::string& name);

    const std::string& trajName() const;
    void trajName(const std::string& trajName);

    const std::string& prefix() const;
    void prefix(const std::string& prefix);

    bool isFlag() const;
    void isFlag(bool isFlag);

    pointer orParent() const;
    void orParent(pointer param);

    const GGAValue& orValue() const;
    void orValue(const GGAValue& orValue);

    bool isTrivial() const; // returns whether the domain of this parameter is the empty set or not
    
    GGAValue valueInRange() const;
    
    std::string info() const;

private:
    GGAParameter(); // Implemented for serialization

    // stream operator
    friend std::ostream& operator<<(std::ostream& output,
                                    const GGAParameter& param);

    // serialization
    friend class boost::serialization::access;
    template<class Archiver>void serialize(Archiver&, const unsigned int);
    
    //
    static int s_varCounter;

    Type m_type;

    bool m_isFlag;
    GGAValue m_rangeStart;
    GGAValue m_rangeEnd;
    StringVector m_cDomain;   

    std::string m_name;
    std::string m_trajName;
    std::string m_prefix;   

    // Pointer to the next OR node ancestor (or NULL if there is none)
    pointer m_orParent;
    // Value of m_orParent that triggers this branch
    GGAValue m_orValue;
};

//==============================================================================
// GGAParameter typedefs
typedef std::map<std::string, GGAParameter::pointer> ParameterMap;

//==============================================================================
// GGAParameter public inline methods

/**
 *
 */
inline void GGAParameter::type(GGAParameter::Type t)
{
    m_type = t;
}

/**
 *
 */
inline GGAParameter::Type GGAParameter::type() const
{
    return m_type;
}

/**
 *
 */
inline void GGAParameter::rangeStart(const GGAValue& start)
{
    m_rangeStart = start;
}

/**
 *
 */
inline const GGAValue& GGAParameter::rangeStart()
{
    return m_rangeStart;
}

/**
 *
 */
inline void GGAParameter::rangeEnd(const GGAValue& end)
{
    m_rangeEnd = end;
}

/**
 *
 */
inline void GGAParameter::categoricalDomain(const StringVector& domain) 
{
    m_cDomain = domain;
    m_rangeStart = GGAValue(0L);
    m_rangeEnd = GGAValue(static_cast<long>(domain.size()) - 1);
}

/**
 *
 */
inline const StringVector& GGAParameter::categoricalDomain() const 
{ return m_cDomain; }

/**
 *
 */
inline const GGAValue& GGAParameter::rangeEnd() 
{ return m_rangeEnd; }

/**
 *
 */
inline const std::string& GGAParameter::name() const
{ return m_name; }

/**
 *
 */
inline void GGAParameter::name(const std::string& name)
{ m_name = name; }

/**
 *
 */
inline const std::string& GGAParameter::trajName() const 
{ return m_trajName; }

/**
 *
 */
inline void GGAParameter::trajName(const std::string& trajName)
{ m_trajName = trajName; }

/**
 *
 */
inline const std::string& GGAParameter::prefix() const
{ return m_prefix; }

/**
 *
 */
inline void GGAParameter::prefix(const std::string& prefix)
{ m_prefix = prefix; }

/**
 *
 */
inline bool GGAParameter::isFlag() const 
{ return m_isFlag; }

/**
 *
 */
inline void GGAParameter::isFlag(bool isFlag) 
{ m_isFlag = isFlag; }

/**
 *
 */
inline GGAParameter::pointer GGAParameter::orParent() const 
{ return m_orParent; }

/**
 *
 */
inline void GGAParameter::orParent(pointer param) 
{ m_orParent = param; }

/**
 *
 */
inline const GGAValue& GGAParameter::orValue() const
{ return m_orValue; }

/**
 *
 */
inline void GGAParameter::orValue(const GGAValue& orValue)
{ m_orValue = orValue; }


//==============================================================================
// GGAParameter private inline/template methods

template<typename Archiver>
void GGAParameter::serialize(Archiver& ar, const unsigned int version)
{
    ar & m_type;
    ar & m_isFlag;
    ar & m_rangeStart;
    ar & m_rangeEnd;
    ar & m_cDomain;
    ar & m_name;
    ar & m_trajName;
    ar & m_prefix;
    ar & m_orParent;
    ar & m_orValue;

    // Save/Restore s_varCounter
    int temp_varCounter = s_varCounter;
    ar & temp_varCounter;    
    s_varCounter = s_varCounter > temp_varCounter ? 
                                    s_varCounter : temp_varCounter;
}

#endif // _GGA_PARAMETER_HPP_
