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

#ifndef _GGA_VALUE_HPP_
#define _GGA_VALUE_HPP_

/*
 * GGAValue
 * This class can represent either a double, int or string. It may not have
 * a value for all of its representations and in those cases will return a 
 * default value and print an erorr message.
 * Note that there is no "get" method in this class. This is because the return
 * type could be one of double, int, string. The proper way to retrieve a value is to
 * cast this class into the value you wish to retrieve.
 */

#include <cassert>

#include <iosfwd>
#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/scoped_ptr.hpp>


class GGAValue 
{
public:
    enum ValueType { BOOL = 0, LONG = 1, DOUBLE = 2, STRING = 3, UNKNOWN = 4 };

    // public static methods
    static GGAValue getValueFromStr(const std::string& str,
                                    ValueType forceType = UNKNOWN);

    // construct/copy/destruct
    GGAValue();
    GGAValue(const GGAValue&);
    virtual ~GGAValue();

    // value initialization
    GGAValue(bool val);
    GGAValue(long val);
    GGAValue(double val);
    GGAValue(const std::string& val);
    
    // setters
    void setValue(bool val);
    void setValue(long val);
    void setValue(double val);
    void setValue(const std::string& val);

    // getters
    long getLong() const;
    bool getBool() const;
    double getDouble() const;
    const std::string& getString() const;

    // query
    bool hasValue() const;
    bool isLong() const;
    bool isDouble() const;
    bool isString() const;
    bool isBool() const;

    std::string toString() const;    

    GGAValue& operator=(const GGAValue& other);
    bool operator==(const GGAValue& other) const;

private:    
    void resetValues();

    // stream
    friend std::ostream& operator<<(std::ostream& output, const GGAValue& val);

    // serialization
    friend class boost::serialization::access;
    template <class Archiver> void serialize(Archiver&, const unsigned int);
    template <class Archiver> void save(Archiver&, const unsigned int) const;
    template <class Archiver> void load(Archiver&, const unsigned int);

    // Only one value allowed
    ValueType m_type;

    boost::scoped_ptr<bool> m_boolValue;
    boost::scoped_ptr<long> m_longValue;
    boost::scoped_ptr<double> m_dblValue;
    boost::scoped_ptr<std::string> m_strValue;   
};

//==============================================================================
// Public inline methods

/**
 *
 */
inline bool GGAValue::hasValue() const
{ return isLong() || isDouble() || isString() || isBool(); }

/**
 *
 */
inline bool GGAValue::isLong() const 
{ return m_longValue.get() != 0; }

/**
 *
 */
inline bool GGAValue::isDouble() const 
{ return m_dblValue.get() != 0; }

/**
 *
 */
inline bool GGAValue::isString() const 
{ return m_strValue.get() != 0; }

/**
 *
 */
inline bool GGAValue::isBool() const 
{ return m_boolValue.get() != 0; }


//==============================================================================
// serialization

template <class Archiver>
void GGAValue::serialize(Archiver& ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(m_type);    // Save/Load type

    boost::serialization::split_member(ar, *this, version);    
}

template <class Archiver>
void GGAValue::save(Archiver& ar, const unsigned int version) const
{
    switch (m_type) {
        case BOOL:
            assert(isBool());
            ar & BOOST_SERIALIZATION_NVP(*m_boolValue);
            break;
        case LONG:
            assert(isLong());
            ar & BOOST_SERIALIZATION_NVP(*m_longValue);
            break;
        case DOUBLE:
            assert(isDouble());
            ar & BOOST_SERIALIZATION_NVP(*m_dblValue);
            break;
        case STRING:
            assert(isString());
            ar & BOOST_SERIALIZATION_NVP(*m_strValue);
            break;
        case UNKNOWN:
            break;
        default:
            // ERROR: Throw exception or something ;)
            break;
    }
}

template <class Archiver>
void GGAValue::load(Archiver& ar, const unsigned int version)
{
    switch (m_type) {
        case BOOL:
            m_boolValue.reset(new bool());
            ar & BOOST_SERIALIZATION_NVP(*m_boolValue);
            break;
        case LONG:
            m_longValue.reset(new long());
            ar & BOOST_SERIALIZATION_NVP(*m_longValue);
            break;
        case DOUBLE:
            m_dblValue.reset(new double());
            ar & BOOST_SERIALIZATION_NVP(*m_dblValue);
            break;
        case STRING:
            m_strValue.reset(new std::string());
            ar & BOOST_SERIALIZATION_NVP(*m_strValue);
            break;
        case UNKNOWN:
            break;
        default:
            // ERROR: Throw exception or something ;)
            break;
    }
}

#endif // _GGA_VALUE_HPP_
