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



#include "GGASelector.hpp"


// ----------------- //
// GGASelectorResult //
// ----------------- //

GGASelectorResult::GGASelectorResult()
    : num_evaluations_(0)
    , winners_()
    , instancesPerformance_()
{ }

GGASelectorResult::GGASelectorResult(const GGASelectorResult& o)
    : num_evaluations_(o.num_evaluations_)
    , winners_(o.winners_)
    , instancesPerformance_(o.instancesPerformance_)
{ }

GGASelectorResult::~GGASelectorResult()
{ }
    
GGASelectorResult& GGASelectorResult::operator=(const GGASelectorResult& o)
{
    this->num_evaluations_ = o.num_evaluations_;
    this->winners_ = o.winners_;
    this->instancesPerformance_ = o.instancesPerformance_;
    
    return *this;
}

int GGASelectorResult::getNumEvaluations() const             { return num_evaluations_; }
const GGAGenomeVector& GGASelectorResult::getWinners() const { return winners_; }

bool GGASelectorResult::hasInstancePerformance(const GGAInstance& instance) const 
{ return instancesPerformance_.count(instance) > 0; }

const std::vector<double>& 
GGASelectorResult::getInstancePerformance(const GGAInstance& instance) const 
{ return mapAt(instancesPerformance_, instance); }


// private methods
// -------------------------------------------------------------------------------------------------

GGASelectorResult::GGASelectorResult(int num_evaluations,
                                     const GGAGenomeVector& winners,
                                     const InstancePerformancesMap& performance)
    : num_evaluations_(num_evaluations)
    , winners_(winners)
    , instancesPerformance_(performance)
{ }


// ------------------------ //
// GGASelectorResultBuilder //
// ------------------------ //

GGASelectorResultBuilder::GGASelectorResultBuilder()
    : num_evaluations_(0)
    , winners_()
    , instancesPerformance_()
{ }

GGASelectorResultBuilder::~GGASelectorResultBuilder()
{ }

GGASelectorResultBuilder& GGASelectorResultBuilder::addAll(const GGASelectorResult& result)
{
    num_evaluations_ += result.getNumEvaluations();
    winners_.insert(winners_.end(), result.getWinners().begin(), result.getWinners().end());
    
    const GGASelectorResult::InstancePerformancesMap& performanceMap = result.instancesPerformance_;
    GGASelectorResult::InstancePerformancesMap::const_iterator it, end = performanceMap.end();
    for (it = performanceMap.begin(); it != end; ++it)
        addInstancePerformance(it->first, it->second);
    return *this;
}

GGASelectorResultBuilder& GGASelectorResultBuilder::addWinner(const GGAGenome& winner)
{
    winners_.push_back(winner);
    return *this;
}

GGASelectorResultBuilder& GGASelectorResultBuilder::addWinners(const GGAGenomeVector& winners)
{
    winners_.insert(winners_.end(), winners.begin(), winners.end());
    return *this;
}

GGASelectorResultBuilder& GGASelectorResultBuilder::addInstancePerformance(
        const GGAInstance& instance, double performance)
{
    if (instancesPerformance_.count(instance) == 0) {
        instancesPerformance_[instance] = GGASelectorResult::InstancePerformancesMap::mapped_type();
    }
    
    instancesPerformance_[instance].push_back(performance);
            
    return *this;
}

GGASelectorResultBuilder& GGASelectorResultBuilder::addInstancePerformance(
        const GGAInstance& instance, const std::vector<double>& performance)
{
    if (instancesPerformance_.count(instance) == 0) {
        instancesPerformance_[instance] = GGASelectorResult::InstancePerformancesMap::mapped_type();
    }
    
    GGASelectorResult::InstancePerformancesMap::mapped_type& perf = instancesPerformance_[instance];
    perf.insert(perf.end(), performance.begin(), performance.end());
            
    return *this;
}

GGASelectorResultBuilder& GGASelectorResultBuilder::increaseNumEvaluations(int num_evaluations)
{
    num_evaluations_ += num_evaluations;
    return *this;
}

GGASelectorResult GGASelectorResultBuilder::build() const
{
    return GGASelectorResult(num_evaluations_, winners_, instancesPerformance_);
}

void GGASelectorResultBuilder::clear()
{
    num_evaluations_ = 0;
    winners_.clear();
    instancesPerformance_.clear();
}
