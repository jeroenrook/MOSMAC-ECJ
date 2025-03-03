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

#include <sstream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/vector.hpp>

#include "GGAExceptions.hpp"
#include "GGAOptions.hpp"
#include "GGAParameterTree.hpp"
#include "GGAUtil.hpp"
#include "OutputLog.hpp"

#include "dgga/dgga_messages.hpp"
#include "dgga/DGGARemoteSelectorWorker.hpp"


//==============================================================================
// DGGARemoteSelectorWorker public methods

/**
 *
 */
DGGARemoteSelectorWorker::DGGARemoteSelectorWorker(
                                        boost::asio::io_service& io_service)
    : m_selector()
    , m_recv_data_buffer()

    , m_tourney_thread()

    , m_io_service(io_service)
    , m_connection(DGGATcpConnection::create(m_io_service))

{
    m_connection->setDisconnectionHandler(
        boost::bind(&DGGARemoteSelectorWorker::handleDisconnection, this, _1));
}


/**
 *
 */
DGGARemoteSelectorWorker::~DGGARemoteSelectorWorker()
{ }


/**
 *
 */
bool DGGARemoteSelectorWorker::connect(const StringVector& ips, uint16_t port)
{
    return m_connection->connect(ips, port);
}


/**
 *
 */
void DGGARemoteSelectorWorker::run()
{
    if (!m_connection->socket().is_open())
        throw std::logic_error("[DGGARemoteSelectorWorker::run] Not connected"
                               " to a master selector.");

    m_connection->setNewMessageHandler(
        boost::bind(&DGGARemoteSelectorWorker::handleHelloAck, this, _1, _2));
    m_connection->startRead();

    m_connection->sendMessage(DGGA_MSG_HELLO);

    m_io_service.run(); // Handlers take control of the execution
}

//==============================================================================
// DGGARemoteSelectorWorker private methods


/**
 *
 */
void DGGARemoteSelectorWorker::handleDisconnection(
                                            DGGATcpConnection::pointer con)
{
    m_io_service.stop();

    if(m_selector.get() != NULL) {
        m_selector->forceStop();        
        m_tourney_thread.join();
    }

}


/**
 *
 */
void DGGARemoteSelectorWorker::handleHelloAck(DGGATcpConnection::pointer con,
                                              const std::string& msg)
{
    if (DGGA_MSG_ACK == msg) {
        con->setNewMessageHandler(boost::bind(
            &DGGARemoteSelectorWorker::handleBegConfig, this, _1, _2));

        con->sendMessage(DGGA_MSG_GET_CONFIGURATION);
    } else {
        LOG_ERROR("[DGGARemoteSelectorWorker::handleHelloAck] Unexpected"
                  " message, communication sequence broken ... Disconnecting");
        con->close();
    }
}


/**
 *
 */
void DGGARemoteSelectorWorker::handleBegConfig(DGGATcpConnection::pointer con,
                                               const std::string& msg)
{
    if (DGGA_MSG_CONFIGURATION_BEG == msg) {
        con->setNewMessageHandler(boost::bind(
            &DGGARemoteSelectorWorker::handleGetConfig, this, _1, _2));

        m_recv_data_buffer.clear();
    } else {
        LOG_ERROR("[DGGARemoteSelectorWorker::handleBegConfig] Unexpected"
                  " message, communication sequence broken ... Disconnecting");
        con->close();
    }
}


/**
 *
 */
void DGGARemoteSelectorWorker::handleGetConfig(DGGATcpConnection::pointer con,
                                               const std::string& msg)
{
    if (DGGA_MSG_CONFIGURATION_END == msg) {
        assert(!m_recv_data_buffer.empty());

        std::stringstream ss(m_recv_data_buffer);
        boost::archive::text_iarchive ia(ss);

        ia >> GGAOptions::mutableInstance();
        ia >> GGAParameterTree::mutableInstance();

        m_selector.reset(new GGALocalSelector());

        printOptions();
        LOG("Parameter Tree:");
        LOG(GGAParameterTree::instance().toString());

        con->setNewMessageHandler(boost::bind(
            &DGGARemoteSelectorWorker::handleIDLEState, this, _1, _2));

        con->sendMessage(DGGA_MSG_ACK);
    } else {
        m_recv_data_buffer.append(msg);
    }
}


/**
 *
 */
void DGGARemoteSelectorWorker::handleIDLEState(DGGATcpConnection::pointer con,
                                              const std::string& msg)
{
    if (msg == DGGA_MSG_TOURNEY_BEG) {
        con->setNewMessageHandler(boost::bind(
            &DGGARemoteSelectorWorker::handleRecvTourneyState, this, _1, _2));

        // Clear buffer
        m_recv_data_buffer.clear();
    } else if (msg == DGGA_MSG_POLL) {
        m_connection->sendMessage(DGGA_MSG_READY);
    } else {
        LOG_ERROR("IDLE Unknown message: " << msg);
    }
}


/**
 *
 */
void DGGARemoteSelectorWorker::handleRecvTourneyState(
                                                DGGATcpConnection::pointer con,
                                                const std::string& msg)
{
    if (msg == DGGA_MSG_TOURNEY_END) {
        con->setNewMessageHandler(boost::bind(
            &DGGARemoteSelectorWorker::handleWorkingState, this, _1, _2));

        m_tourney_thread = boost::thread(boost::bind(
            &DGGARemoteSelectorWorker::executeTourney, this, con));
    } else if (isResetTimeout(msg)) {
        resetTimeout(msg);
    } else {
        m_recv_data_buffer.append(msg);
    }
}


/**
 *
 */
void DGGARemoteSelectorWorker::handleWorkingState(DGGATcpConnection::pointer con,
                                                 const std::string& msg)
{
    if (msg == DGGA_MSG_POLL) {
        con->sendMessage(DGGA_MSG_WORKING);
    } else if (isResetTimeout(msg)) {
        resetTimeout(msg);
    } else {
        LOG_ERROR("WORKING Unknown message: " << msg);
    }
}


/**
 *
 */
void DGGARemoteSelectorWorker::executeTourney(DGGATcpConnection::pointer con)
{
    double timeout;
    GGAInstanceVector instances;
    GGAGenomeVector genomes;

    { // scope begin
        std::stringstream ss(m_recv_data_buffer);
        
        boost::archive::text_iarchive ia(ss);

        ia >> boost::serialization::make_nvp(DGGA_TIMEOUT_TAG, timeout);
        ia >> boost::serialization::make_nvp(DGGA_INSTANCES_TAG, instances);
        ia >> boost::serialization::make_nvp(DGGA_PARTICIPANTS_TAG, genomes);
    } // scope end: automatically destroys ss and ia objects

    LOG("#INSTANCES: " << instances.size());
    LOG("#PARTICIPANTS: " << genomes.size());

    try {
        GGASelectorResult result = m_selector->select(genomes, instances, timeout);
        
        { // scope begin
            std::stringstream ss;
            boost::archive::text_oarchive oa(ss);
            oa << boost::serialization::make_nvp(DGGA_SELECT_RESULT_TAG, result);

            con->sendMessage(DGGA_MSG_RESULTS_BEG);
            con->sendMessage(ss.str());
            con->sendMessage(DGGA_MSG_RESULTS_END);
        } // scope end    

        //con->sendMessage(DGGA_MSG_READY);

        con->setNewMessageHandler(boost::bind(
                &DGGARemoteSelectorWorker::handleIDLEState, this, _1, _2));

    } catch (GGAInterruptedException &e) {
        // Ignore (only triggered by an abrupt disconnection)
    }
}

/**
 *
 */
bool DGGARemoteSelectorWorker::isResetTimeout(const std::string& msg) const
{
    return boost::starts_with(msg, DGGA_MSG_RESET_TIMEOUT);
}

/**
 *
 */
void DGGARemoteSelectorWorker::resetTimeout(const std::string& msg)
{
    double new_timeout;

    size_t pos = msg.find_first_of(" ");
    std::string str_timeout(msg.substr(pos));
    boost::trim(str_timeout);
    assert(!str_timeout.empty());

    {
        std::stringstream ss(str_timeout);
        boost::archive::text_iarchive ia(ss);
        ia >> boost::serialization::make_nvp(DGGA_TIMEOUT_TAG, new_timeout);
    }

    LOG_VERBOSE("Reset timeout: " << new_timeout);
    m_selector->resetTimeout(new_timeout);
}