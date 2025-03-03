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

#include <cassert>

#include <stdexcept>

#include <boost/algorithm/string/trim.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "dgga/net/DGGATcpConnection.hpp"
#include "OutputLog.hpp"


//==============================================================================
// DGGATcpConnection public constants
const std::string DGGATcpConnection::DEF_EOM ("\n\n");

//==============================================================================
// DGGATcpConnection public static methods

/**
 *
 */
DGGATcpConnection::pointer DGGATcpConnection::create(
                                        boost::asio::io_service& io_service)
{
    return pointer(new DGGATcpConnection(io_service));
}

//==============================================================================
// DGGATcpConnection public methods

/**
 *
 */
DGGATcpConnection::~DGGATcpConnection()
{ }


/**
 *
 */
void DGGATcpConnection::close()
{  
    if (m_socket.is_open()) {
        boost::system::error_code ec;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        m_socket.close(ec);
    }
}


/**
 *
 */
bool DGGATcpConnection::connect(const std::string& host, uint16_t port)
{
    boost::asio::ip::tcp::resolver r(m_socket.get_io_service());
    std::string str_port = boost::lexical_cast<std::string>(port);

    boost::asio::ip::tcp::resolver::query query(host, str_port);
    boost::asio::ip::tcp::resolver::iterator ret, end;

    try {
        ret = boost::asio::connect(m_socket, r.resolve(query));
        if (ret != end)
            return true;
    } catch (...) {
        return false;
    }

    return false;
}


/**
 *
 */
bool DGGATcpConnection::connect(const std::vector<std::string>& hosts,
                                uint16_t port)
{
    if (hosts.empty())
        throw std::invalid_argument("[DGGATcpConnection::connect] Empty list"
                                    " of hosts.");

    bool connected = false;
    std::vector<std::string>::const_iterator it;
    for (it = hosts.begin(); !connected && it != hosts.end(); ++it)
        connected = connect(*it, port);

    return connected;
}


/**
 *
 */
void DGGATcpConnection::sendMessage(const std::string& msg)
{
    boost::unique_lock<boost::recursive_mutex> lock(m_write_mutex);

    if (!msg.empty()) {
        m_temp_out_buffer.append(msg);
        m_temp_out_buffer.append(m_end_of_message_mark);

        if (!m_write_in_progress) {
            m_temp_out_buffer.swap(m_out_buffer);
            m_temp_out_buffer.clear();
            startWrite();
        }
    }
}


/**
 *
 */
void DGGATcpConnection::startRead()
{
    // Calling this twice will override the previous handler, which is the same
    // so it is fine to do it ;)
    m_socket.async_read_some(boost::asio::buffer(m_temp_in_buffer),
        boost::bind(&DGGATcpConnection::handleReadSome,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    m_read_in_progress = true;
}


/**
 *
 */
void DGGATcpConnection::startWrite()
{
    boost::unique_lock<boost::recursive_mutex> lock(m_write_mutex);

    m_socket.async_write_some(boost::asio::buffer(m_out_buffer),
        boost::bind(&DGGATcpConnection::handleWriteSome,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    m_write_in_progress = true;
}


//==============================================================================
// DGGATcpConnection private methods

/**
 *
 */
DGGATcpConnection::DGGATcpConnection(boost::asio::io_service& io_service)
    : m_socket(io_service)

    , m_disconnection_handler()
    , m_new_message_handler()

    , m_write_mutex()
    , m_disconnection_handler_mutex()
    , m_new_message_handler_mutex()

    , m_in_buffer()
    , m_out_buffer()
    , m_temp_in_buffer()
    , m_temp_out_buffer()
    , m_read_in_progress(false)
    , m_write_in_progress(false)
    , m_end_of_message_mark(DEF_EOM)
{ }


/**
 *
 */
void DGGATcpConnection::handleReadSome(const boost::system::error_code& error,
                                      size_t bytes)
{
    if (!error) {
        assert(bytes % sizeof(std::string::value_type) == 0);

        size_t ncharacters = bytes / sizeof(std::string::value_type);
        m_in_buffer.append(m_temp_in_buffer.data(), ncharacters);

        processReadData();

        m_read_in_progress = false;
        startRead();

    } else if (error == boost::asio::error::eof
               || error == boost::asio::error::connection_reset) 
    {
        boost::unique_lock<boost::recursive_mutex> lock(
            m_disconnection_handler_mutex);

        close();        
        if (!m_disconnection_handler.empty())
            m_disconnection_handler(shared_from_this());

    } else {
        // Exception
    }
}


/**
 *
 */
void DGGATcpConnection::handleWriteSome(const boost::system::error_code& error,
                                       size_t bytes)
{
    boost::unique_lock<boost::recursive_mutex> lock(m_write_mutex);

    if (!error) {
        if (!m_out_buffer.empty()) {
            assert(bytes % sizeof(std::string::value_type) == 0);

            size_t ncharacters = bytes / sizeof(std::string::value_type);
            m_out_buffer.erase(0, ncharacters);        
        }
        
        if (!m_temp_out_buffer.empty()) {
            m_out_buffer.append(m_temp_out_buffer);
            m_temp_out_buffer.clear();
        }

        m_write_in_progress = false;
        if (!m_out_buffer.empty()) {
            startWrite();
        }

    } else {
        // Exception
    }
}


/**
 *
 */
void DGGATcpConnection::processReadData()
{
    size_t msg_end_pos;
    for (msg_end_pos = m_in_buffer.find(m_end_of_message_mark);
         msg_end_pos != std::string::npos;
         msg_end_pos = m_in_buffer.find(m_end_of_message_mark))
    {
        boost::unique_lock<boost::recursive_mutex> lock(
            m_new_message_handler_mutex);

        std::string msg = m_in_buffer.substr(0, msg_end_pos);
        m_in_buffer.erase(0, msg_end_pos + 1);

        boost::trim(msg);
        if (!m_new_message_handler.empty())
            m_new_message_handler(shared_from_this(), msg);
    }
}
