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

#ifndef _GGA_TCP_SERVER_HPP_
#define _GGA_TCP_SERVER_HPP_

#include <boost/asio.hpp>
#include <boost/function.hpp>

#include "dgga/net/DGGATcpConnection.hpp"


class DGGATcpAcceptor
{
public:
    typedef boost::function1<void, DGGATcpConnection::pointer> 
                                                        NewConnectionHandler;

    DGGATcpAcceptor(boost::asio::io_service&, uint16_t);
    virtual ~DGGATcpAcceptor();

    void setNewConnectionHandler(NewConnectionHandler);

private:
    // Intentionally unimplemented
    DGGATcpAcceptor();
    DGGATcpAcceptor(const DGGATcpAcceptor&);
    DGGATcpAcceptor& operator=(const DGGATcpAcceptor&);

    //
    void startAsyncAccept();

    // I/O handlers
    void handleAccept(DGGATcpConnection::pointer,
                      const boost::system::error_code&);

    //
    boost::asio::ip::tcp::acceptor m_acceptor;

    NewConnectionHandler m_new_connection_handler;
};

//==============================================================================
// DGGATcpAcceptor public inline methods

/**
 *
 */
inline void DGGATcpAcceptor::setNewConnectionHandler(NewConnectionHandler handler)
{ m_new_connection_handler = handler; }


#endif // _GGA_TCP_SERVER_HPP_