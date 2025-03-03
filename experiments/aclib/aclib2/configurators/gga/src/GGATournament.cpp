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
#include <fstream>
 
#include "GGAExceptions.hpp"
#include "GGAGenome.hpp"
#include "GGAOptions.hpp"
#include "GGAParameterTree.hpp"
#include "GGAPopulation.hpp"
#include "GGARandEngine.hpp"
#include "GGASystem.hpp"
#include "GGATournament.hpp"
#include "GGAUtil.hpp"
#include "OutputLog.hpp"


/**
 * @todo initialize population in GGATournament using information from
 * GGAOptions and GGAParameterTree.
 */
GGATournament::GGATournament(const GGAInstances& instances, GGASelector& selector) 
    : instances_(instances)
    , pop_()
    , mostFit_()
    , selector_(selector)
    , learningStrategy_(createLearningStrategy(instances_))

    , instancesInfo_()
    , instancesInfoAlpha_(0.8)

    , gen_(1)
    , evalCount_(0)
    , firstGenMaxInstances_(0)
 
    , lastBest_()
    , numLastBest_(4)
    , improvementThreshold_(0.1)
{ }

GGATournament::~GGATournament() 
{ }


void GGATournament::populate()
{
    const GGAOptions& opts = GGAOptions::instance();
    const GGAParameterTree& ptree = GGAParameterTree::instance();

    const std::vector<GenomeMap>& sGenomes = ptree.seededGenomes();
    std::vector<GenomeMap>::const_iterator itr;
    for(itr = sGenomes.begin(); itr != sGenomes.end(); ++itr) {
        GGAGenome seedGenome(ptree, *itr);
        pop_.addGenome(seedGenome);
    }
        
    // Initialize population
    int popSize = opts.pop_size;
    popSize -= opts.seeded_genomes;
    pop_.addRandomGenomes(ptree, popSize);
}


void GGATournament::run()
{
    const GGAOptions& opts = GGAOptions::instance();
    firstGenMaxInstances_ = 0;
 
    // Trajectory file first line
    initTrajectoryFile();
     
    while(!shouldStop()) {
        nextGeneration();
        lastBest_.push_back(mostFit_.objValue());
        if(lastBest_.size() > numLastBest_) {
            lastBest_.erase(lastBest_.begin());
        }
    }

    LOG("Final most fit Obj: " << mostFit_.objValue());
    LOG("Final most fit parameters: " << mostFit_.toString());

    StringVector extra;
    extra.push_back("SATISFIABLE");
    GGAInstance inst(123456789, opts.target_algo_cpu_limit, "instance_here",
                     extra);

    LOG("Final most fit command: " << makeCommand(GGAParameterTree::instance(),
                                                  mostFit_.genome(), inst));
    
    /*LOG_DEBUG("---- Instances statistics ----")
    for (size_t i = 0; i < m_instances.getNumberOfInstances(); ++i) {
        const GGAInstance& inst = m_instances.getInstance(i);
        LOG_DEBUG("Instance: " << inst.instance());
        if (m_instances_statistics.has(inst)) {
            GGAEvaluationStatistics& stats = m_instances_statistics.get(inst);
            LOG_DEBUG("    Avg. Performance: " << stats.getAvgPerformance());
            LOG_DEBUG("    # Evaluations: " << stats.getNumberOfEvaluations());
        } else {
            LOG_DEBUG("    N/D");
        }
    }*/
}

//==============================================================================
// GGATournament private methods

void GGATournament::initTrajectoryFile() const
{
    const GGAOptions& opts = GGAOptions::instance();
    
    std::ofstream fp(opts.traj_file.c_str());
    if(fp.is_open()) {
        fp << "run-group-name-ignored 0" << std::endl;
        fp.close();
    }
}


bool GGATournament::shouldStop() const
{
    const GGAOptions& opts = GGAOptions::instance();

    bool ret = gen_ > opts.num_generations;
    if(!ret) {
        // If we are beyond the GEN_INST_FINISH threshold, check for improvement
        if(firstGenMaxInstances_ > 0 && gen_ > (int)(firstGenMaxInstances_ + numLastBest_) ) {
            double avgImprovement = 0.0;
            for(unsigned int i = firstGenMaxInstances_ + 1; i < lastBest_.size(); ++i) {
                avgImprovement += lastBest_[i - 1] - lastBest_[i];
            }
            avgImprovement /= (lastBest_.size() - opts.gen_inst_finish);
            ret = avgImprovement < improvementThreshold_;
            if(ret) LOG("Stopping because: below improvement threshold");
        }
    }
    bool timeExceeded = (userTime() >= opts.tuner_cutoff || 
                        wallClockTime() >= opts.tuner_wall_cutoff);
    if(timeExceeded) LOG("Stopping because: CPU time exceeded. [current: " << 
                         userTime() << "; maximum: " << opts.tuner_cutoff);

    bool evalsExceeded = evalCount_ >= opts.max_evals;
    if(evalsExceeded) LOG("Stopping because: maximum evaluations exceeded."
                          " evals: " << evalCount_ << "; maximum: " 
                          << opts.max_evals);
    
    return ret || timeExceeded || evalsExceeded;
}


void GGATournament::nextGeneration() {
    LOG("Start generation " << gen_);
    LOG("Generation " << gen_ << " population: " << pop_.toString());

    // Get instances from the learning strategy
    GGAInstanceVector instances = learningStrategy_->instances(gen_);
    if (instances.size() == instances_.getNumberOfInstances() && firstGenMaxInstances_ <= 0) {
        firstGenMaxInstances_ = gen_;
    }

    GGAInstanceVector::iterator it;
    for (it = instances.begin(); it != instances.end(); ++it)
        LOG("nextGeneration(): 1 | " << it->getSeed()  << " | " << it->getInstance());

    const GGAGenomeVector winners = performSelection(instances);
    
    GGAGenomeVector children = performCrossover(winners);
    
    performMutation(children);
    
    pop_.addGenomes(children);
    
    pop_.agePopulation();

    LOG("End generation " << gen_);
    ++gen_;
}


/**
 * Returns a vector of winners of a series of tournaments on a subset
 * of instances in instances
 */
const GGAGenomeVector GGATournament::performSelection(const GGAInstanceVector& instances) 
{
    GGAGenomeVector& compPop = pop_.population(GGAGenome::COMPETITIVE);

    resetHighlander(compPop);
    resetObjectiveValue(compPop);

    GGASelectorResult result = selector_.select(compPop, instances);
    evalCount_ += result.getNumEvaluations();
    GGAGenomeVector winners = result.getWinners();

    // Sort winners. winners[0] is the one that generates the most fit command.
    std::sort(winners.begin(), winners.end(), GGAGenome::compareObjValueAscendingOrder);
    
    GGAGenome& mf = winners[0];
    mf.bestInPopulation(true);
    mostFit_ = mf;

    LOG("Generation " << gen_ << " most fit Obj: " << mf.objValue());
    LOG("Generation " << gen_ << " most fit parameters: " << mf.toString());

    const GGAOptions& opts = GGAOptions::instance();
    StringVector extra;
    extra.push_back("SATISFIABLE");
    std::string command = makeCommand(GGAParameterTree::instance(), mf.genome(),
        GGAInstance(123456789, opts.target_algo_cpu_limit,
            "instance_here", extra));

    LOG("Generation " << gen_ << " most fit cmd: " << command);
    outputTrajectory(GGAParameterTree::instance(), mf.genome(), 
                     userTime(), wallClockTime());
    LOG("Generation " << gen_ << " number of evaluations so far: " 
        << evalCount_);
    

    double avg = 0.0;
    unsigned solved = 0;
    GGAInstanceVector::const_iterator itr, itrend = instances.end();    
    for(itr = instances.begin(); itr != itrend; ++itr) {
        double perf = winners[0].performance(*itr);
        if (perf < opts.target_algo_cpu_limit) { solved += 1; }

        LOG("(Winner Performance) " << itr->getInstance() << ": " << perf);
        avg += perf;
    }

    avg /= instances.size();
    LOG("(Winner Average Performance): " << avg);
    LOG("(Winner Solved instances): " << solved << " out of " << instances.size());
    LOG("TRAJECTORY | " << userTime() << " , " << avg << " , -1 , " 
        << instances.size() << " , " << evalCount_ << " | params: " 
        << command);

    // Update population values using select return
    LOG("Updating winners in population ...");
    pop_.updatePopulation(winners);
    
    return winners;
}


/**
 * Returns a vector of children created from the tournament winners
 * and the non-competitive portion of the population
 */
GGAGenomeVector GGATournament::performCrossover(
    const GGAGenomeVector& winners) const
{
    const GGAOptions& opts = GGAOptions::instance();
    int desiredPopSize = opts.pop_size;

    int maxPopSize = int(desiredPopSize * 1.25);
    int minPopSize = int(desiredPopSize * 0.75);
    int numOld = checkNumOld();
    int curPopSize = pop_.populationSize();

    GGAGenomeVector children = crossoverHelper(winners);
    int newSize = curPopSize + children.size() - numOld;

    while(newSize < minPopSize) { // Population too small, keep breeding        
        GGAGenomeVector moreChildren = crossoverHelper(winners);

        if(moreChildren.size() == 0)
            throw GGAPopulationException("Population has become too small to"
                                      " continue.");

        children.insert(children.end(), moreChildren.begin(), 
                        moreChildren.end());
        newSize = curPopSize + children.size() - numOld;
    }
    
    // If pop is too big, randomly kill children
    while(newSize > maxPopSize) {
        int randKill = GGARandEngine::randInt(0, children.size() - 1);
        children.erase(children.begin() + randKill);     

        newSize -= 1;
    }

    return children;
}
 

const GGAGenomeVector& GGATournament::performMutation(
    GGAGenomeVector& children) const
{
    GGAGenomeVector::iterator itr;
    for(itr = children.begin(); itr != children.end(); ++itr)
        itr->mutate(GGAParameterTree::instance());

    return children;
}


GGAGenomeVector GGATournament::crossoverHelper(
    const GGAGenomeVector& winners) const
{
    const GGAOptions& opts = GGAOptions::instance();

    int maxAge = opts.genome_age;
    double mateAmt = 2.0 / maxAge;

    GGAGenomeVector children;    
    const GGAGenomeVector& nPop = pop_.population(GGAGenome::NONCOMPETITIVE);

    GGAGenomeVector::const_iterator itr;
    for(itr = nPop.begin(); itr != nPop.end(); ++itr) {
        // Only mate mateAmt of the population to keep the size stable
        const GGAGenome& curN = *itr;
        if(GGARandEngine::randDouble(0.0, 1.0) <= mateAmt) {
            int randWinner = GGARandEngine::randInt(0, winners.size() - 1);
            //LOG_ERROR(curN << " <--> " << winners[randWinner] << ": " << curN->genome().size() << " :: " << winners[randWinner]->genome().size());
            GGAGenome child = crossover(GGAParameterTree::instance(), curN,
                                        winners[randWinner]);
            children.push_back(child);
        }
    }
     
    return children;
}


/**
 * Checks how many members of the population will be killed
 */
int GGATournament::checkNumOld() const 
{
    const GGAOptions& opts = GGAOptions::instance();
    int maxAge = opts.genome_age;
    int ret = 0;

    const GGAGenomeVector& npop = pop_.population(GGAGenome::NONCOMPETITIVE);
    const GGAGenomeVector& cpop = pop_.population(GGAGenome::COMPETITIVE);

    GGAGenomeVector::const_iterator it;
    for(it = npop.begin(); it != npop.end(); ++it)
        if (it->age() >= maxAge && !it->bestInPopulation())
            ret += 1;

    for (it = cpop.begin(); it != cpop.end(); ++it)
        if (it->age() >= maxAge && !it->bestInPopulation())
            ret += 1;
    
    return ret;
}


void GGATournament::updateInstancesInfo(const GGAInstanceVector& instances, 
                                        const GGASelectorResult& result)
{
    GGAInstanceVector::const_iterator itinst, endinst = instances.end();
    for (itinst = instances.begin(); itinst != endinst; ++itinst) {
        if (result.hasInstancePerformance(*itinst)) {
            const std::vector<double>& performance = result.getInstancePerformance(*itinst);
            
            std::vector<double>::const_iterator it, end = performance.end();
            for (it = performance.begin(); it != end; ++it) {
                if (instancesInfo_.count(*itinst) == 0) {
                    GGAInstanceInfo iinfo;
                    iinfo.setEstimatedExecutionTime(*it);
                    instancesInfo_[*itinst] = iinfo;                    
                } else {                
                    GGAInstanceInfo& iinfo = instancesInfo_[*itinst];
                    iinfo.setEstimatedExecutionTime(
                            iinfo.getEstimatedExecutionTime() * instancesInfoAlpha_ 
                            + (*it) * (1 - instancesInfoAlpha_)
                        );
                }
            }
        }
    }
}
