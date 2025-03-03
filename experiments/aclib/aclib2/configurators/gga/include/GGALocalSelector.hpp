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

#ifndef _GGA_LOCAL_SELECTOR_HPP_
#define _GGA_LOCAL_SELECTOR_HPP_

#include <limits>

#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>


#include "ggatypedefs.hpp"
#include "GGASelector.hpp"
#include "GGAGenome.hpp"
#include "GGARunner.hpp"


class GGALocalSelector : public GGASelector {
public:
    GGALocalSelector();
    virtual ~GGALocalSelector();

    void forceStop();
    void resetTimeout(double new_timeout);

    virtual GGASelectorResult select(const GGAGenomeVector& participants, 
                                     const GGAInstanceVector& instances,
                                     double timout = std::numeric_limits<double>::max());
private:
    // Intentionally unimplemented
    GGALocalSelector(const GGALocalSelector&);
    GGALocalSelector& operator=(const GGASelector&);

    void initializeTournament(const GGAGenomeVector& participants,
                              const GGAInstanceVector& instances,
                              double timeout);

    void registerInstancesResults(const RunnerVector& runners,
                                  const GGAInstanceVector& instances,
                                  GGAGenomeVector& genomes,
                                  GGASelectorResultBuilder& resultBuilder);
    
    void killRunners();
    void cleanUpRunners();
    void stopPoint();
           
    // attributes
    boost::mutex m_runners_mutex;
    RunnerVector m_runners;
    boost::interprocess::interprocess_semaphore m_runners_ctrl_semaphore;

    boost::mutex m_stop_mutex;
    bool m_stop_requested;

    const size_t m_num_threads;
    const bool m_runtime_tuning;
    const int m_cpu_limit;
    const double m_penalty;
    const double m_pct_winners;
    const bool m_propagate_timeout;
};

#endif // _GGA_LOCAL_SELECTOR_HPP_
