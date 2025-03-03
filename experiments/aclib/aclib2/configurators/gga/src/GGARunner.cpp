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


#include <cerrno>
#include <csignal>
#include <cstring>

#include <algorithm>
#include <iostream>
#include <limits>
#include <fstream>
#include <stdexcept>

#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/bind.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include "GGAExceptions.hpp"
#include "GGAOSProber.hpp"
#include "GGARandEngine.hpp"
#include "GGARunner.hpp"
#include "GGAUtil.hpp"
#include "OutputLog.hpp"

#if defined(OS_LINUX)
# include <sys/prctl.h>
#endif

/* namespace(s) alias */
namespace bio = boost::iostreams;

/* Local typedefs */
typedef boost::char_separator<char> CharSeparator;
typedef boost::tokenizer<CharSeparator> CharTokenizer;
typedef const boost::iterator_range<std::string::const_iterator> StringRange;

//==============================================================================
// Functions local to this file

/**
 *
 */
static inline double getUserTimeInSeconds(const struct rusage& ru)
{
    struct timeval tim;
    tim = ru.ru_utime;
    return (double)tim.tv_sec + (double)tim.tv_usec / CLOCKS_PER_SEC;
}


//==================================================================================================
// GGARunner public methods

/**
 *
 */
GGARunner::~GGARunner() 
{
    // Interrupt and wait thread
    m_runThread.interrupt();
    m_runThread.join();

    // Release commands
    m_commands.clear();
}


/*
 * Begins execution of the target algorithm. Note that run() returns 
 * immediately due to thread creation.
 *
 * When this thread is done, it will unlock the sontrol to indicate that that
 * the objective value stored is final.
 *
 */
void GGARunner::run(boost::interprocess::interprocess_semaphore& ctrlSem)
{
    if (!m_runThread.joinable() && !hasFinished()) {
        m_runThread = boost::thread(boost::bind(
            &GGARunner::runThread, this, boost::ref(ctrlSem)));
    }
}


/*
 * Cancels the execution of the running thread. This call waits until the thread
 * has completely stopped.
 */
void GGARunner::interrupt()
{
    if (m_runThread.joinable()) {
        if (childPid() != 0)
            kill(childPid(), SIGKILL);
        m_runThread.interrupt();
        try {
            m_runThread.join();
        } catch (boost::thread_interrupted& e)
        { }
    }
}

//==================================================================================================
// GGARunner private methods

/**
 *
 */
GGARunner::GGARunner(int id, bool runtimeTuning, double penalty, unsigned cpuLimit,
                     const GGAInstanceVector& instances, const GGAParameterTree& ptree,
                     const GGAGenome& genome)
    : m_commands()
    
    , m_id(id)    
    , m_runtimeTuning(runtimeTuning)
    , m_penalty(penalty)
    , m_cpuLimit(cpuLimit)
    , m_instances(instances)

    , m_runThread()
    , m_rwMutex()

    , m_evals(0)
    , m_childPid(0)
    , m_hasFinished(false)
    , m_objVal(0.0)
    , m_timeout(std::numeric_limits<double>::max())
    , m_performance()
{ 
    buildCommands(ptree, genome);
}


void GGARunner::buildCommands(const GGAParameterTree& ptree, 
                              const GGAGenome& genome)
{
    hasFinished(false);
    setObjectiveValue(0.0);
    initializePerformanceData(m_instances.size());
    
    // Rotate so each runner starts with a different instance
    std::rotate(m_instances.begin(),
                m_instances.begin() + this->m_id % m_instances.size(),
                m_instances.end());    
    
    GGAInstanceVector::const_iterator itr, end = m_instances.end();
    for(itr = m_instances.begin(); itr != end; ++itr) {
        GGAInstancePerformanceMap::const_iterator pitr = 
                                        genome.allPerformances().find(*itr);

        if (pitr != genome.allPerformances().end()) {
            setObjectiveValue(objValue() + pitr->second);
            setPerformance(*itr, pitr->second);
        } else {
            std::string tcmd = makeCommand(ptree, genome.genome(), *itr);
            LOG_DEBUG("GGAGenome (" << &genome << "): " << tcmd);
                        
            StringVector splitCmd;
            CharSeparator separators(" \t\n\v");
            CharTokenizer tok(tcmd, separators);
            for(CharTokenizer::iterator it = tok.begin(); it != tok.end(); ++it)
                splitCmd.push_back(*it);

            m_commands.push_back(splitCmd);
        }
    }
}


/**
 *
 */
void GGARunner::runThread(boost::interprocess::interprocess_semaphore& ctrlSem) 
{
    // TODO Don't do this if we are setting the objective value from output
    // (runtime or not)
    if(objValue() > 0.0)
        LOG("GGARunner::run(): Runner " << m_id << " starting with a"
            " performance time of " << objValue() << ".");

    for (size_t index = 0; index < m_commands.size(); ++index) {       
        boost::this_thread::interruption_point();

        if (m_runtimeTuning && objValue() > getTimeout()) { 
            LOG("GGARunner::run(): Runner " << m_id << " skipping next instance"
                " (" << index << ") due to timeout... (" << objValue() << " > "
                <<  getTimeout() << ")");

            //performance(index, m_cpu_limit);
            setObjectiveValue(objValue() + m_cpuLimit); // Old version does not do this.
            continue; 

        } else if (!m_runtimeTuning) { 
            LOG("GGARunner::run(): Runner " << m_id << " skipping next instance"
                " (" << index << ") due to timeout... (" << objValue() << " > "
                <<  getTimeout() << ")");

            setObjectiveValue(objValue() + m_cpuLimit * m_penalty);
            continue; 
        }

        LOG("GGARunner::run(): Runner " << m_id << " running next instance (" 
            << index << ") ... (" << objValue() << " <= " <<  getTimeout() << ")");
        runCommand(m_commands[index], index);
        ++m_evals;
    }

    if (m_runtimeTuning) {
        LOG("*   Runner " << m_id << ": performance: " <<  objValue() 
            << "  cutoff: " << m_cpuLimit << "   penalty: " << m_penalty
            << "    timeout: " << getTimeout());  
    } else { // ???
    }

    hasFinished(true);
    ctrlSem.post();
}


/**
 *
 */
void GGARunner::runCommand(const StringVector& cmd, size_t index)
{
    int pfd[2];
    if (-1 == ::pipe(pfd))
        LOG_ERROR("Error creating pipe");

    pid_t cpid = fork();

    if(cpid == 0) {                // child
        runCommandChild(pfd, cmd);
    } else if(cpid > 0) {          // parent
        childPid(cpid);        
        runCommandParent(pfd, cpid, index);
    } else {                       // fork failed
        LOG_ERROR("Failed to fork child!");
        // TODO Exception!
    }
}


/**
 * Parent code after fork
 */
void GGARunner::runCommandParent(int pipe[2], const int child_pid,
                                 const size_t index)
{
    bio::stream_buffer<bio::file_descriptor_source> 
        fpstream(pipe[0], bio::close_handle); // Automatically closes pipe[0] 
    std::istream in(&fpstream);
    ::close(pipe[1]);

    std::string last_line, line;
    while(in) {
        last_line = line; // The last getline fails and sets the error flags
        // TODO Do the following:
        // -- Optionally check for error state
        // -- Optionally grab CPU time from the target algo
        // -- None runtime scenarios need to grep for the objective            
        std::getline(in, line);
    }

    struct rusage ru;
    int status;
    wait4(child_pid, &status, 0, &ru);
    double user_time = getUserTimeInSeconds(ru);

    double perf = m_runtimeTuning ? 
                    computeRuntimePerformance(status, user_time, last_line) :
                    computeOutputPerformance(status, last_line);
    setPerformance(m_instances[index], perf);
    setObjectiveValue(objValue() + perf);
}


/**
 * Child body
 */
void GGARunner::runCommandChild(int pipe[2], const StringVector& cmd)
{
#if defined(OS_LINUX)
    ::prctl(PR_SET_PDEATHSIG, SIGTERM);
#endif

    //::dup2(pipe[1], STDERR_FILENO);  // STDERR -> pipe[1]
    ::dup2(pipe[1], STDOUT_FILENO);  // STDOUT -> pipe[1]
    ::close(pipe[0]); ::close(pipe[1]);
    
    // Extract c-string representation of the command arguments
    std::vector<char*> cstr_cmd;
    for (size_t i = 0; i < cmd.size(); ++i)
        cstr_cmd.push_back(const_cast<char*>(cmd[i].c_str()));
    cstr_cmd.push_back(NULL);

    setProcessResourceLimits();
    int res = execvp(cstr_cmd[0], &cstr_cmd[0]);
    if(res == -1) // TODO Exception
        LOG_ERROR("execvp failed: " << strerror(errno));
    
    exit(0); // should never be reached
}


/**
 *
 */
double GGARunner::computeRuntimePerformance(int exit_status, double user_time,
                                            const std::string& last_line)
{
    static const std::string CRASHED_STR("CRASHED");

    if (WIFEXITED(exit_status)) {
        bool crashed = false;
        if(boost::istarts_with(last_line, "Result for")) {
            StringRange input(last_line.begin(), last_line.end());
            StringRange search(CRASHED_STR.begin(), CRASHED_STR.end());
            if (boost::ifind_first(input, search))
                crashed = true;
        }

        if (crashed || user_time > (m_cpuLimit - 0.5))
            return m_cpuLimit * m_penalty;
        else
            return user_time;

    } else {//if (WIFSIGNALED(exit_status)) {
        return m_cpuLimit * m_penalty;
    }
}


/**
 *
 */
double GGARunner::computeOutputPerformance(int exit_status,
                                           const std::string& last_line)
{
    try {
        return boost::lexical_cast<double>(last_line);
    } catch (boost::bad_lexical_cast& e) {
        throw GGAException("[GGARunner::computeOutputPerformance] Can not"
            " convert last line to <double>.");
    }
}


/**
 *
 */
void GGARunner::setProcessResourceLimits()
{
    struct rlimit cpu_lim, core_lim;

    core_lim.rlim_cur = 0;
    core_lim.rlim_max = 0;
    if (::setrlimit(RLIMIT_CORE, &core_lim) != 0)
        throw std::runtime_error("[GGARunner::setProcessResourceLimits] Unable"
            " to disable core files.");

    cpu_lim.rlim_cur = m_cpuLimit + 30;  // SIGXCPU
    cpu_lim.rlim_max = m_cpuLimit + 30;  // SIGKILL
    if (::setrlimit(RLIMIT_CPU, &cpu_lim) != 0)
        throw std::runtime_error("[GGARunner::setProcessResourceLimits] Unable"
            " to set the cpu time limit.");
}



//
// ===== GGARunnerBuilder =====
//



GGARunnerBuilder::GGARunnerBuilder()
    : m_id(-1)
    , m_runtimeTuning(true)
    , m_penalty(1.0)
    , m_cpuLimit(std::numeric_limits<int>::max())
    , m_instances(NULL)
    , m_ptree(NULL)
    , m_genome(NULL)
{ }

GGARunnerBuilder::~GGARunnerBuilder()
{ }

GGARunner* GGARunnerBuilder::build() const
{
    if (m_ptree == NULL) { 
        throw GGANullPointerException("[GGARunnerBuilder::build] null parameter tree");
    } else if (m_genome == NULL) {
        throw GGANullPointerException("[GGARunnerBuilder::build] null genome");
    } else if (m_instances == NULL) {
        throw GGANullPointerException("[GGARunnerBuilder::build] null instances vector");
    }
    
    return new GGARunner(m_id, m_runtimeTuning, m_penalty, m_cpuLimit,
                         *m_instances, *m_ptree, *m_genome);
    
}


GGARunnerBuilder& GGARunnerBuilder::setId(int id)
{
    m_id = id;
    return *this;
}

GGARunnerBuilder& GGARunnerBuilder::setRuntimeTuning(bool runtimeTuning) 
{ 
    m_runtimeTuning = runtimeTuning; 
    return *this;
}

GGARunnerBuilder& GGARunnerBuilder::setPenaltyFactor(double penalty)
{
    m_penalty = penalty;
    return *this;
}

GGARunnerBuilder& GGARunnerBuilder::setCpuLimit(unsigned cpuLimit) 
{
    m_cpuLimit = cpuLimit;
    return *this;
}

GGARunnerBuilder& GGARunnerBuilder::setInstances(const GGAInstanceVector* instances)
{
    m_instances = instances;
    return *this;
}

GGARunnerBuilder& GGARunnerBuilder::setParameterTree(const GGAParameterTree* ptree)
{
    m_ptree = ptree;
    return *this;
}

GGARunnerBuilder& GGARunnerBuilder::setGenome(const GGAGenome* genome)
{
    m_genome = genome;
    return *this;
}
