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


#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>


#include "gga.hpp"
#include "ggatypedefs.hpp"
#include "GGAExceptions.hpp"
#include "GGAGenome.hpp"
#include "GGAInstance.hpp"
#include "GGALocalSelector.hpp"
#include "GGAOptions.hpp"
#include "GGAPopulation.hpp"
#include "GGATournament.hpp"
#include "GGAUtil.hpp"
#include "OutputLog.hpp"

// Function prototypes
//

/**
 *
 */
int runGGA() 
{
    const GGAOptions& opts = GGAOptions::instance();
    
    GGAInstances instances;
    parseLocalData(&instances);

    // Initialize tournament + add population
    GGALocalSelector selector;
        
    GGATournament tournament(instances, selector);
    tournament.populate();  

    if(opts.nc_execdir != "")
        boost::filesystem::current_path(opts.nc_execdir.c_str());
    
    // Run tournament
    tournament.run();
    
    // No processes should be running at this point, but just in case...
    //kill(0, SIGTERM);
    return 0;
}

/**
 *
 */
void parseLocalData(GGAInstances* instances)
{
    GGAOptions& opts = GGAOptions::mutableInstance();

    OutputLog::setReportLevel(static_cast<OutputLog::Level>(opts.verbosity));
    printOptions();

    if (!boost::filesystem::exists(opts.param_tree_file))
        throw GGAFileNotFoundException(opts.param_tree_file);

    // Parse parameter Tree
    GGAParameterTree& paramTree = GGAParameterTree::mutableInstance();  
    paramTree.parseTreeFile(opts.param_tree_file);
    LOG("Parameter Tree:" << std::endl << paramTree.toString());
    LOG("Command: " << std::endl << paramTree.command()->rawCommand());
    
    // Parse instances
    instances->loadInstancesFile(opts.instance_seed_file);
    if (!opts.clusters_file.empty()) {
        if (!boost::filesystem::exists(opts.clusters_file))
            throw GGAFileNotFoundException(opts.clusters_file);
        instances->loadInstanceClustersFile(opts.clusters_file);
    }
}
