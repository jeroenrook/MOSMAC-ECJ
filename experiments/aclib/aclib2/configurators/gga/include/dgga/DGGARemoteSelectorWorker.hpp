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

#ifndef _GGA_REMOTE_SELECTOR_WORKER_HPP_
#define _GGA_REMOTE_SELECTOR_WORKER_HPP_

#include <boost/array.hpp>
#include <boost/thread.hpp>

#include "ggatypedefs.hpp"
#include "GGALocalSelector.hpp"

#include "dgga/net/DGGATcpConnection.hpp"


/**
 * Handles messages from DGGARemoteSelectorMaster and execute mini-tournaments
 * in the localhost using the GGALocalSelector.
 */
class DGGARemoteSelectorWorker 
{
public:
    DGGARemoteSelectorWorker(boost::asio::io_service& io_service);
    virtual ~DGGARemoteSelectorWorker();

    bool connect(const StringVector& ips, uint16_t port);

    void run();

private:
    // Intentionally unimplemented
    DGGARemoteSelectorWorker(const DGGARemoteSelectorWorker&);
    DGGARemoteSelectorWorker& operator=(const DGGARemoteSelectorWorker&);

    // Disconnection handler
    void handleDisconnection(DGGATcpConnection::pointer);

    // State handlers
    void handleHelloAck(DGGATcpConnection::pointer, const std::string&);
    void handleBegConfig(DGGATcpConnection::pointer, const std::string&);
    void handleGetConfig(DGGATcpConnection::pointer, const std::string&);
    void handleIDLEState(DGGATcpConnection::pointer, const std::string&);
    void handleRecvTourneyState(DGGATcpConnection::pointer, const std::string&);
    void handleWorkingState(DGGATcpConnection::pointer, const std::string&);

    // Helpful methods
    void executeTourney(DGGATcpConnection::pointer);

    bool isResetTimeout(const std::string& msg) const;
    void resetTimeout(const std::string& msg);

    //
    boost::scoped_ptr<GGALocalSelector> m_selector;
    std::string m_recv_data_buffer;

    boost::thread m_tourney_thread;

    // Network I/O
    boost::asio::io_service& m_io_service;
    DGGATcpConnection::pointer m_connection;
};

#endif // _GGA_REMOTE_SELECTOR_WORKER_HPP_