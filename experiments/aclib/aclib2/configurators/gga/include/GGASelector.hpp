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


#ifndef _GGA_SELECTOR_HPP_
#define _GGA_SELECTOR_HPP_

#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

#include "GGAInstance.hpp"
#include "GGAGenome.hpp"
#include "GGAUtil.hpp"


/* ===== Forward declarations ===== */
class GGASelectorResultBuilder;


/**
 * @brief Immutable selection result.
 */
class GGASelectorResult
{
    friend class GGASelectorResultBuilder;
    
public:
    typedef std::map<GGAInstance, std::vector<double>, GGAInstance::STLMapComparator> 
            InstancePerformancesMap;
    
    
    GGASelectorResult();
    GGASelectorResult(const GGASelectorResult&);
    virtual ~GGASelectorResult();
    
    GGASelectorResult& operator=(const GGASelectorResult&);
    
    int getNumEvaluations() const;
    
    const GGAGenomeVector& getWinners() const;
    
    bool hasInstancePerformance(const GGAInstance&) const;
    const std::vector<double>& getInstancePerformance(const GGAInstance&) const;
        
private:
    GGASelectorResult(int num_evaluations,
                      const GGAGenomeVector& winners, 
                      const InstancePerformancesMap& performances);
    
    // serialization
    friend class boost::serialization::access;
    template <class Archiver> void serialize(Archiver&, const unsigned int);
    
    int num_evaluations_;
    GGAGenomeVector winners_;
    InstancePerformancesMap instancesPerformance_;
};


// [serialization] ---------------------------------------------------------------------------------

template<class Archiver>
void GGASelectorResult::serialize(Archiver& ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(num_evaluations_);
    ar & BOOST_SERIALIZATION_NVP(winners_);
    ar & BOOST_SERIALIZATION_NVP(instancesPerformance_);
}




/**
 * @brief Incrementally constructs a selector result instance.
 */
class GGASelectorResultBuilder
{
public:
    GGASelectorResultBuilder();
    virtual ~GGASelectorResultBuilder();
    
    GGASelectorResultBuilder& addAll(const GGASelectorResult& result);
    
    GGASelectorResultBuilder& addWinner(const GGAGenome& winner);
    GGASelectorResultBuilder& addWinners(const GGAGenomeVector& winners);
    
    GGASelectorResultBuilder& addInstancePerformance(const GGAInstance& instance,
                                                     double performance);
    GGASelectorResultBuilder& addInstancePerformance(const GGAInstance& instance,
                                                     const std::vector<double>& performance);
    
    /** @brief Increases the number of evaluations by the given amount. */
    GGASelectorResultBuilder& increaseNumEvaluations(int num_evaluations);
    
    GGASelectorResult build() const;
    void clear();

    // attribute getters
    int getNumEvaluations() const;
    
private:
    // Intentionally unimplemented
    GGASelectorResultBuilder(const GGASelectorResultBuilder&);
    GGASelectorResultBuilder& operator=(const GGASelectorResultBuilder&);
        
    // attributes
    int num_evaluations_;
    GGAGenomeVector winners_;
    GGASelectorResult::InstancePerformancesMap instancesPerformance_;
};


// public in-line methods
// ------------------------------------------------------------------------------------------------

inline int GGASelectorResultBuilder::getNumEvaluations() const { return num_evaluations_; }



/**
 * @brief Selector interface
 */
class GGASelector
{
public:
    virtual GGASelectorResult select(const GGAGenomeVector& participants,
                                     const GGAInstanceVector& instances,
                                     double timeout = std::numeric_limits<double>::max()) = 0;
};


#endif // _GGA_SELECTOR_HPP_
