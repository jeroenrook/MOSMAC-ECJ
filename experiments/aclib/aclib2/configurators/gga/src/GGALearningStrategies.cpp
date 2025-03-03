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


#include <cmath>
#include <boost/math/special_functions/round.hpp>

#include "ggatypedefs.hpp"
#include "GGAExceptions.hpp"
#include "GGALearningStrategies.hpp"
#include "GGAOptions.hpp"
#include "OutputLog.hpp"

//
// === GGALearningStrategyLinear ===
//

/**
 *
 */
GGALearningStrategyLinear::GGALearningStrategyLinear(
                                            const GGAInstances& instances) 
    : GGALearningStrategy(instances)
{ }

/**
 *
 */
GGALearningStrategyLinear::~GGALearningStrategyLinear() {
}

/**
 *
 */
GGAInstanceVector GGALearningStrategyLinear::instances(int generation) const
{
    const GGAOptions& opts = GGAOptions::instance();
    int maxGen = opts.num_generations;
    int delay = opts.learning_strategy_delay;
    int gif = opts.gen_inst_finish;
    int instStart = opts.inst_start;
    int instFinish = opts.inst_finish;

    if(gif > -1)
        maxGen = gif;
    maxGen -= delay;
    
    int numInsts = instStart;
    if(generation > delay && generation < maxGen)
        numInsts = instStart + ((instFinish - instStart) *
                    ((generation - delay) / (double)maxGen));

    else if(generation >= maxGen)
        numInsts = instFinish;

    LOG_VERY_VERBOSE("[GGALearningStrategyLinear] Using " << numInsts 
            << " instances.");
    
    numInsts = std::max(1, numInsts); // Don't let this be 0
    return selectRandomInstances(numInsts);
}

//
// === GGALearningStrategyStep ===
//

/**
 *
 */
GGALearningStrategyStep::GGALearningStrategyStep(
                                        const GGAInstances& instances) 
    : GGALearningStrategy(instances)
{ }

/**
 *
 */
GGALearningStrategyStep::~GGALearningStrategyStep()
{ }

/**
 *
 */
GGAInstanceVector GGALearningStrategyStep::instances(int generation) const
{
    const GGAOptions& opts = GGAOptions::instance();
    int maxGen = opts.num_generations;
    int delay = opts.learning_strategy_delay;
    int gif = opts.gen_inst_finish;
    int instStart = opts.inst_start;
    int instFinish = opts.inst_finish;
    int stepGens = opts.learning_strategy_step;

    if(gif > -1)
        maxGen = gif;
    maxGen -= delay;
    
    int numInsts = instStart;
    if(generation > delay && generation < maxGen) {
        int useGen = int(generation / stepGens) * stepGens;

        numInsts = instStart + ((instFinish - instStart) * 
                    ((useGen - delay) / (double)maxGen));
    } else if(generation >= maxGen) {
        numInsts = instFinish;
    }

    LOG_VERY_VERBOSE("[GGALearningStrategyStep] Using " << numInsts 
            << " instances.");
    
    numInsts = std::max(1, numInsts); // Don't let this be 0
    return selectRandomInstances(numInsts);

}

//
// === GGALearningStrategyParabola ===
//

/**
 *
 */
GGALearningStrategyParabola::GGALearningStrategyParabola(
                                            const GGAInstances& instances) 
    : GGALearningStrategy(instances) 
{
    const GGAOptions& opts = GGAOptions::instance();
    int maxGen = opts.num_generations;
    int delay = opts.learning_strategy_delay;
    int gif = opts.gen_inst_finish;
    int instStart = opts.inst_start;
    int instFinish = opts.inst_finish;

    if(gif > -1)
        maxGen = gif;
    maxGen -= delay;
    
    m_a = (instFinish - instStart) / float(maxGen * maxGen);
}

/**
 *
 */
GGALearningStrategyParabola::~GGALearningStrategyParabola()
{ }

/**
 *
 */
GGAInstanceVector GGALearningStrategyParabola::instances(int generation) const
{
    const GGAOptions& opts = GGAOptions::instance();
    int maxGen = opts.num_generations;
    int delay = opts.learning_strategy_delay;
    int gif = opts.gen_inst_finish;
    int instStart = opts.inst_start;
    int instFinish = opts.inst_finish;
    
    if(gif > -1)
        maxGen = gif;
    maxGen -= delay;
    
    int numInsts = instStart;
    if(generation > delay && generation < maxGen)
        numInsts = m_a * (generation * generation) + instStart;
    else if(generation >= maxGen)
        numInsts = instFinish;
    
    LOG_VERY_VERBOSE("[GGALearningStrategyParabola] Using " << numInsts 
            << " instances.");
    
    numInsts = std::max(1, numInsts); // Don't let this be 0
    return selectRandomInstances(numInsts);

}

//
// === GGALearningStrategyExp ===
//

/**
 *
 */
GGALearningStrategyExp::GGALearningStrategyExp(const GGAInstances& instances) 
    : GGALearningStrategy(instances)
{
    const GGAOptions& opts = GGAOptions::instance();
    int maxGen = opts.num_generations;
    int delay = opts.learning_strategy_delay;
    int gif = opts.gen_inst_finish;
    int instStart = opts.inst_start;
    int instFinish = opts.inst_finish;

    if(gif > -1)
        maxGen = gif;
    maxGen -= delay;
        
    m_a = log(instFinish - instStart) / float(maxGen);
}

/**
 *
 */
GGALearningStrategyExp::~GGALearningStrategyExp()
{ }

/**
 *
 */
GGAInstanceVector GGALearningStrategyExp::instances(int generation) const
{
    const GGAOptions& opts = GGAOptions::instance();
    int maxGen = opts.num_generations;
    int delay = opts.learning_strategy_delay;
    int gif = opts.gen_inst_finish;
    int instStart = opts.inst_start;
    int instFinish = opts.inst_finish;

    if(gif > -1)
        maxGen = gif;
    maxGen -= delay;
    
    int numInsts = instStart;
    if(generation > delay && generation < maxGen)
        numInsts = pow(M_E, m_a * generation) + instStart;
    else if(generation >= maxGen)
        numInsts = instFinish;

    LOG_VERY_VERBOSE("[GGALearningStrategyExp] Using " << numInsts 
            << " instances.");
    
    numInsts = std::max(1, numInsts); // Don't let this be 0
    return selectRandomInstances(numInsts);
}

///
/// GGALearningStrategyClusterUniformSelection
///

/**
 *
 */
GGALearningStrategyRndCluster::GGALearningStrategyRndCluster(
        const GGAInstances& instances) 
    : GGALearningStrategy(instances)
{
    if (!m_instances.hasClusters()) {
        throw GGAException(
                "[GGALearningStrategyRndCluster] No clusters defined");
    }
}

/**
 *
 */
GGALearningStrategyRndCluster::~GGALearningStrategyRndCluster()
{ }

/**
 *
 */
GGAInstanceVector GGALearningStrategyRndCluster::instances(int generation) const 
{
    const GGAOptions& opts = GGAOptions::instance();
    int maxGen = opts.num_generations;
    int instStart = opts.inst_start;
    int instFinish = opts.inst_finish;
    int delay = opts.learning_strategy_delay;
    int gif = opts.gen_inst_finish;
    
    //throw GGAException(
    //        "[GGALearningStrategyRndCluster::instances] Not implemented");
    // Linear strategy to know how much instances in this generation should be
    // selected
    if(gif > -1)
        maxGen = gif;
    maxGen -= delay;
    
    unsigned lineal_num_insts = instStart;
    if(generation > delay && generation < maxGen)
        lineal_num_insts = instStart + ((instFinish - instStart) *
                    ((generation - delay) / (double)maxGen));
    else if(generation >= maxGen)
        lineal_num_insts = instFinish;

    LOG_VERY_VERBOSE("[GGALearningStrategyRndCluster] Lineal strategy should"
            << " use " << lineal_num_insts << " instances.");
    //==========================================================================
    
    // Casts operations to double
    double total_num_insts = m_instances.getNumberOfInstances();    
    GGAInstanceVector retInst;            

    for (unsigned i = 0; i < m_instances.getNumberOfClusters(); ++i) {
        const std::vector<unsigned>& cluster = m_instances.getCluster(i);
        double cluster_ratio = cluster.size() / total_num_insts;

        LOG_VERY_VERBOSE("[GGALearningStrategyRndCluster] Cluster '" << i 
                << "' ratio is " << (cluster_ratio * 100) << "\%." )
        unsigned insts = boost::math::round(lineal_num_insts * cluster_ratio);
        unsigned num_insts = std::max(1U, insts);
        
        GGAInstanceVector selected = selectRandomClusterInstances(i, num_insts);
        retInst.insert(retInst.end(), selected.begin(), selected.end());
    }
    
    LOG_VERY_VERBOSE("[GGALearningStrategyRndCluster] Using " << retInst.size() 
            << " instances.");
    
    return retInst;
}
