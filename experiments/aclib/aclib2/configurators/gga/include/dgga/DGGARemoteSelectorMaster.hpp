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

#ifndef _GGA_REMOTE_SELECTOR_MASTER_HPP_
#define _GGA_REMOTE_SELECTOR_MASTER_HPP_

#include <map>
#include <set>
#include <sstream>
#include <string>

#include <boost/asio.hpp>

#include "GGAInstance.hpp"
#include "GGASelector.hpp"
#include "GGAGenome.hpp"

#include "dgga/net/DGGATcpConnection.hpp"
#include "dgga/net/DGGATcpAcceptor.hpp"


/**
 * Handles connections and distributed jobs to workers.
 *
 * (NOT THREAD SAFE)
 */
class DGGARemoteSelectorMaster : public GGASelector
{
public:
    // construct/destruct
    DGGARemoteSelectorMaster(boost::asio::io_service& ioService);
    virtual ~DGGARemoteSelectorMaster();

    // Set/Get start worker cmd
    const std::string& getStartWorkerCmd() const;
    void setStartWorkerCmd(const std::string& cmd);

    // Add new jobs to the internal queue
    virtual GGASelectorResult select(const GGAGenomeVector& participants,
                                     const GGAInstanceVector& instances,
                                     double timout = std::numeric_limits<double>::max());

private:
    // start, accept, poll
    void startWorkers();
    void pollWorkers();

    // Connection handlers
    void handleNewConnection(DGGATcpConnection::pointer);
    void handleDisconnection(DGGATcpConnection::pointer);

    // Worker state handlers
    void handleInitState(DGGATcpConnection::pointer, const std::string&);
    void handleGetConfig(DGGATcpConnection::pointer, const std::string&);
    void handleConfigAck(DGGATcpConnection::pointer, const std::string&);
    void handleIDLEState(DGGATcpConnection::pointer, const std::string&);
    void handleWorkingState(DGGATcpConnection::pointer, const std::string&);
    void handleResultsState(DGGATcpConnection::pointer, const std::string&);

    // Manage tourney
    bool sendTourney(DGGATcpConnection::pointer);
    void rollbackTourney(DGGATcpConnection::pointer);
    void recoverResults(const std::string&);
    void resetTimeout();

    // Private variables
    // Nodes information
    size_t nodes_;     ///< # cluster nodes to use.
    size_t cores_;     ///< # of cores to request per node.

    std::string startWorkerCmd_; ///< System command to request an slave.

    // Requested nodes
    size_t requestedNodes_;   ///< Nodes requested to the queue system.
    size_t avaliableNodes_;   ///< Nodes already awake and willing to work.

    // Tourneys
    GGAGenomeVector participants_;
    GGAInstanceVector instances_;

    GGASelectorResultBuilder resultBuilder_;

    UIntVector tourneySizes_;
    size_t nparticipantsCompeting_;
    bool propagateTimeout_;
    double timeout_;


    std::map<DGGATcpConnection::pointer, GGAGenomeVector> participantsInWorker_;    

    // Workers
    std::set<DGGATcpConnection::pointer> connections_;
    std::map<DGGATcpConnection::pointer, bool> connectionIdle_;
    std::map<DGGATcpConnection::pointer, std::string> connectionAddr_;
    std::map<DGGATcpConnection::pointer, std::string> connectionResults_;

    // Network I/O
    boost::asio::io_service& ioService;
    DGGATcpAcceptor acceptor_;
};


//==============================================================================
// DGGARemoteSelectorMaster public in-line methods

inline const std::string& DGGARemoteSelectorMaster::getStartWorkerCmd() const
{ return startWorkerCmd_; }

inline void DGGARemoteSelectorMaster::setStartWorkerCmd(const std::string& cmd)
{ startWorkerCmd_ = cmd; }

#endif // _GGA_REMOTE_SELECTOR_MASTER_HPP_
