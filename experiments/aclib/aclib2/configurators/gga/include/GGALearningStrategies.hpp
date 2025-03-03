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

#ifndef _GGA_LEARNING_STRATEGIES_HPP_
#define _GGA_LEARNING_STRATEGIES_HPP_

#include "GGALearningStrategy.hpp"


/** === GGALearningStrategyLinear === **/

class GGALearningStrategyLinear : public GGALearningStrategy 
{
public:
    GGALearningStrategyLinear(const GGAInstances& instances);
    virtual ~GGALearningStrategyLinear();
    virtual GGAInstanceVector instances(int generation) const;
};


/** === GGALearningStrategyStep === **/

class GGALearningStrategyStep : public GGALearningStrategy 
{
public:
    GGALearningStrategyStep(const GGAInstances& instances);
    virtual ~GGALearningStrategyStep();
    virtual GGAInstanceVector instances(int generation) const;
};


/** === GGALearningStrategyParabola === **/

class GGALearningStrategyParabola : public GGALearningStrategy 
{
public:
    GGALearningStrategyParabola(const GGAInstances& instances);
    virtual ~GGALearningStrategyParabola();
    virtual GGAInstanceVector instances(int generation) const;

private:
    double m_a;
};


/** === GGALearningStrategyExp === **/

class GGALearningStrategyExp : public GGALearningStrategy 
{
public:
    GGALearningStrategyExp(const GGAInstances& instances);
    virtual ~GGALearningStrategyExp();
    virtual GGAInstanceVector instances(int generation) const;

private:
    double m_a;
};

/** === GGALearningStrategyClusterUniformSelection === **/

class GGALearningStrategyRndCluster : public GGALearningStrategy
{
public:
    GGALearningStrategyRndCluster(const GGAInstances& instances);
    virtual ~GGALearningStrategyRndCluster();
    virtual GGAInstanceVector instances(int generation) const;
    
private:
    
};

#endif
