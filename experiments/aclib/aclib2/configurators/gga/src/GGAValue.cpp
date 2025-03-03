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


#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include "GGAValue.hpp"
#include "OutputLog.hpp"


//==============================================================================
// GGAValue public static methods

/**
 * Converts the string into the appropriate GGAValue, forcing the type to
 * be forceType if it is set to anything other than UNKNOWN.
 */
GGAValue GGAValue::getValueFromStr(const std::string& str, ValueType forceType)
{
    // TODO bool support
    boost::regex dblP("^-?\\d+\\.\\d+((e|E)(\\+?|-)\\d+)?$");
    boost::regex intP("^-?\\d+$");
    

    if (forceType == LONG 
            || (boost::regex_match(str, intP) && forceType == UNKNOWN)) {
        return GGAValue(boost::lexical_cast<long>(str));
    } else if(forceType == DOUBLE 
            || (boost::regex_match(str, dblP) && forceType == UNKNOWN)) {
        return GGAValue(boost::lexical_cast<double>(str));
    } else {
        return GGAValue(str);
    }
}


//==============================================================================
// GGAValue public methods

/**
 *
 */
GGAValue::GGAValue()
    : m_type(UNKNOWN)
    , m_boolValue()
    , m_longValue()
    , m_dblValue()
    , m_strValue()
{ }


/**
 *
 */
GGAValue::GGAValue(const GGAValue& cpy)
    : m_type(cpy.m_type)
    , m_boolValue(m_type == BOOL ? new bool(*cpy.m_boolValue) : 0)
    , m_longValue(m_type == LONG ? new long(*cpy.m_longValue) : 0)
    , m_dblValue(m_type == DOUBLE ? new double(*cpy.m_dblValue) : 0)
    , m_strValue(m_type == STRING ? new std::string(*cpy.m_strValue) : 0)
    
{ }

/**
 *
 */
GGAValue::GGAValue(bool val) 
    : m_type(BOOL)
    , m_boolValue(new bool(val))
    , m_longValue()
    , m_dblValue()
    , m_strValue() 
{ }

/**
 *
 */
GGAValue::GGAValue(long val)
    : m_type(LONG)
    , m_boolValue()
    , m_longValue(new long(val))
    , m_dblValue()
    , m_strValue()
{ }


/**
 *
 */
GGAValue::GGAValue(double val) 
    : m_type(DOUBLE)
    , m_boolValue()
    , m_longValue()
    , m_dblValue(new double(val))
    , m_strValue()    
{ }

/**
 *
 */
GGAValue::GGAValue(const std::string& val)
    : m_type(STRING)
    , m_boolValue()
    , m_longValue()
    , m_dblValue()
    , m_strValue(new std::string(val))
{ }


/**
 *
 */
GGAValue::~GGAValue() 
{ }


/**
 *
 */
void GGAValue::setValue(bool val) 
{
    resetValues();

    m_boolValue.reset(new bool(val));
    m_type = BOOL;
}


/**
 *
 */
void GGAValue::setValue(long val)
{
    resetValues();

    m_longValue.reset(new long(val));
    m_type = LONG;
}



/**
 *
 */
void GGAValue::setValue(double val) 
{
    resetValues();
    
    m_dblValue.reset(new double(val));
    m_type = DOUBLE;
}


/**
 *
 */
void GGAValue::setValue(const std::string& val) 
{
    resetValues();

    m_strValue.reset(new std::string(val));
    m_type = STRING;
}


/**
 *
 */
bool GGAValue::getBool() const
{
    if(isBool())
        return *m_boolValue;
    else
        throw std::domain_error("Attempted to access a bool value in GGAValue"
            "that is not storing a bool value.");
}


/**
 *
 */
long GGAValue::getLong() const
{
    if(isLong())
        return *m_longValue;
    else
        throw std::domain_error("Attempted to access a long value in GGAValue"
            "that is not storing a long value.");
}


/**
 *
 */
double GGAValue::getDouble() const
{
    if(isDouble())
        return *m_dblValue;
    else
        throw std::domain_error("Attempted to access a double value in GGAValue"
            "that is not storing a double value.");
}


/**
 *
 */
const std::string& GGAValue::getString() const
{
    if(isString())
        return *m_strValue;
    else
        throw std::domain_error("Attempted to access a string value in GGAValue"
            "that is not storing a string value.");
}


/**
 *
 */
std::string GGAValue::toString() const {
    std::stringstream ss;
    if(isLong() && m_longValue.get())
        ss << *m_longValue;
    else if(isBool() && m_boolValue.get())
        ss << *m_boolValue;
    else if(isDouble() && m_dblValue.get())
        ss << *m_dblValue;
    else if(isString() && m_strValue.get())
        ss << *m_strValue;

    return ss.str();
}


/**
 *
 */
GGAValue& GGAValue::operator=(const GGAValue& cpy)
{
    resetValues();
    m_type = cpy.m_type;

    switch (cpy.m_type) {
        case BOOL:
            m_boolValue.reset(new bool(*cpy.m_boolValue));
            break;
        case LONG:
            m_longValue.reset(new long(*cpy.m_longValue));
            break;
        case DOUBLE:
            m_dblValue.reset(new double(*cpy.m_dblValue));
            break;
        case STRING:
            m_strValue.reset(new std::string(*cpy.m_strValue));
            break;
        default:
            break;
    }

    return *this;
}


/**
 *
 */
bool GGAValue::operator==(const GGAValue& other) const
{
    if(isLong() && other.isLong())
        return getLong() == other.getLong();
    if(isBool() && other.isBool())
        return getBool() == other.getBool();
    if(isDouble() && other.isDouble())
        return getDouble() == other.getDouble();
    if(isString() && other.isString())
        return getString() == other.getString();

    return false;
}


//==============================================================================
// GGAValue private methods

/**
 *
 */
void GGAValue::resetValues()
{
    m_type = UNKNOWN;

    m_longValue.reset();
    m_boolValue.reset();
    m_dblValue.reset();
    m_strValue.reset();
}

//==============================================================================
// GGAValue stream

std::ostream& operator<<(std::ostream& output, const GGAValue& val) {
    output << val.toString();
    return output;
}
