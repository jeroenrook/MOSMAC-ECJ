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

/*
 * GGAOptions.h
 * Singleton class holding global options for GGA.
 */
#ifndef _GGA_OPTIONS_HPP_
#define _GGA_OPTIONS_HPP_

#include <boost/serialization/serialization.hpp>
#include <boost/program_options.hpp>

#include <string>

#include "ggatypedefs.hpp"
#include "GGAExceptions.hpp"


class GGAOptions {
public:
    
    // Public variables with the value result of the parsing
    //
    std::string prog_name;
    std::string param_tree_file;
    std::string instance_seed_file;
    //
    int num_generations;
    int pop_size;
    int num_threads;
    double pct_winners;
    int inst_start;
    int inst_finish;
    int gen_inst_finish;
    int seed;
    int seeded_genomes;
    double penalty_mult;
    int max_evals;    
    double mutation_rate;
    double subtree_split;
    double sigma_pct;
    int verbosity;
    int genome_age;
    //
    bool runtime_tuning;
    bool propagate_timeout;
    bool normalize_cont;
    bool send_sigusr1;
    //
    int learning_strategy;
    int learning_strategy_delay;
    int learning_strategy_step;
    //
    int target_algo_cpu_limit;
    int tuner_cutoff;
    int tuner_wall_cutoff;
    //
    std::string conf_file;
    std::string traj_file;
    std::string scen_file;
    //
    bool master;
    bool worker;
    StringVector ips;
    uint16_t port;
    int num_nodes;
    std::string start_worker_wrapper;
    //
    std::string clusters_file;
    //
    bool help;
    // NC parameters = no command line (read from scenario file only right now)
    bool nc_deterministic;
    std::string nc_execdir;
    std::string nc_prepend_cmd;

    // Public static methods
    //
    static const GGAOptions& instance();
    static GGAOptions& mutableInstance();
    static void deleteInstance();

    // destruct
    virtual ~GGAOptions();
        
    void parse(int argc, char** argv);
    bool parseHelpOptions(int argc, char** argv);
    void parseRemoteOptions(int argc, char** argv);

    void loadScenarioFile();

    std::string getHelpMessage() const;

private:
    GGAOptions();
    GGAOptions(const GGAOptions&);            // Intentionally unimplemented
    GGAOptions& operator=(const GGAOptions&); // Intentionally unimplemented

    // Options' value check
    void checkOptions();
    void checkGenericOptions();
    void checkRemoteOptions();

    // Options set up
    void setUpOptions();
    void setUpGenericOptions();
    void setUpBooleanOptions();
    void setUpLearningStrategyOptions();
    void setUpCutoffOptions();
    void setUpFileOptions();
    void setUpRemoteOptions();
    void setUpClusterOptions();
    void setUpHelpOptions();                // cmd line only
    void setUpNCOptions();                  // Scenario file only

    // serialization
    friend class boost::serialization::access;
    template<class Archiver>void serialize(Archiver&, const unsigned int);

    // Variables
    static GGAOptions* s_pInstance;

    boost::program_options::options_description m_help_opts;    // cmd only
    boost::program_options::options_description m_remote_opts;  // cmd only
    boost::program_options::options_description m_cluster_opts; // cmd only
    boost::program_options::options_description m_cmdline_opts; // cmd only
    boost::program_options::options_description m_conf_file_opts;
    boost::program_options::variables_map m_vm;
};


//==============================================================================
// GGAOptions public inline/template static methods

/**
 *
 */
inline const GGAOptions& GGAOptions::instance() 
{ 
    return *(s_pInstance != NULL ? 
                s_pInstance : (s_pInstance = new GGAOptions())); 
}

/**
 *
 */
inline GGAOptions& GGAOptions::mutableInstance()
{
    return const_cast<GGAOptions&>(GGAOptions::instance());
}

/**
 *
 */
inline void GGAOptions::deleteInstance()
{
    delete s_pInstance;
    s_pInstance = NULL;
}


//==============================================================================
// GGAOptions public inline/template methods.

//==============================================================================
// GGAOptions private inline/template methods.

template<typename Archiver>
void GGAOptions::serialize(Archiver& ar, const unsigned int version)
{   
    ar & prog_name;
    ar & param_tree_file;
    ar & instance_seed_file;
    ar & clusters_file;
    //
    ar & num_generations;
    ar & pop_size;
    ar & num_threads;
    ar & pct_winners;
    ar & inst_start;
    ar & inst_finish;
    ar & gen_inst_finish;
    ar & seed;
    ar & seeded_genomes;
    ar & penalty_mult;
    ar & max_evals;    
    ar & mutation_rate;
    ar & subtree_split;
    ar & sigma_pct;
    ar & verbosity;
    ar & genome_age;
    //
    ar & runtime_tuning;
    ar & normalize_cont;
    ar & send_sigusr1;
    //
    ar & learning_strategy;
    ar & learning_strategy_delay;
    ar & learning_strategy_step;
    //
    ar & target_algo_cpu_limit;
    ar & tuner_cutoff;
    ar & tuner_wall_cutoff;
    //
    ar & conf_file;
    ar & traj_file;
    ar & scen_file;
    //
    ar & master;
    ar & worker;
    ar & ips;
    ar & port;
    ar & num_nodes;
    ar & start_worker_wrapper;
    //
    ar & help;
    //
    ar & nc_deterministic;
    ar & nc_execdir;
    ar & nc_prepend_cmd;
}


#endif // _GGA_OPTIONS_HPP_
