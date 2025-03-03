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


#ifndef _GGA_LEARNING_STRATEGY_HPP_
#define _GGA_LEARNING_STRATEGY_HPP_


#include <vector>
#include "GGAInstances.hpp"


class GGALearningStrategy 
{
public:
    
    // construct/copy/destruct
    GGALearningStrategy(const GGAInstances& instances);
    GGALearningStrategy(const GGALearningStrategy& other);
    virtual ~GGALearningStrategy();
    
    //
    virtual GGAInstanceVector instances(int generation) const;
    virtual GGAInstanceVector selectRandomInstances(size_t n) const;
    GGAInstanceVector selectRandomClusterInstances(
            unsigned index, unsigned n) const;

protected:
    GGAInstances m_instances;
};


//==============================================================================
// GGALearningStrategy public inline methods

/**
 * Default strategy is to just return all instances. Don't use this except for
 * testing.
 */
inline std::vector<GGAInstance> GGALearningStrategy::instances(
        int generation) const
{ return m_instances.getAllInstances(); }


#endif // _GGA_LEARNING_STRATEGY_HPP_
