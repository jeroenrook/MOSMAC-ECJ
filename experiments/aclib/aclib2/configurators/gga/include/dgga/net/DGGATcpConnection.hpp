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

#ifndef _GGA_TCP_CONNECTION_HPP_
#define _GGA_TCP_CONNECTION_HPP_

#include <sstream>
#include <string>
#include <vector>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>

/**
 * Wraps a tcp connection and provides methods and handlers to interact with
 * the communication channel. (NOT THREAD SAFE)
 */
class DGGATcpConnection
    : public boost::enable_shared_from_this<DGGATcpConnection>
{
public:
    typedef boost::shared_ptr<DGGATcpConnection> pointer;
    typedef boost::function1<void, pointer> DisconnectionHandler;
    typedef boost::function2<void, pointer, const std::string&> 
                                                            NewMessageHandler;

    static const std::string DEF_EOM;

    //
    static pointer create(boost::asio::io_service&);

    // Destruct
    virtual ~DGGATcpConnection();

    // close/connect
    void close();
    bool connect(const std::string& host, uint16_t port);
    bool connect(const std::vector<std::string>& hosts, uint16_t port);

    void sendMessage(const std::string&);

    void startRead();
    void startWrite();    

    // access internal socket
    const boost::asio::ip::tcp::socket& socket() const;
    boost::asio::ip::tcp::socket& socket();

    void setDisconnectionHandler(DisconnectionHandler);
    void setNewMessageHandler(NewMessageHandler);
private:
    // Intentionally unimplemented
    DGGATcpConnection();
    DGGATcpConnection(const DGGATcpConnection&);
    DGGATcpConnection& operator=(const DGGATcpConnection);

    // Private constructor
    DGGATcpConnection(boost::asio::io_service&);

    // Handlers
    void handleReadSome(const boost::system::error_code&, size_t);
    void handleWriteSome(const boost::system::error_code&, size_t);

    void processReadData();

    //
    boost::asio::ip::tcp::socket m_socket;

    DisconnectionHandler m_disconnection_handler;
    NewMessageHandler m_new_message_handler;

    boost::recursive_mutex m_write_mutex;
    boost::recursive_mutex m_disconnection_handler_mutex;
    boost::recursive_mutex m_new_message_handler_mutex;

    std::string m_in_buffer;
    std::string m_out_buffer;

    boost::array<char, 1024> m_temp_in_buffer;
    std::string m_temp_out_buffer;

    bool m_read_in_progress;
    bool m_write_in_progress;

    std::string m_end_of_message_mark;
};

//==============================================================================
// DGGATcpConnection public inline methods

/**
 *
 */
inline const boost::asio::ip::tcp::socket& DGGATcpConnection::socket() const
{ return m_socket; }

/**
 *
 */
inline boost::asio::ip::tcp::socket& DGGATcpConnection::socket()
{ 
    return const_cast<boost::asio::ip::tcp::socket&>(
        static_cast<DGGATcpConnection*>(this)->m_socket); 
}

/**
 *
 */
inline void DGGATcpConnection::setDisconnectionHandler(
                                                DisconnectionHandler handler)
{
    boost::unique_lock<boost::recursive_mutex> lock(
            m_disconnection_handler_mutex);

    m_disconnection_handler = handler;
}


/**
 *
 */
inline void DGGATcpConnection::setNewMessageHandler(NewMessageHandler handler)
{
    boost::unique_lock<boost::recursive_mutex> lock(
        m_new_message_handler_mutex);

    m_new_message_handler = handler;
}


#endif // _GGA_TCP_CONNECTION_HPP_