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

#ifndef _GGA_TOURNAMENT_HPP_
#define _GGA_TOURNAMENT_HPP_

#include <ctime>
#include <boost/scoped_ptr.hpp>

#include "GGAGenome.hpp"
#include "GGAInstances.hpp"
#include "GGALearningStrategy.hpp"
#include "GGAPopulation.hpp"
#include "GGASelector.hpp"


// TODO Add option to stop after no improvement in x generations (to be set in options)
class GGATournament
{
public:
    GGATournament(const GGAInstances& instances, GGASelector& selector);
    virtual ~GGATournament();
    
    void populate();
    void run();    

    int getCurrentGeneration() const;

    const GGAPopulation& getPopulation() const;
    GGAPopulation& getPopulation();

    const GGAInstances& getInstances() const;

private:
    // intentionally unimplemented
    GGATournament();
    GGATournament(const GGATournament&);
    GGATournament& operator=(const GGATournament&);

    void initTrajectoryFile() const;
    bool shouldStop() const;

    void nextGeneration();
    const GGAGenomeVector performSelection(const GGAInstanceVector& instances); 
    GGAGenomeVector performCrossover(const GGAGenomeVector& winners) const;
    const GGAGenomeVector& performMutation(GGAGenomeVector& children) const;

    GGAGenomeVector crossoverHelper(const GGAGenomeVector& winners) const;
    int checkNumOld() const;
    
    void updateInstancesInfo(const GGAInstanceVector& instances, const GGASelectorResult& result);

    // attributes
    GGAInstances instances_;
    GGAPopulation pop_;
    GGAGenome mostFit_;
    GGASelector& selector_;
    boost::scoped_ptr<GGALearningStrategy> learningStrategy_;   
    
    GGAInstanceInfoMap instancesInfo_;
    double instancesInfoAlpha_;   // TODO Make an option
    
    int gen_;
    int evalCount_;    
    int firstGenMaxInstances_; 

    std::vector<double> lastBest_;
    unsigned int numLastBest_;    // TODO Make an option
    double improvementThreshold_; // TODO Make an option
};


//==============================================================================
// GGATournament public in-line methods
   
inline int GGATournament::getCurrentGeneration() const { return gen_; }

inline const GGAPopulation& GGATournament::getPopulation() const { return pop_; }
inline GGAPopulation& GGATournament::getPopulation()             { return pop_; }

inline const GGAInstances& GGATournament::getInstances() const   { return instances_; }

#endif  // _GGA_TOURNAMENT_HPP_
