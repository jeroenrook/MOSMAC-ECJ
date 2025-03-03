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


#include <algorithm>
#include "GGALearningStrategy.hpp"

//==============================================================================
// GGALearningStrategy public methods

/**
 * Initializes a learning strategy with some instances.
 */
GGALearningStrategy::GGALearningStrategy(const GGAInstances& instances)
    : m_instances(instances)
{ }


/**
 *
 */
GGALearningStrategy::GGALearningStrategy(const GGALearningStrategy& other)
    : m_instances(other.m_instances)
{ }


/**
 *
 */
GGALearningStrategy::~GGALearningStrategy()
{ }

/**
 * @brief Selects n instances randomly.
 */
GGAInstanceVector GGALearningStrategy::selectRandomInstances(size_t n) const
{
    GGAInstanceVector retInsts(m_instances.getAllInstances());
    std::random_shuffle(retInsts.begin(), retInsts.end());
    retInsts.resize(n > retInsts.size() ? retInsts.size() : n);
    
    return retInsts;
}

GGAInstanceVector GGALearningStrategy::selectRandomClusterInstances(
        unsigned index, unsigned n) const
{
    std::vector<unsigned> cluster = m_instances.getCluster(index);
    std::random_shuffle(cluster.begin(), cluster.end());
    cluster.resize(n > cluster.size() ? cluster.size() : n);

    GGAInstanceVector retInsts;   
    for (unsigned i = 0; i < cluster.size(); ++i)
        retInsts.push_back(m_instances.getInstance(cluster[i]));
    
    return retInsts;
}
