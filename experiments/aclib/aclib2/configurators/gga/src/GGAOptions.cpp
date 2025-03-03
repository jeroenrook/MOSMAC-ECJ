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


#include <ctime>

#include <fstream>
#include <limits>
#include <sstream>
#include <string>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>

#include "ggatypedefs.hpp"
#include "GGAOptions.hpp"
#include "GGASystem.hpp"
#include "OutputLog.hpp"

namespace po = boost::program_options;

//==============================================================================
// GGAOptions public constants

//==============================================================================
// GGAOptions static variables

GGAOptions* GGAOptions::s_pInstance = NULL;

//==============================================================================
// GGAOptions public static methods

//==============================================================================
// GGAOptions public methods

/**
 *
 */
GGAOptions::GGAOptions() 
    : m_help_opts("Help options")
    , m_remote_opts("DGGA options")
    , m_cluster_opts("Clusters options")
    , m_cmdline_opts("Command line options")
    , m_conf_file_opts("Configuration file options")
    , m_vm()
{
    setUpOptions();
}


/**
 *
 */
GGAOptions::~GGAOptions() 
{ }


/**
 * @brief Parses command line and configuration file options.
 * 
 * Does not allow unrecognized options.
 */
void GGAOptions::parse(int argc, char** argv)
{
    if(argc < 3)
        throw GGAOptionsException("Invalid number of command line arguments.");
    
    prog_name = std::string(argv[0]);
    param_tree_file = std::string(argv[1]);
    instance_seed_file = std::string(argv[2]);

    /*set<std::string>(PROG_NAME, prog_name);
    set<std::string>(PARAM_TREE_FILE, param_tree_file);
    set<std::string>(INSTANCE_SEED_FILE, instance_seed_file);*/

    try {
        // parse_command_line skips the first argument, if we skip two
        // parse_command_line will start at the correct position.
        po::store(po::parse_command_line(argc - 2, argv + 2, m_cmdline_opts),
                  m_vm);
        po::notify(m_vm);

        if (!conf_file.empty()) {
            std::ifstream ifs(conf_file.c_str());
            if (!ifs)
                throw GGAFileNotFoundException(conf_file);

            po::store(po::parse_config_file(ifs, m_conf_file_opts), m_vm);
            po::notify(m_vm);
            ifs.close();
        }
    } catch (po::error& e) { // Hide boost errors
        throw GGAOptionsException(e.what());
    }

    checkOptions();
}


/**
 * @brief Parses command line searching only "help" flags.
 * 
 * Allows unrecognized options, which are ignored.
 *
 * @return true if any help option is found, false otherwise.
 */
bool GGAOptions::parseHelpOptions(int argc, char** argv)
{
    po::parsed_options parsed = po::command_line_parser(argc, argv)
                                    .options(m_help_opts)
                                    .allow_unregistered()
                                    .run();

    po::store(parsed, m_vm);
    po::notify(m_vm);

    return help;
}


/**
 * @brief Parses command line searching only "remote" flags and checks
 *        if their values are ok.
 *
 * Allows unrecognized options, which are ignored.
 *
 */
void GGAOptions::parseRemoteOptions(int argc, char** argv)
{
    po::parsed_options parsed = po::command_line_parser(argc, argv)
                                    .options(m_remote_opts)
                                    .allow_unregistered()
                                    .run();
    po::store(parsed, m_vm);
    po::notify(m_vm);

    checkRemoteOptions();
}


/**
 *  Loads the scenario file specified when parsing the options
 *  TODO: Better code
 */
void GGAOptions::loadScenarioFile()
{
    std::ifstream fp(scen_file.c_str());

    if(fp.is_open()) {        
        std::string line;

        while(std::getline(fp, line)) {
            std::stringstream ss;

            std::vector<std::string> lspl;
            boost::algorithm::split(lspl, line, 
                                    boost::algorithm::is_any_of("="));

            if(lspl.size() < 2)
                throw GGAScenarioFileException("Error reading scenario file: " +
                                            scen_file);

            std::string key (boost::algorithm::trim_copy(lspl[0]));
            std::string val (boost::algorithm::trim_copy(lspl[1]));
            
            if(key == "execdir") {
                nc_execdir = val;
            } else if(key == "deterministic") {
                ss << val;
                bool determ;
                ss >> determ;
                nc_deterministic = determ;
            } else if(key == "run_obj") {
                if(val == "runtime") {
                    runtime_tuning = true;
                } else {
                    runtime_tuning = false;
                }
            } else if(key == "overall_obj") {
                if(val == "mean") {
                    penalty_mult = 1.0;
                } else if(val == "mean10") {
                    penalty_mult = 10.0;
                } else {
                    GGAScenarioFileException("Warning: Scenario file: overal_obj;" 
                            " only mean/mean10 are currently supported.");
                }
            } else if(key == "cutoff_time") {
                ss << val;
                double algCutoff;
                ss >> algCutoff;
                target_algo_cpu_limit = algCutoff;
            } else if(key == "tuner-timeout") {
                ss << val;
                int tunCutoff;
                ss >> tunCutoff;
                tuner_cutoff = tunCutoff;
            } else if(key == "runcount_limit") {
                ss << val;
                int maxEvals;
                ss >> maxEvals;
                max_evals = maxEvals;
            } else if(key == "instance_file") {
                instance_seed_file = val;
            } else if(key == "test_instance_file") {
                LOG("Note: test instance file is not used by GGA.");
            } else if(key == "instance_seed_file") {
                instance_seed_file = val;
            } else if(key == "test_instance_seed_file") {
                LOG("Note: test instance seed file is not used by GGA.");
            } else if(key == "feature_file") {
                LOG("Note: feature file is not used by GGA.");
            } else if(key == "algo") {
                lspl.erase(lspl.begin());
                nc_prepend_cmd = boost::algorithm::join(lspl, "=");
                boost::algorithm::trim(nc_prepend_cmd);
            } else if(key == "paramfile") {
                lspl.erase(lspl.begin());
                std::string pftrim = boost::algorithm::join(lspl, "=");
                boost::algorithm::trim(pftrim);
                std::stringstream ss2;
                ss2 << pftrim << ".xml";
                param_tree_file = ss2.str();
            } else if(key == "wallclock_limit") {
                ss << val;
                int wcTimeout;
                ss >> wcTimeout;
                tuner_wall_cutoff = wcTimeout;
            } else {
                GGAScenarioFileException("Unknown key found in scenario file: " +
                                      key);
            }
        }
    }
}


/**
 *
 */
std::string GGAOptions::getHelpMessage() const
{
    boost::filesystem::path exepath(getExecutablePath());
    std::stringstream ss;

    ss << "Usage: " << exepath.filename().native()
       << " <param_tree_file> <instance_seed_file> [parameters]" << std::endl 
       << "Usage: " << exepath.filename().native() 
       << " --worker [DGGA options]" << std::endl << std::endl
       << "Parameters:" << std::endl;

    ss << m_cmdline_opts << std::endl;

    return ss.str();
}


//==============================================================================
// Private methods

/*****************/
/* Check Options */
/*****************/

/**
 *
 */
void GGAOptions::checkOptions()
{
/*  TODO
    checkPositionalsOptions();
    checkGenericOptions();
    checkBooleanOptions();
    checkLearningStrategyOptions();
    ...
*/
    checkGenericOptions();
    checkRemoteOptions();
}

void GGAOptions::checkGenericOptions()
{
    if (gen_inst_finish < 1) { 
        gen_inst_finish = ::round(0.75 * num_generations); 
        LOG_DEBUG("[GGAOptions] gen_inst_finish automatically set to: " << gen_inst_finish);
    }
}    

void GGAOptions::checkRemoteOptions()
{
    if (master && worker)
        throw GGAOptionsException("An instance of DGGA can't be master and "
                " worker at the same time.");
    
    if (master) {
        if (!m_vm.count("nodes"))
            throw GGAOptionsException("[DGGA] number of nodes is required.");
        if (!m_vm.count("start-worker-wrapper"))
            throw GGAOptionsException(
                "[DGGA] missing wrapper to start workers.");
    }

    if (worker) {
        if (!m_vm.count("ip"))
            throw GGAOptionsException(
                "[DGGA] missing ips to connect to master");
    }
}


/******************/
/* Options set up */
/******************/

/**
 *
 */
void GGAOptions::setUpOptions()
{
    setUpGenericOptions();
    setUpBooleanOptions();
    setUpLearningStrategyOptions();
    setUpCutoffOptions();
    setUpFileOptions();
    setUpRemoteOptions();
    setUpClusterOptions();
    setUpHelpOptions();
    setUpNCOptions();
}


/**
 *
 */
void GGAOptions::setUpGenericOptions()
{
    po::options_description generic("");

    generic.add_options()
        ("generations,g", 
            po::value<int>(&num_generations)->default_value(100),
            "# generations")

        ("population_size,p", 
            po::value<int>(&pop_size)->default_value(100),
            "# members")

        ("num_threads,t", 
            po::value<int>(&num_threads)->default_value(4),
            "Tournament size (number of members to run simultaneously)")

        ("pct_winners,w", 
            po::value<double>(&pct_winners)->default_value(0.125),
            "Percent winners per tournament [0.0, 1.0]")

        ("is", po::value<int>(&inst_start)->default_value(5),
            "Number of instances/gen per member at the start of tuning")

        ("ie", po::value<int>(&inst_finish)->default_value(100),
            "Number of instances/gen per member at the end of tuning")

        ("gf", po::value<int>(&gen_inst_finish)->default_value(-1),
            "Generation at which to reach the maximum number of legs, if negative is automatically"
            " set to 3/4 of the maximum number of generations.")

        ("seed", po::value<int>(&seed)->default_value(
                                            static_cast<int>(::time(NULL))),
            "Seed. Defaults to the current time")

        ("seeded_genomes", po::value<int>(&seeded_genomes)->default_value(0),
            "Number of \"seeded gnomes\" to create from some set of default"
            " parameters. One member will be created with the parameters and"
            " sg - 1 will be mutated from the parameters and inserted into the"
            " population with random genders")

        ("pe", po::value<double>(&penalty_mult)->default_value(1.0),
            "Penalty for reaching cutoff time per instance (performance[i] ="
            " penalty * cutoff")

        ("max_evals", po::value<int>(&max_evals)->default_value(
                                               std::numeric_limits<int>::max()),
            "Maximum evaluations of the objective function. This is not strict,"
            " but a best effort will be made to come close to this number")

        ("mutation_rate,m",
            po::value<double>(&mutation_rate)->default_value(0.1),
            "Mutation rate [0.0, 1.0]")

        ("st", po::value<double>(&subtree_split)->default_value(0.1),
            "Sub-tree split probability")

        ("sp", po::value<double>(&sigma_pct)->default_value(1.0),
            "Sigma percent. Determines sigma for the mutation gaussian.")

        ("verbosity,v",
            po::value<int>(&verbosity)->default_value(2),
            "Verbosity. Levels are:\n"
            "    0    No output\n"
            "    1    Errors only\n"
            "    2    Normal output\n"
            "    3    Verbose output\n"
            "    4    Very verbose- gives every last detail\n"
            "    5    Debug output")

        ("ga", po::value<int>(&genome_age)->default_value(3),
            "Gnomes age")
        ;

        m_cmdline_opts.add(generic);
        m_conf_file_opts.add(generic);
}


/**
 *
 */
void GGAOptions::setUpBooleanOptions()
{
    po::options_description bool_opts("Boolean options");

    bool_opts.add_options()
        ("rt", 
            po::value<bool>(&runtime_tuning)->default_value(true),
            "If false tunes for output, rather than runtime. The last line"
            " of algorithm output must be the objective value to minimize")

        ("propagate_timeout",
            po::bool_switch(&propagate_timeout),
            "If enabled, propagates timeout between minitournaments of the"
            " same generation.")

        ("nc", 
            po::value<bool>(&normalize_cont)->default_value(false),
            "Normalize all continuous variables")

        ("su1",
            po::value<bool>(&send_sigusr1)->default_value(false),
            "When set, sends SIGUSR1 before sending SIGTERM to kill processes")
        ;

    m_cmdline_opts.add(bool_opts);
    m_conf_file_opts.add(bool_opts);
}


/**
 *
 */
void GGAOptions::setUpLearningStrategyOptions()
{
    po::options_description learning_strategy_opts("Learning strategy options");

    learning_strategy_opts.add_options()
        ("ls", po::value<int>(&learning_strategy)->default_value(1),
            "Specify a learning strategy. {0 = TESTING, 1 = Linear,"
            " 2 = Step, 3 = Parabola, 4 = Exponential}")

        ("lsd", po::value<int>(&learning_strategy_delay)->default_value(0),
            "Generation to start the learning strategy at. Until then"
             " 'is' instances will be used.")

        ("lss", po::value<int>(&learning_strategy_step)->default_value(5),
            "# Generations / step for the step strategy.")
        ;

    m_cmdline_opts.add(learning_strategy_opts);
    m_conf_file_opts.add(learning_strategy_opts);
}


/**
 *
 */
void GGAOptions::setUpCutoffOptions()
{
    po::options_description cutoff_opts("Cutoff options");

    cutoff_opts.add_options()
        ("tacl",
            po::value<int>(&target_algo_cpu_limit)->default_value(30),
            "Target algorithm cpu time limit in seconds")

        ("tc", po::value<int>(&tuner_cutoff)->default_value(
                std::numeric_limits<int>::max()),
            "Tunner cutoff time in seconds")

        ("twc",
            po::value<int>(&tuner_wall_cutoff)->default_value(
                std::numeric_limits<int>::max()),
            "Tuner wallclock cutoff time in seconds")
        ;

    m_cmdline_opts.add(cutoff_opts);
    m_conf_file_opts.add(cutoff_opts);
}


/**
 *
 */
void GGAOptions::setUpFileOptions()
{
    po::options_description file_opts("File options");

    file_opts.add_options()
        ("conf_file", po::value<std::string>(&conf_file)->default_value(""),
            "Specify a configuration file. Command line options are preferred")
        
        ("traj_file", po::value<std::string>(&traj_file)->default_value(""),
            "Trajectory file output path")

        ("scen_file", po::value<std::string>(&scen_file)->default_value(""),
            "Specify a scenario file as used by ParamILS/SMAC."
            " Note this file overrides certain command line options.")
        ;

    m_cmdline_opts.add(file_opts);
    m_conf_file_opts.add(file_opts);
}


/**
 *
 */
void GGAOptions::setUpRemoteOptions()
{
    m_remote_opts.add_options()
        ("master", po::bool_switch(&master),
            "The program acts as the master node of DGGA. If 'master' or"
            " 'worker' are not specified the program will run as the classical"
            " GGA.")
        ("worker", po::bool_switch(&worker),
            "The program acts as a worker of DGGA. If 'master' or 'worker'"
            " are not specified the program will run as the classical GGA.")
        ("ip", po::value<StringVector>(&ips),
            "IPs to connect to the master (required: worker).")
        ("port", po::value<uint16_t>(&port)->default_value(6789),
            "Master port.")
        ("nodes", po::value<int>(&num_nodes),
            "Number of nodes (required: master).")
        ("start-worker-wrapper", po::value<std::string>(&start_worker_wrapper),
            "A wrapper whose job is start worker processes. It will receive"
            " as parameters the necessary information to start a worker"
            " instance: the first parameter provided is always the absolute"
            " path to the binary and then an arbitrary amount of parameters,"
            " which are the necessary flags, such as the master IPs, etc."
            "\n"
            "Special requirements:"
            "   - Can not block.\n"
            "   - Must return 0 on success.")
        ;

    m_cmdline_opts.add(m_remote_opts);
}

/**
 *
 */
void GGAOptions::setUpClusterOptions()
{
    m_cluster_opts.add_options()
        ("clusters_file", po::value<std::string>(&clusters_file),
            "Path to the file that contains one or more clusters that group the"
            " different instances. The file must contain one instance per line"
            " and one or more blank lines between two series of instances mark"
            " the end and beggining of a new cluster")
        ;
    
    m_cmdline_opts.add(m_cluster_opts);
}

/**
 *
 */
void GGAOptions::setUpHelpOptions()
{
    m_help_opts.add_options()
        ("help,h", po::bool_switch(&help),
            "Prints this help message and exits")
    ;

    m_cmdline_opts.add(m_help_opts);
}


/**
 *
 */
void GGAOptions::setUpNCOptions()
{
    /* Old options name
    const std::string GGAOptions::NC_DETERMINISTIC        ("deterministic");
    const std::string GGAOptions::NC_EXECDIR              ("execdir");
    const std::string GGAOptions::NC_PREPEND_CMD          ("prepend_cmd");
    m_vm.insert(std::make_pair(NC_DETERMINISTIC, 
                               po::variable_value(false, false)));
    m_vm.insert(std::make_pair(NC_EXECDIR, 
                               po::variable_value(std::string(), false)));
    m_vm.insert(std::make_pair(NC_PREPEND_CMD, 
                               po::variable_value(std::string(), false)));*/
    nc_deterministic = false;
    nc_execdir = "";
    nc_prepend_cmd = "";
}
