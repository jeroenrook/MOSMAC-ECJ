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

#include <boost/bind.hpp>
#include "dgga/net/DGGATcpAcceptor.hpp"

#include "OutputLog.hpp"
//==============================================================================
// DGGATcpAcceptor public methods

/**
 *
 */
DGGATcpAcceptor::DGGATcpAcceptor(boost::asio::io_service& io_service, 
                                 uint16_t port)
    : m_acceptor(io_service)
{
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), port);
    m_acceptor.open(ep.protocol());
    //m_acceptor.set_option(boost::asio::ip::v6_only(false));
    m_acceptor.bind(ep);
    m_acceptor.listen();

    startAsyncAccept();
}


/**
 *
 */
DGGATcpAcceptor::~DGGATcpAcceptor()
{ }

//==============================================================================
// DGGATcpAcceptor private methods

/**
 *
 */
void DGGATcpAcceptor::startAsyncAccept()
{
    DGGATcpConnection::pointer new_connection = 
                        DGGATcpConnection::create(m_acceptor.get_io_service());

    m_acceptor.async_accept(new_connection->socket(),
        boost::bind(&DGGATcpAcceptor::handleAccept, this, new_connection,
                    boost::asio::placeholders::error));
}


/**
 * Accepts and dispatches a new connection, if there is not a handler to attend
 * the new connection, it is closed when the function scope finishes.
 */
void DGGATcpAcceptor::handleAccept(DGGATcpConnection::pointer new_connection,
                                const boost::system::error_code& error)
{
    if (!error && !m_new_connection_handler.empty())
        m_new_connection_handler(new_connection);

    startAsyncAccept();
}