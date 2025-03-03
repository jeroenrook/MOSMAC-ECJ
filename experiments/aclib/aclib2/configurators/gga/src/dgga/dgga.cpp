// This file is part of GGA.
// 
// GGA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// GGA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with GGA.  If not, see <http://www.gnu.org/licenses/>.

#include <map>
#include <fstream>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

#include "gga.hpp"
#include "GGAOptions.hpp"
#include "GGAParameterTree.hpp"
#include "GGASystem.hpp"
#include "GGATournament.hpp"
#include "GGAUtil.hpp"
#include "OutputLog.hpp"

#include "dgga/dgga.hpp"
#include "dgga/dgga_messages.hpp"
#include "dgga/DGGARemoteSelectorMaster.hpp"
#include "dgga/DGGARemoteSelectorWorker.hpp"

// Function prototypes
//
int runMaster();
int runWorker();

std::string createStartWorkerCmd(const std::string&, unsigned cores,
                                 uint16_t port);

/**
 *
 */
int runDGGA() 
{
    LOG("runDGGA");
    const GGAOptions& opts = GGAOptions::instance();
    return opts.master ? runMaster() : runWorker();
}


/**
 *
 */
int runMaster()
{
    LOG("DGGA MASTER");
    const GGAOptions& opts = GGAOptions::instance();
    
    GGAInstances instances;
    parseLocalData(&instances);

    boost::asio::io_service io_service;    
    DGGARemoteSelectorMaster selector(io_service);

    // Start workers command
    selector.setStartWorkerCmd(
        createStartWorkerCmd(opts.start_worker_wrapper, opts.num_threads, 
                             opts.port));
        
    GGATournament tournament(instances, selector);
    tournament.populate();

    if(opts.nc_execdir != "")
        boost::filesystem::current_path(opts.nc_execdir.c_str());
   
    LOG("TOURNAMENT START"); 
    tournament.run();
    LOG("TOURNAMENT END");

    return EXIT_SUCCESS;
}


/**
 *
 */
int runWorker()
{
    LOG("DGGA WORKER");
    const GGAOptions& opts = GGAOptions::instance();

    boost::asio::io_service io_service;
    DGGARemoteSelectorWorker worker(io_service);
    bool connected = worker.connect(opts.ips, opts.port);

    if (!connected) {
        LOG_ERROR("[Worker] Unable to connect to master");
        LOG_ERROR("\tIPs: " << OutputLog::stringVectorToString(opts.ips));
        LOG_ERROR("\tPort: " << opts.port);
        return EXIT_FAILURE;
    }

    LOG("WORKER START");
    worker.run();
    LOG("WORKER END");

    return EXIT_SUCCESS;
}


// === Utility ===

/**
 *
 */
std::string createStartWorkerCmd(const std::string& wrapper, unsigned int cores,
                                 uint16_t port)
{
    std::stringstream ss;

    if (!wrapper.empty())
        ss << wrapper << " ";

    ss << cores << " " << getExecutablePath() << " --worker --port " << port;

    std::vector<std::string> ips = getLocalIPAddrs();

    for (size_t i = 0; i < ips.size(); ++i)
        ss << " --ip " << ips[i];

    return ss.str();
}