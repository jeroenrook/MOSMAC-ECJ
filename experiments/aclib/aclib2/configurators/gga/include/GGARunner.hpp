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


#ifndef _GGA_RUNNER_HPP_
#define _GGA_RUNNER_HPP_

#include <limits>
#include <map>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "ggatypedefs.hpp"
#include "GGACommand.hpp"
#include "GGAEvaluationStatistics.hpp"
#include "GGAGenome.hpp"
#include "GGAInstance.hpp"
#include "GGAUtil.hpp"


// ===== Forward Declarations =====
class GGARunnerBuilder;


// ===== GGARunner =====

/**
 * @brief Executes and stores the results of a genome configuration over a given set of instances.
 */
class GGARunner 
{
    friend class GGARunnerBuilder;
    
public:
    virtual ~GGARunner();

    void run(boost::interprocess::interprocess_semaphore& ctrlSem);
    
    void interrupt();        
    int evals();

    // secure access to shared memory data (upgradable mutex)   
    pid_t childPid() const;
    bool hasFinished() const;
    double objValue() const;
    double getTimeout() const;
    const double getPerformance(const GGAInstance&) const;
    bool hasPerformance(const GGAInstance&) const;

    void resetTimeout(double t);

private:
    GGARunner(int id, bool runtimeTuning, double penalty, unsigned cpuLimit,
              const GGAInstanceVector& instances, const GGAParameterTree& ptree,
              const GGAGenome& genome);
    
    GGARunner();                                 // Intentionally unimplemented
    GGARunner(const GGARunner&);                 // Intentionally unimplemented  
    GGARunner& operator=(const GGARunner& orig); // Intentionally unimplemented
    
    /** @brief Builds the commands to test the given genome performance for each instance. */
    void buildCommands(const GGAParameterTree& ptree, const GGAGenome& genome);
    
    // 
    void childPid(pid_t);
    void hasFinished(bool);
    void setObjectiveValue(double);
    void setPerformance(const GGAInstance&, double);

    void initializePerformanceData(size_t);

    void runThread(boost::interprocess::interprocess_semaphore& ctrlSem);
    void runCommand(const StringVector&, const size_t index);
    void runCommandParent(int[2], const int child_pid, const size_t index);
    void runCommandChild(int[2], const StringVector&);

    double computeRuntimePerformance(int exit_status, double user_time,
                                     const std::string& last_line);

    double computeOutputPerformance(int exit_status,
                                    const std::string& last_line);

    void setProcessResourceLimits();

    //
    std::vector<StringVector> m_commands;

    const int m_id; // unique id among runners currently running
    const bool m_runtimeTuning;
    const double m_penalty;
    const unsigned m_cpuLimit;
    GGAInstanceVector m_instances;
    
    boost::thread m_runThread;
    mutable boost::shared_mutex m_rwMutex; // Modifiable in constant methods

    int m_evals;
    int m_childPid;
    bool m_hasFinished;
    double m_objVal;
    double m_timeout;    
    GGAInstancePerformanceMap m_performance;
};

//==============================================================================
// Typedefs
typedef std::vector<GGARunner*> RunnerVector;

//==============================================================================
// GGARunner in-line methods

inline int GGARunner::evals()
{
    boost::shared_lock<boost::shared_mutex> rlock(m_rwMutex);
    return m_evals;
}

inline pid_t GGARunner::childPid() const 
{
    boost::shared_lock<boost::shared_mutex> rlock(m_rwMutex);
    return static_cast<pid_t>(m_childPid);
}

inline bool GGARunner::hasFinished() const
{
    boost::shared_lock<boost::shared_mutex> rlock(m_rwMutex);
    return m_hasFinished;
}

inline double GGARunner::objValue() const
{
    boost::shared_lock<boost::shared_mutex> rlock(m_rwMutex);
    return m_objVal;
}

inline double GGARunner::getTimeout() const 
{
    boost::shared_lock<boost::shared_mutex> rlock(m_rwMutex);
    return m_timeout; 
}

inline const double GGARunner::getPerformance(const GGAInstance& inst) const 
{
    boost::shared_lock<boost::shared_mutex> rlock(m_rwMutex);
    return mapAt(m_performance, inst);
}

inline bool GGARunner::hasPerformance(const GGAInstance& inst) const
{
    boost::shared_lock<boost::shared_mutex> rlock(m_rwMutex);
    return m_performance.count(inst) > 0;
}

inline void GGARunner::resetTimeout(double t)
{
    boost::unique_lock<boost::shared_mutex> wlock(m_rwMutex);
    m_timeout = t;
}

//==============================================================================
// GGARunner private in-line methods

inline void GGARunner::childPid(pid_t p)
{
    boost::unique_lock<boost::shared_mutex> wlock(m_rwMutex);
    m_childPid = p;
}

inline void GGARunner::hasFinished(bool has_finished)
{
    boost::unique_lock<boost::shared_mutex> wlock(m_rwMutex);
    m_hasFinished = has_finished;
}

inline void GGARunner::setObjectiveValue(double val)
{
    boost::unique_lock<boost::shared_mutex> wlock(m_rwMutex);
    m_objVal = val;
}

inline void GGARunner::setPerformance(const GGAInstance& instance, double val)
{
    boost::unique_lock<boost::shared_mutex> wlock(m_rwMutex);
    m_performance[instance] = val;
}

inline void GGARunner::initializePerformanceData(size_t size)
{
    boost::unique_lock<boost::shared_mutex> wlock(m_rwMutex);
    m_performance.clear();
}


// ===== GGARunnerBuilder =====

class GGARunnerBuilder
{
public:
    GGARunnerBuilder();
    virtual ~GGARunnerBuilder();
    
    /**
     * @return A new runner initialised with the builder configuration.
     */
    GGARunner* build() const;
    
    GGARunnerBuilder& setId(int id);
    GGARunnerBuilder& setRuntimeTuning(bool runtimeTuning);
    GGARunnerBuilder& setPenaltyFactor(double penalty);
    GGARunnerBuilder& setCpuLimit(unsigned cpuLimit);
    
    GGARunnerBuilder& setInstances(const GGAInstanceVector* instances);
    GGARunnerBuilder& setParameterTree(const GGAParameterTree* ptree);
    GGARunnerBuilder& setGenome(const GGAGenome* genome);    
    
private:
    GGARunnerBuilder(const GGARunnerBuilder&);            // Intentionally unimplemented
    GGARunnerBuilder& operator=(const GGARunnerBuilder&); // Intentionally unimplemented
    
    // attributes
    int m_id;
    bool m_runtimeTuning;
    double m_penalty;
    unsigned m_cpuLimit;
    
    const GGAInstanceVector* m_instances;   
    const GGAParameterTree* m_ptree;
    const GGAGenome* m_genome;    
};

#endif // _GGA_RUNNER_HPP_
