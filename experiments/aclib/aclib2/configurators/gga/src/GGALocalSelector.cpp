//
// This file is part of DGGA.
// 
// The MIT License (MIT)
// 
// Copyright (c) 2015 Josep Pon Farreny
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


#include <cassert>
#include <csignal>
#include <cstring>
#include <cerrno>

#include <algorithm>
#include <fstream>
#include <limits>
#include <sstream>

#include <sys/resource.h>
#include <sys/wait.h>

#include "ggatypedefs.hpp"
#include "GGAExceptions.hpp"
#include "GGAGenome.hpp"
#include "GGALocalSelector.hpp"
#include "GGAOptions.hpp"
#include "GGAParameterTree.hpp"
#include "GGARandEngine.hpp"
#include "GGASystem.hpp"
#include "GGAUtil.hpp"
#include "OutputLog.hpp"


//==============================================================================
// Local functions

/**
 *
 */
static void doWarn(const RunnerVector& runners) 
{
    const GGAOptions& opts = GGAOptions::instance();

    if (opts.send_sigusr1) {
        LOG_VERY_VERBOSE("Sending SIGUSR1 to child processes (Warning)");
        RunnerVector::const_iterator itr;
        for(itr = runners.begin(); itr != runners.end(); ++itr) {
            pid_t pc = (*itr)->childPid();
            if(pc != 0)
                ::kill(pc, SIGUSR1);
        }
    }
}

static void doTerm(const RunnerVector& runners) 
{
    LOG_VERY_VERBOSE("Sending SIGTERM to child processes (Termination)");
    RunnerVector::const_iterator itr;
    for(itr = runners.begin(); itr != runners.end(); ++itr) {
        pid_t pc = (*itr)->childPid();
        if(pc != 0)
            ::kill(pc, SIGTERM);
    }
}

static void dokill(const RunnerVector& runners)
{
    LOG_VERY_VERBOSE("Sending SIGKILL to child processes (Some processes"
                     " hanging after SIGTERM)");
    RunnerVector::const_iterator itr;
    for(itr = runners.begin(); itr != runners.end(); ++itr) {
        pid_t pc = (*itr)->childPid();
        if(pc != 0)
            ::kill(pc, SIGKILL);
    }
}


/*
 * Returns whether or not it is safe to continue (all child processes
 * accepted SIGTERM)
 */
static bool waitAfterTerm()
{
    int res = waitpid(0, NULL, WNOHANG);
    int count = 0;

    while(res == 0 && count < 500) {
        sleepFor(0, 1000000); // 0.001 seconds
        res = waitpid(0, NULL, WNOHANG);
        ++count;
    }

    return res == -1;
}


//==============================================================================
// GGALocalSelector public methods

/**
 *
 */
GGALocalSelector::GGALocalSelector()
    : m_runners_mutex()
    , m_runners()
    , m_runners_ctrl_semaphore(0)

    , m_stop_mutex()
    , m_stop_requested(false)

    , m_num_threads(GGAOptions::instance().num_threads)
    , m_runtime_tuning(GGAOptions::instance().runtime_tuning)
    , m_cpu_limit(GGAOptions::instance().target_algo_cpu_limit)
    , m_penalty(GGAOptions::instance().penalty_mult)
    , m_pct_winners(GGAOptions::instance().pct_winners)
    , m_propagate_timeout(GGAOptions::instance().propagate_timeout)
{
    /*if (std::signal(SIGCHLD, handleZombies) == SIG_ERR)
        throw std::runtime_error(
            std::string("Error installing SIGCHLD: ") += std::strerror(errno));
    */
}


/**
 *
 */
GGALocalSelector::~GGALocalSelector()
{
    /*if (std::signal(SIGCHLD, SIG_DFL) == SIG_ERR)
        throw std::runtime_error(
            std::string("Error reseting SIGCHLD: ") += std::strerror(errno));*/
}


/**
 *
 */
void GGALocalSelector::forceStop()
{
    {
        boost::unique_lock<boost::mutex> lock(m_stop_mutex);
        m_stop_requested = true;
        if (!m_runners_ctrl_semaphore.try_wait()) // release semaphore lock
            m_runners_ctrl_semaphore.post();
    }
}


/**
 *
 */
void GGALocalSelector::resetTimeout(double new_timeout)
{
    boost::unique_lock<boost::mutex> lock(m_runners_mutex);
    for (unsigned i = 0; i < m_runners.size(); ++i) {
        m_runners[i]->resetTimeout(new_timeout);
    }
}


/**
 * Takes in a vector of participants and returns a subset of winners.
 */
GGASelectorResult GGALocalSelector::select(const GGAGenomeVector& participants, 
                                           const GGAInstanceVector& instances,
                                           double start_timeout)
{   
    GGASelectorResultBuilder resultBuilder;
    double timeout = start_timeout;

    // TODO Sort instances in order of descending timeout!!
    UIntVector tournySizes = balanceTournaments(participants.size(),
                                                m_num_threads);
    LOG_VERBOSE("Tournament sizes for this generation: [" 
                << OutputLog::uintVectorToString(tournySizes) << "]");
    
    // Run each tournament of this generation
    unsigned numUsed = 0, tournyNum = 1;
    UIntVector::const_iterator itr;

    for(itr = tournySizes.begin(); itr != tournySizes.end(); ++itr) {
        LOG_VERY_VERBOSE("Beginning tournament " << tournyNum << " of " 
                         << tournySizes.size());
        
        // Propagate previous timeout?
        if (!m_propagate_timeout)
            timeout = start_timeout;

        // Fill with references to participants
        GGAGenomeVector tournyParticipants;
        tournyParticipants.assign(participants.begin() + numUsed,
                                  participants.begin() + numUsed + *itr);
                
        // Does not pick the same genome twice (DGGA is happy with that :D)
        initializeTournament(tournyParticipants, instances, timeout);
        assert(tournyParticipants.size() == m_runners.size());

        // Begin tournament execution
        assert(!m_runners_ctrl_semaphore.try_wait());
        for(size_t i = 0; i < m_runners.size(); ++i)
            m_runners[i]->run(m_runners_ctrl_semaphore);
        
        // determine the number of winners to wait for
        int numWinners = int(m_pct_winners * tournyParticipants.size());
        if(numWinners <= 0)
            numWinners = 1;

        LOG_VERBOSE("Waiting for " << numWinners << " competitor" 
                    << ((numWinners > 1) ? "s" : "") << " to finish.");

        // Wait until all the participants have finished
        size_t nfinished = 0;
        double local_best_time = std::numeric_limits<double>::max();
        std::vector<bool> observed(m_runners.size(), false);

        stopPoint();
        while (nfinished < m_runners.size()) {
            m_runners_ctrl_semaphore.wait();
            stopPoint();

            for (size_t i = 0; i < m_runners.size(); ++i) {
                if (m_runners[i]->hasFinished() && !observed[i]) {
                    LOG("Registering runner " << m_runners[i] 
                        << " as finished");

                    nfinished += 1;
                    observed[i] = true;

                    local_best_time = std::min(local_best_time,
                                               m_runners[i]->objValue());
                    if (m_propagate_timeout)
                        timeout = std::min(timeout, local_best_time);
                    else
                        timeout = local_best_time;

                    for (size_t j = 0; j < m_runners.size(); ++j)
                        m_runners[j]->resetTimeout(timeout);

                    break;
                }
            }
        }

        // Register participants results
        registerInstancesResults(m_runners, instances, tournyParticipants, resultBuilder);
        
        // Sort & select winners
        std::sort(tournyParticipants.begin(), tournyParticipants.end(),
                  GGAGenome::compareObjValueAscendingOrder);
    
        GGAGenomeVector::const_iterator winners_beg = 
                                    tournyParticipants.begin();
        GGAGenomeVector::const_iterator winners_end = 
                                    tournyParticipants.begin() + numWinners;

        resultBuilder.addWinners(GGAGenomeVector(winners_beg, winners_end));
         
        GGAGenomeVector::const_iterator gitr;
        for(gitr = winners_beg; gitr != winners_end; ++gitr) {
            LOG("Winner of tournament " << tournyNum << "." << std::endl
                    << "\tObjective: " << gitr->objValue() << std::endl
                    << "\tDetails: " << gitr->toString());
        }
                
        numUsed += *itr;
        LOG_VERY_VERBOSE("Ending tournament " << tournyNum << " of " << tournySizes.size());
        tournyNum += 1;

        // Collect evaluations
        for (RunnerVector::size_type i = 0; i < m_runners.size(); ++i)
            resultBuilder.increaseNumEvaluations(m_runners[i]->evals());
        LOG("Number of evaluations in this generation so far: " <<
            resultBuilder.getNumEvaluations() << std::endl); 

        killRunners();
        cleanUpRunners();
    }
    stopPoint();
    
    return resultBuilder.build();
}

//==============================================================================
// GGALocalSelector private methods

/**
 *
 */
void GGALocalSelector::initializeTournament(const GGAGenomeVector& participants, 
                                            const GGAInstanceVector& instances,
                                            double timeout)
{
    boost::unique_lock<boost::mutex> lock(m_runners_mutex);

    int id = 0;
    GGARunnerBuilder builder;
    GGAGenomeVector::const_iterator itr;
    for(itr = participants.begin(); itr != participants.end(); ++itr) {
        builder.setId(id).setRuntimeTuning(m_runtime_tuning).setPenaltyFactor(m_penalty)
                .setCpuLimit(m_cpu_limit).setInstances(&instances)
                .setParameterTree(&GGAParameterTree::instance())
                .setGenome(&(*itr));
        GGARunner* r = builder.build();
        r->resetTimeout(timeout);
        m_runners.push_back(r);
        ++id;
    }
}


/**
 *
 */
void GGALocalSelector::registerInstancesResults(const RunnerVector& runners,
                                                const GGAInstanceVector& instances,
                                                GGAGenomeVector& genomes,
                                                GGASelectorResultBuilder& resultBuilder)
{
    assert(genomes.size() == runners.size());

    for (size_t i = 0; i < genomes.size(); ++i) {
        GGAGenome& genome = genomes[i];
        const GGARunner* runner = runners[i];

        genome.objValue(runner->objValue());
        for (size_t j = 0; j < instances.size(); ++j) {
            if (runner->hasPerformance(instances[j])) {
                double inst_performance = runner->getPerformance(instances[j]);
                genome.setPerformance(instances[j], inst_performance);
                resultBuilder.addInstancePerformance(instances[j], inst_performance);
            }
        }
    }
}


/**
 *
 */
void GGALocalSelector::killRunners()
{
    // 0.5s of sleep between warn and term.
    doWarn(m_runners);
    sleepFor(0, 500000000);
    doTerm(m_runners);
    
    // Wait for processes to act on SIGTERM. 
    // If after 0.5 seconds all aren't gone, send a SIGKILL
    bool safe = waitAfterTerm();
    if(!safe)
        dokill(m_runners);
}


/**
 *
 */
void GGALocalSelector::cleanUpRunners()
{
    boost::unique_lock<boost::mutex> lock(m_runners_mutex);
    RunnerVector::iterator itr;
    for(itr = m_runners.begin(); itr != m_runners.end(); ++itr) {
        delete *itr;
    }
    m_runners.clear();
}


/**
 *
 */
void GGALocalSelector::stopPoint()
{
    boost::unique_lock<boost::mutex> lock(m_stop_mutex);
    if (m_stop_requested) {
        m_stop_requested = false;

        for (size_t i = 0; i < m_runners.size(); ++i)
            m_runners[i]->interrupt();
        
        killRunners();
        cleanUpRunners();

        throw GGAInterruptedException(
            "GGALocalSelector stop forced by another thread.");
    }
}
