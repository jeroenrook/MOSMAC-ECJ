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


#include <cstdlib>

#include <algorithm>
#include <sstream>
#include <string>

#include "GGAExceptions.hpp"
#include "GGAParameter.hpp"
#include "GGAValue.hpp"
#include "OutputLog.hpp"
#include "GGARandEngine.hpp"

//==============================================================================
// GGAParameter private static variables

int GGAParameter::s_varCounter = 0;

//==============================================================================
// GGAParameter public static methds

/**
 *
 */
std::string GGAParameter::genUniqueVarName() {
    // Warning: Unsynchronized. I don't think there's any need for this to be thread safe, though
    std::stringstream ss;
    ss << "___UNNAMEDVAR" << s_varCounter;
    ++s_varCounter;
    return ss.str();
}

/**
 *
 */
std::string GGAParameter::genRandomName() {
    std::string ret;
    for(int i = 0; i < 4; ++i) {
        ret += (char)(GGARandEngine::randInt('a','z'));
        // I realize this is totally unnecessary. I'm doing it anyway.
        char toAdd = 'a';
        switch(GGARandEngine::randInt(0,4)) {
            case 0:
                toAdd = 'a';
                break;
            case 1:
                toAdd = 'e';
                break;
            case 2:
                toAdd = 'i';
                break;
            case 3:
                toAdd = 'o';
                break;
            case 4:
                toAdd = 'u';
                break;
            default:
                toAdd = 'a';
        }
        ret += toAdd;
    }
    return ret;
}


//==============================================================================
// GGAParameter public methods

/**
 *
 */
GGAParameter::GGAParameter(GGAParameter::Type t) 
    : m_type(t)
    , m_isFlag(false)
    , m_rangeStart()
    , m_rangeEnd()
    , m_cDomain()
    , m_name(genUniqueVarName())
    , m_trajName(m_name)
    , m_prefix()
    , m_orParent(static_cast<GGAParameter*>(NULL))
    , m_orValue()
{ }


/**
 * Represents a categorical parameter with the given domain of string values.
 */
GGAParameter::GGAParameter(const StringVector& domain, const std::string& name, 
                           const std::string& prefix) 
    : m_type(CATEGORICAL)
    , m_isFlag(false)
    , m_rangeStart(0L)
    , m_rangeEnd(static_cast<long>(domain.size() - 1))
    , m_cDomain(domain)
    , m_name(name)
    , m_trajName(name)
    , m_prefix(prefix)
    , m_orParent(static_cast<GGAParameter*>(NULL))
    , m_orValue()
{ }


/*
 * Represents a parameter of type t, with a range of [start, end], and a name.
 */
GGAParameter::GGAParameter(GGAParameter::Type t, const GGAValue& start, 
                           const GGAValue& end, const std::string& name,
                           const std::string& prefix)
    : m_type(t)
    , m_isFlag(false)
    , m_rangeStart(start)
    , m_rangeEnd(end)
    , m_cDomain()
    , m_name(name)
    , m_trajName(name)
    , m_prefix(prefix)
    , m_orParent(static_cast<GGAParameter*>(NULL))
    , m_orValue()
{
    if(t == CATEGORICAL) {
        throw GGAParameterException("Error: Invalid initialization of"
            " GGAParameter with a categorical type (Called discrete/continuous"
            " constructor)");
    }
}


/**
 *
 */
GGAParameter::~GGAParameter() 
{ }


/**
 *
 */
bool GGAParameter::isValueInRange(const GGAValue& val) const {
    bool retval = true;
    if(m_type == DISCRETE) {
        long ival = val.getLong();
        long start = m_rangeStart.getLong();
        long end = m_rangeEnd.getLong();
        retval = start <= ival && end >= ival;
    } else if(m_type == CONTINUOUS) {
        double dval = val.getDouble();
        double start = m_rangeStart.getDouble();
        double end = m_rangeEnd.getDouble();
        retval = start <= dval && end >= dval;
    } else if(m_type == CATEGORICAL) {
        retval = std::find(m_cDomain.begin(), m_cDomain.end(), val.getString()) 
                    != m_cDomain.end();
    } else {
        retval = false;
    }
    return retval;
}


/**
 * Returns a value selected uniformly at random within the current 
 * range of the parameter
 */
GGAValue GGAParameter::valueInRange() const {

    if(m_type == DISCRETE || m_type == CATEGORICAL) {
        long start = m_rangeStart.getLong();
        long end = m_rangeEnd.getLong();
        long lrand = GGARandEngine::randLong(start, end);
        if(m_type == DISCRETE) {
            return GGAValue(lrand);
        } else {
            return GGAValue(m_cDomain[lrand]);
        }

    } else if(m_type == CONTINUOUS) {
        double start = m_rangeStart.getDouble();
        double end = m_rangeEnd.getDouble();
        return GGAValue(GGARandEngine::randDouble(start, end));

    } else {
        LOG_ERROR("Warning: GGAParameter type invalid, returning null pointer from valueInRange().");
        LOG_ERROR(info());
    }
    return GGAValue();
}


/**
 *
 */
std::string GGAParameter::info() const 
{
    std::stringstream ss;
    ss << "(GGAParameter)[Name: " << m_name << "; Type: ";

    if(m_type == DISCRETE)
        ss << "Discrete";
    else if(m_type == CONTINUOUS)
        ss << "Continuous";
    else if(m_type == CATEGORICAL)
        ss << "Categorical";
    else
        ss << "Unknown";

    if(m_type == DISCRETE) {
        ss << "; Start/End: ";
        ss << m_rangeStart.getLong();
        ss << "/";
        ss << m_rangeEnd.getLong();
    } else if(m_type == CONTINUOUS) {
        ss << "; Start/End: ";
        ss << m_rangeStart.getDouble();
        ss << "/";
        ss << m_rangeEnd.getDouble();
    }
    
    if(m_type == CATEGORICAL) {
        StringVector::const_iterator itr;
        ss << "; Domain: {";
        for(itr = m_cDomain.begin(); itr != m_cDomain.end(); ++itr) {
            ss << *itr;
            if(itr + 1 != m_cDomain.end()) {
                ss << ", ";
            }
        }
        ss << "}";
    }

    if(m_orParent != NULL) {
        ss << " orParent: " << m_orParent;
        if(m_orValue.hasValue()) {
            ss << "; orValue: " << m_orValue.toString();
        }
    }
    
    ss << "]";
    return ss.str();
}


//==============================================================================
// GGAParameter private methods

/**
 * Implemented for serialization
 */
GGAParameter::GGAParameter() 
    : m_type()
    , m_isFlag(false)
    , m_rangeStart()
    , m_rangeEnd()
    , m_cDomain()
    , m_name()
    , m_trajName()
    , m_prefix()
    , m_orParent(static_cast<GGAParameter*>(NULL))
    , m_orValue()
{ }


/**
 *
 */
std::ostream& operator<<(std::ostream& output, const GGAParameter& param)
{
    output << param.info();
    return output;
}
