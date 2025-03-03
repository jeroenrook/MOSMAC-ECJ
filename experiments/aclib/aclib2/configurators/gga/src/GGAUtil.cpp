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
#include <stdexcept>

#include "GGALocalSelector.hpp"
#include "GGAOptions.hpp"
#include "GGAUtil.hpp"


/**
 * Builds a command from the genome and provided progValues
 * progValues can be any one of the protected names given in PROTECTED_NAMES
 */
std::string makeCommand(const GGAParameterTree& ptree, const GenomeMap& genome,
                        const GGAInstance& inst) 
{
    const GGAOptions& opts = GGAOptions::instance();
    StringMap progVals;
    std::stringstream ss;
   
    ss << inst.getSeed();
    progVals["seed"] = ss.str();
    progVals["instance"] = inst.getInstance();

    // The following three lines don't seem to work, probably because I never implemented instance specific timeouts
//     std::stringstream ss2;
//     ss2 << inst->timeout();
//     progVals["cutoff"] = ss2.str();

    std::stringstream ss2;    
    ss2 << opts.target_algo_cpu_limit;
    progVals["cutoff"] = ss2.str();

    const StringVector& extra = inst.getExtra();
    for(size_t i = 0; i < extra.size() && i < 3; ++i) {
        std::stringstream ss;
        ss << "extra" << i;
        progVals[ss.str()] = extra[i];
    }

    return makeCommand(ptree, genome, progVals);
}


/**
 *
 */
std::string makeCommand(const GGAParameterTree& ptree, const GenomeMap& genome,
                        const StringMap& progValues)
{
    // Iterate over the command and insert variables from the genome when necessary
    std::stringstream ss;
    const GGACommand& cmd = *ptree.command();
    const CommandArgVector& arguments = cmd.arguments();

    CommandArgVector::const_iterator itr;
    for(itr = arguments.begin(); itr != arguments.end(); ++itr) {
        if(itr->type() == GGACommandArg::FIXED) {
            ss << itr->fixedValue();

        } else { 
            if(itr->isProtected()) {
                ss << mapAt(progValues, itr->name());
            } else {
                const std::string& varName = itr->name();
                GGAParameter::pointer param = mapAt(ptree.parameters(),
                                                    varName);

                if(cmd.isOrPathOk(param, genome)) {
                    if(param->isFlag()) {
                        if(mapAt(genome, varName).getLong() == 1)
                            ss << mapAt(genome, varName);
                    } else {
                        ss << param->prefix();
                        ss << mapAt(genome, varName);
                    }
                }
            }
        }
    }

    return ss.str();
}


/**
 *
 */
UIntVector balanceTournaments(size_t participants, size_t tourny_size)
{
    size_t numExtra = participants % tourny_size;
    UIntVector tournamentSizes;

    for(unsigned i = 0; i < participants / tourny_size; ++i) {
        tournamentSizes.push_back(tourny_size);
    }

    if(numExtra == 0) return tournamentSizes;
    
    tournamentSizes.push_back(numExtra);
    size_t curPos = 0;
    size_t i = 0;
    while ((i < tourny_size - numExtra)
            && tournamentSizes[curPos] - 1 > tournamentSizes.back()) 
    {
        tournamentSizes[curPos] -= 1;
        tournamentSizes[tournamentSizes.size() - 1] += 1;
        // Circle around the tournaments except for the last tournament, which is the "extra" tournament
        curPos = (curPos + 1) % (tournamentSizes.size() - 1);
        ++i;
    }

    return tournamentSizes;
}


/**
 * Creates and returns the learning strategy associated with the value of 
 * "strategy" in GGAOptions.
 *
 * @warning The returned pointer must freed with delete or stored in a smart
 *          pointer.
 *
 * @throw std::invalid_argument if "strategy" is not one of the values
 *        specified in GGAOptions
 */
GGALearningStrategy* createLearningStrategy(const GGAInstances& instances)
{
    const GGAOptions& opts = GGAOptions::instance();

    switch(opts.learning_strategy) {
        case 0:
            return new GGALearningStrategy(instances);
        case 1:
            return new GGALearningStrategyLinear(instances);            
        case 2:
            return new GGALearningStrategyStep(instances);
        case 3:
            return new GGALearningStrategyParabola(instances);
        case 4:
            return new GGALearningStrategyExp(instances);
        case 5:
            return new GGALearningStrategyRndCluster(instances);
        default:
            throw std::invalid_argument("Unknown strategy, valid values are"
                                        " [0, 1, 2, 3, 4].");
    }
}

/**
 *
 */
void outputTrajectory(const GGAParameterTree& ptree, const GenomeMap& genome,
                      double utime, double wtime) 
{
    using namespace std;

    const GGAOptions& opts = GGAOptions::instance();
    const GGACommand& cmd = *ptree.command();

    if(!opts.traj_file.empty()) {
        ofstream fp(opts.traj_file.c_str(), fstream::out | fstream::app);

        if(fp.is_open()) {
            fp << utime << ", 0.0, " << wtime << ", 1, 0.0, ";

            GenomeMap::const_iterator it, itend = genome.end();
            for(it = genome.begin(); it != itend; ++it) {

                if(it->first.find("__dummy__") == string::npos) {
                    GGAParameter::pointer param = mapAt(ptree.parameters(),
                                                        it->first);

                    if(cmd.isOrPathOk(param, genome)) {
                        fp << param->trajName() << "=" << it->second;
                        if(1 < distance(it, itend))
                            fp << ", ";
                    }
                }
            }
            fp << std::endl;
        }
        fp.close();
    }
}

/**
 *
 */
void printOptions()
{
    const GGAOptions& opts = GGAOptions::instance();
    LOG("PROGRAM OPTIONS:");
    LOG("Program name: " << opts.prog_name);
    LOG("Parameter Tree file: " << opts.param_tree_file);
    LOG("Instance Seed file: " << opts.instance_seed_file);
    LOG("---");
    LOG("Generations: " << opts.num_generations);
    LOG("Population size: " << opts.pop_size);
    LOG("Num. Threads: " << opts.num_threads);
    LOG("Pct winners: " << opts.pct_winners);
    LOG("# Instances start: " << opts.inst_start);
    LOG("# Instances end: " << opts.inst_finish);
    LOG("Gen inst finish: " << opts.gen_inst_finish);
    LOG("Seed: " << opts.seed);
    LOG("# Seeded Members: " << opts.seeded_genomes);
    LOG("Cutoff penalty multiplier: " << opts.penalty_mult);
    LOG("Max obj evals: " << opts.max_evals);
    LOG("Mutation rate: " << opts.mutation_rate);
    LOG("Sub-tree Split: " << opts.subtree_split);
    LOG("Sigma %: " << opts.sigma_pct);
    LOG("Genome age: " << opts.genome_age);
    LOG("Verbosity: " << opts.verbosity);
    LOG("---");
    LOG("Runtime tuning: " << opts.runtime_tuning);
    LOG("Normalize cont: " << opts.normalize_cont);
    LOG("Send SIGUSR1: " << opts.send_sigusr1);
    LOG("---");
    LOG("Learning Strategy: " << opts.learning_strategy);
    LOG("Learning Strategy Delay: " << opts.learning_strategy_delay);
    LOG("Learning Strategy Step size: " << opts.learning_strategy_step);
    LOG("---");
    LOG("Target algorithm cutoff: " << opts.target_algo_cpu_limit << "s");
    LOG("Tuner cutoff: " << opts.tuner_cutoff << "s");
    LOG("Tuner wallclock cutoff: " << opts.tuner_wall_cutoff << "s");
    LOG("---");
    LOG("Configuration file: \"" << opts.conf_file << "\"");
    LOG("Trajectory file: \"" << opts.traj_file << "\"");
    LOG("Scenario file (in): \"" << opts.scen_file << "\"");
    LOG("---");
    LOG("Run as master: " << opts.master);
    LOG("Run as worker: " << opts.worker);
    LOG("Master Ifaces IPs: " << OutputLog::stringVectorToString(opts.ips));
    LOG("Master listen port: " << opts.port);
    LOG("Desired # workers: " << opts.num_nodes);    
    LOG("---");
}
