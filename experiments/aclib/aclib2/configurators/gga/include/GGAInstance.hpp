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

#ifndef _GGA_INSTANCE_HPP_
#define _GGA_INSTANCE_HPP_

#include <exception>
#include <string>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include "ggatypedefs.hpp"

/**
 * @brief User provided instance.
 */
class GGAInstance {
public:

    // Creates a new instances from a line in an instance-seed file.
    static GGAInstance fromString(const std::string& inst_str);

    // construct/copy/destruct
    GGAInstance();
    GGAInstance(const GGAInstance&);
    GGAInstance(int seed, double timeout, const std::string& instance,
                const StringVector& extra);
    virtual ~GGAInstance();
    
    const std::string& getInstance() const;
    void setInstance(const std::string& instance);

    int getSeed() const;
    void setSeed(int seed);

    const StringVector& getExtra() const;
    void setExtra(const StringVector& extra);

    std::string toString() const;

    GGAInstance& operator=(const GGAInstance&);
    bool operator==(const GGAInstance&) const;

    struct STLMapComparator {
        bool operator()(const GGAInstance&, const GGAInstance&) const;
    };

private:
    // serialization
    friend class boost::serialization::access;
    template <class Archiver> void serialize(Archiver& ar, const unsigned int version);

    //
    int m_seed;
    std::string instance_;    
    StringVector extra_;    
};

// typedef (avoids cyclic dependencies of the old typedefs.h)
typedef std::vector<GGAInstance> GGAInstanceVector;
typedef std::map<GGAInstance, double, GGAInstance::STLMapComparator> GGAInstancePerformanceMap;

//==============================================================================
// GGAInstance public in-line methods

inline const std::string& GGAInstance::getInstance() const        { return instance_; }
inline void GGAInstance::setInstance(const std::string& instance) { instance_ = instance; }


inline int GGAInstance::getSeed() const    { return m_seed; }
inline void GGAInstance::setSeed(int seed) { m_seed = seed; }

inline const StringVector& GGAInstance::getExtra() const     { return extra_; }
inline void GGAInstance::setExtra(const StringVector& extra) { extra_ = extra; }


/**
 *
 */
inline bool GGAInstance::operator==(const GGAInstance& o) const
{
    return instance_ == o.instance_ && m_seed == o.m_seed &&
           extra_ == o.extra_;
}

//==============================================================================
// GGAInstance serialization

template <class Archiver>
void GGAInstance::serialize(Archiver& ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(instance_);
    ar & BOOST_SERIALIZATION_NVP(m_seed);
    ar & BOOST_SERIALIZATION_NVP(extra_);
}



/**
 * @brief Additional instance information.
 */
class GGAInstanceInfo
{
public:
    // constructor/copy/destructor
    GGAInstanceInfo();
    GGAInstanceInfo(const GGAInstanceInfo&);
    virtual ~GGAInstanceInfo();
    
    GGAInstanceInfo& operator=(const GGAInstanceInfo&);
    
    double getEstimatedExecutionTime() const;
    void setEstimatedExecutionTime(double t);
    
private:
    // serialization
    friend class boost::serialization::access;
    template <class Archiver> void serialize(Archiver& ar, const unsigned int version);
    
    // attributes
    double estimatedExecutionTime_;
};

//==================================================================================================
// typedef
typedef std::map<GGAInstance, GGAInstanceInfo, GGAInstance::STLMapComparator> GGAInstanceInfoMap;

//==================================================================================================
// GGAInstanceInfo public in-line methods

inline double GGAInstanceInfo::getEstimatedExecutionTime() const { return estimatedExecutionTime_; }
inline void GGAInstanceInfo::setEstimatedExecutionTime(double t) { estimatedExecutionTime_ = t; }

#endif // _GGA_INSTANCE_HPP_
