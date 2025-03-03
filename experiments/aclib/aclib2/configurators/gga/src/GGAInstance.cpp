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

#include <fstream>
#include <sstream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/functional/hash.hpp>

#include "GGAExceptions.hpp"
#include "GGAInstance.hpp"
#include "GGAOptions.hpp"
#include "OutputLog.hpp"

//==============================================================================
// GGAInstance public static methods

/**
 * Creates a GGAInstance from a line in an instance-seed file taking the 
 * following format:
 *                     <seed> <instance> <extra1 ... extra n>
 *
 * extra could be anything, perhaps optimal values for those instances, correct
 * program output, etc
 */
GGAInstance GGAInstance::fromString(const std::string& inst_str)
{
    const GGAOptions& opts = GGAOptions::instance();
    int seed;
    std::string instance;
    StringVector extra;

    std::stringstream ss(inst_str);

    if (!(ss >> seed)) { // NC_DETERMINISTIC
        ss.str("");
        ss.clear();
        ss << inst_str; 
        seed = 1;
    }

    if (!(ss >> instance))
        throw GGAMalformedInstanceException(inst_str);
    boost::algorithm::trim(instance);

    std::string tmp;
    while (ss >> tmp) {
        boost::algorithm::trim(tmp);
        extra.push_back(tmp);
    }
    
    return GGAInstance(seed, opts.target_algo_cpu_limit, instance, extra);
}

//==============================================================================
// GGAInstance public methods

GGAInstance::GGAInstance() 
    : m_seed(-1)
    , instance_()
    , extra_()
{ }

GGAInstance::GGAInstance(const GGAInstance& other)
    : m_seed(other.m_seed)
    , instance_(other.instance_)
    , extra_(other.extra_)
{ }

GGAInstance::GGAInstance(int seed, double timeout, const std::string& instance,
                         const StringVector& extra) 
    : m_seed(seed)
    , instance_(instance)
    , extra_(extra) 
{ }

GGAInstance::~GGAInstance() 
{ }

GGAInstance& GGAInstance::operator=(const GGAInstance& o)
{
    m_seed = o.m_seed;
    instance_ = o.instance_;
    extra_ = o.extra_;

    return *this;
}

std::string GGAInstance::toString() const
{
    std::stringstream ss;
    ss << "[GGAInstance: " << this << "; Seed: " << m_seed 
       << "; Instance: " << instance_ << "; Extra: " <<
       OutputLog::stringVectorToString(extra_) << "]";
    return ss.str();
}


// =====================================
// === GGAInstance::STLMapComparator ===
// =====================================

/**
 *
 */
bool GGAInstance::STLMapComparator::operator()(const GGAInstance& inst1,
                                               const GGAInstance& inst2) const
{
    static boost::hash<std::string> string_hash;
    static boost::hash<StringVector> strvec_hash;
    static boost::hash<int> int_hash;

    size_t seed1 = 0;
    boost::hash_combine(seed1, string_hash(inst1.getInstance()));
    boost::hash_combine(seed1, int_hash(inst1.getSeed()));
    boost::hash_combine(seed1, strvec_hash(inst1.getExtra()));

    size_t seed2 = 0;
    boost::hash_combine(seed2, string_hash(inst2.getInstance()));
    boost::hash_combine(seed2, int_hash(inst2.getSeed()));
    boost::hash_combine(seed2, strvec_hash(inst2.getExtra()));

    return seed1 < seed2;
}


// =========================== //
// ===== GGAInstanceInfo ===== //
// =========================== //

GGAInstanceInfo::GGAInstanceInfo()
    : estimatedExecutionTime_(-1)
{ }

GGAInstanceInfo::GGAInstanceInfo(const GGAInstanceInfo& orig)
    : estimatedExecutionTime_(orig.estimatedExecutionTime_)
{ }

GGAInstanceInfo::~GGAInstanceInfo()
{ }

GGAInstanceInfo& GGAInstanceInfo::operator=(const GGAInstanceInfo& orig)
{
    estimatedExecutionTime_ = orig.estimatedExecutionTime_;
    return *this;
}
