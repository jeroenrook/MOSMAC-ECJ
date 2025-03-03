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
#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <boost/algorithm/string/trim.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp> // _1, _2, ..., _9
#include <boost/serialization/vector.hpp>

#include "GGAOptions.hpp"
#include "GGAUtil.hpp"
#include "OutputLog.hpp"

#include "dgga/dgga_messages.hpp"
#include "dgga/DGGARemoteSelectorMaster.hpp"

//==============================================================================
// DGGARemoteSelectorMaster public methods

/**
 *
 */
DGGARemoteSelectorMaster::DGGARemoteSelectorMaster(
                                        boost::asio::io_service& io_service)
    : nodes_(GGAOptions::instance().num_nodes)
    , cores_(GGAOptions::instance().num_threads)
    , startWorkerCmd_()
    , requestedNodes_(0)
    , avaliableNodes_(0)

    , participants_()
    , instances_()

    , resultBuilder_()
    
    , tourneySizes_()
    , nparticipantsCompeting_(0)
    , propagateTimeout_(GGAOptions::instance().propagate_timeout)
    , participantsInWorker_()

    , connections_()
    , connectionIdle_()
    , connectionAddr_()
    , connectionResults_()

    , ioService(io_service)
    , acceptor_(ioService, GGAOptions::instance().port)
{ 
    acceptor_.setNewConnectionHandler(
        boost::bind(&DGGARemoteSelectorMaster::handleNewConnection, this, _1));
}

DGGARemoteSelectorMaster::~DGGARemoteSelectorMaster()
{ }

/**
 *
 */
GGASelectorResult DGGARemoteSelectorMaster::select(const GGAGenomeVector& participants,
                                                   const GGAInstanceVector& instances,
                                                   double timeout)
{
    
    resultBuilder_.clear();

    if (!participants.empty() && !instances.empty()) {
        participants_ = participants;
        instances_ = instances;
        tourneySizes_ = balanceTournaments(participants.size(), cores_);
        nparticipantsCompeting_ = 0;
        timeout_ = timeout;

        LOG_VERBOSE("Tournament sizes for this generation: [" 
                << OutputLog::uintVectorToString(tourneySizes_) << "]");

        startWorkers(); // Tries to wake up some workers
        pollWorkers();

        while (participants_.size() > 0 || nparticipantsCompeting_ > 0)
            ioService.run_one();
    }

    return resultBuilder_.build();
}


//==============================================================================
// DGGARemoteSelectorMaster private methods

/**
 * Disabled for testing. (start workers manually)
 */
void DGGARemoteSelectorMaster::startWorkers()
{
    while (requestedNodes_ + avaliableNodes_ < nodes_) {
        int ret = std::system(startWorkerCmd_.c_str());  // fork + execvp?
        if (ret == 0)                                       // boost::process?
            requestedNodes_ += 1;
        else
            throw std::runtime_error(std::strerror(errno));
    }
}


/**
 *
 */
void DGGARemoteSelectorMaster::pollWorkers()
{
    std::set<DGGATcpConnection::pointer>::iterator it;
    for (it = connections_.begin(); it != connections_.end(); ++it)
        if (connectionIdle_[*it])
            (*it)->sendMessage(DGGA_MSG_POLL);
}


/**
 *
 */
void DGGARemoteSelectorMaster::handleNewConnection(DGGATcpConnection::pointer con)
{
    con->setDisconnectionHandler(boost::bind(
        &DGGARemoteSelectorMaster::handleDisconnection, this, _1));
    con->setNewMessageHandler(boost::bind(
        &DGGARemoteSelectorMaster::handleInitState, this, _1, _2));
    con->startRead();

    // Initialize connection data
    connections_.insert(con);
    connectionIdle_[con] = false;
    try {  // TODO: Check if it is necessary
        connectionAddr_[con] = con->socket().remote_endpoint().address().to_string();
    } catch (...) { 
        connectionAddr_[con] = "addr_here";
    }
    connectionResults_[con].clear();
}


/**
 *
 */
void DGGARemoteSelectorMaster::handleDisconnection(
                                            DGGATcpConnection::pointer con)
{
    LOG("Disconnected (" << mapAt(connectionAddr_, con) << ")");

    // Rollback any unfinished tourney associated with con
    rollbackTourney(con);

    // Clean up data
    participantsInWorker_.erase(con);

    connections_.erase(con);
    connectionIdle_.erase(con);
    connectionResults_.erase(con);

    con->close();

    avaliableNodes_ -= 1;
    
    if (participants_.size() > 0 || nparticipantsCompeting_ > 0) {
        startWorkers();
        pollWorkers();
    }
}


/**
 *
 */
void DGGARemoteSelectorMaster::handleInitState(DGGATcpConnection::pointer con,
                                               const std::string& msg)
{
    if (DGGA_MSG_HELLO == msg) {
        LOG("Validation (" << mapAt(connectionAddr_, con) << ")    [OK]");
        con->sendMessage(DGGA_MSG_ACK);
        con->setNewMessageHandler(boost::bind(
            &DGGARemoteSelectorMaster::handleGetConfig, this, _1, _2));
    } else {
        LOG_ERROR("[DGGARemoteSelectorMaster::handleInitState] Unexpected"
                  " message, communication sequence broken ... Disconnecting");
        con->close();
    }
}


/**
 *
 */
void DGGARemoteSelectorMaster::handleGetConfig(DGGATcpConnection::pointer con,
                                               const std::string& msg)
{
    if (DGGA_MSG_GET_CONFIGURATION == msg) {
        std::stringstream ss;
        boost::archive::text_oarchive oa(ss);

        oa << GGAOptions::instance();
        oa << GGAParameterTree::instance();

        con->sendMessage(DGGA_MSG_CONFIGURATION_BEG);
        con->sendMessage(ss.str());
        con->sendMessage(DGGA_MSG_CONFIGURATION_END);

        con->setNewMessageHandler(boost::bind(
            &DGGARemoteSelectorMaster::handleConfigAck, this, _1, _2));
    } else {
        LOG_ERROR("[DGGARemoteSelectorMaster::handleGetConfig] Unexpected"
                  " message, communication sequence broken ... Disconnecting");
        con->close();
    }
}


/**
 *
 */
void DGGARemoteSelectorMaster::handleConfigAck(DGGATcpConnection::pointer con,
                                               const std::string& msg)
{
    if (DGGA_MSG_ACK == msg) {
        con->setNewMessageHandler(boost::bind(
            &DGGARemoteSelectorMaster::handleIDLEState, this, _1, _2));
        connectionIdle_[con] = true;

        requestedNodes_ -= 1;
        avaliableNodes_ += 1;

        con->sendMessage(DGGA_MSG_POLL);
    } else {
        LOG_ERROR("[DGGARemoteSelectorMaster::handleConfigAck] Unexpected"
                  " message, communication sequence broken ... Disconnecting");
        con->close();
    }
}


/**
 *
 */
void DGGARemoteSelectorMaster::handleIDLEState(DGGATcpConnection::pointer con,
                                               const std::string& msg)
{
    if (DGGA_MSG_READY == msg) {
        if (sendTourney(con)) {
            con->setNewMessageHandler(boost::bind(
                &DGGARemoteSelectorMaster::handleWorkingState, this, _1, _2));
            connectionIdle_[con] = false;
        }
    } else {
        LOG_ERROR("IDLE unexpected (" << mapAt(connectionAddr_, con) << "): " << msg);
        LOG_ERROR("----  Received message ----\n" << msg);
    }
}


/**
 *
 */
void DGGARemoteSelectorMaster::handleWorkingState(
                                                DGGATcpConnection::pointer con,
                                                const std::string& msg)
{
    if (DGGA_MSG_RESULTS_BEG == msg) {
        con->setNewMessageHandler(boost::bind(
            &DGGARemoteSelectorMaster::handleResultsState, this, _1, _2));
    } else if (DGGA_MSG_WORKING == msg) {
        LOG_VERBOSE("Polled worker is working!");
    } else {
        LOG_ERROR("Working unexpected (" << mapAt(connectionAddr_, con) << ")");
        LOG_ERROR("----  Received message ----\n" << msg);
    }
}


/**
 *
 */
void DGGARemoteSelectorMaster::handleResultsState(DGGATcpConnection::pointer con,
                                                  const std::string& msg)
{
    if (DGGA_MSG_RESULTS_END == msg) {
       LOG("Recovering tournament results (" << mapAt(connectionAddr_, con) << ")");
       recoverResults(connectionResults_[con]);

        GGAGenomeVector& con_participants = participantsInWorker_[con];
        nparticipantsCompeting_ -= con_participants.size();
        con_participants.clear();

        // Clear buffer (forcing memory release)
        std::string().swap(connectionResults_[con]);

        con->setNewMessageHandler(boost::bind(
            &DGGARemoteSelectorMaster::handleIDLEState, this, _1, _2));
        connectionIdle_[con] = true;

        // Propagate timeout updated in recoverResults(...)
        if (propagateTimeout_)
            resetTimeout();
        
        // If there are remaining participants, poll the worker again.
        if (!participants_.empty())
            con->sendMessage(DGGA_MSG_POLL);
    } else {
        // Initializes results buffer if necessary and appends new data
        connectionResults_[con].append(msg);
    }
}


/**
 * Sends tourney instances and genomes.
 */
bool DGGARemoteSelectorMaster::sendTourney(DGGATcpConnection::pointer con)
{
    assert(participantsInWorker_[con].empty());
    assert(tourneySizes_.empty() == participants_.empty());

    if (participants_.empty())
        return false;

    // Start from the back because this way it is more efficient.
    assert(tourneySizes_.back() > 0);
    size_t start = participants_.size() - tourneySizes_.back(); 
    
    GGAGenomeVector& wparticipants = participantsInWorker_[con];
    wparticipants.assign(participants_.begin() + start, participants_.end());
    participants_.erase(participants_.begin() + start, participants_.end());

    nparticipantsCompeting_ += tourneySizes_.back();
    tourneySizes_.pop_back();

    // MSG_TOURNEY_BEG
    // #instances instances #genomes genomes 1..m
    // MSG_TOURNEY_END
    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    
    oa << boost::serialization::make_nvp(DGGA_TIMEOUT_TAG, timeout_);
    oa << boost::serialization::make_nvp(DGGA_INSTANCES_TAG, instances_);
    oa << boost::serialization::make_nvp(DGGA_PARTICIPANTS_TAG, wparticipants);

    con->sendMessage(DGGA_MSG_TOURNEY_BEG);
    con->sendMessage(ss.str());
    con->sendMessage(DGGA_MSG_TOURNEY_END);
    LOG("Tournament sent (" << mapAt(connectionAddr_, con) << ")");

    return true;
}


/**
 *
 */
void DGGARemoteSelectorMaster::rollbackTourney(DGGATcpConnection::pointer con)
{
    GGAGenomeVector& wparticipants = participantsInWorker_[con];
    if (wparticipants.empty())
        return;

    LOG("Rolling back a tournament (" << mapAt(connectionAddr_, con) << ")");

    nparticipantsCompeting_ -= wparticipants.size();
    tourneySizes_.push_back(wparticipants.size());
    participants_.insert(participants_.end(), wparticipants.begin(),
                                                wparticipants.end());

    wparticipants.clear();
}


/**
 *
 */
void DGGARemoteSelectorMaster::recoverResults(const std::string& results)
{
    std::stringstream ss(results);
    boost::archive::text_iarchive ia(ss);

    GGASelectorResult result;
    
    ia >> boost::serialization::make_nvp(DGGA_SELECT_RESULT_TAG, result);    
    resultBuilder_.addAll(result);

    LOG("Number of evaluations in this generation so far: " << resultBuilder_.getNumEvaluations());

    const GGAGenomeVector& twinners = result.getWinners();
    if (propagateTimeout_) {
        for (unsigned i = 0; i < twinners.size(); ++i)
            timeout_ = std::min(timeout_, twinners[i].objValue());
    }
}

/**
 *
 */
void DGGARemoteSelectorMaster::resetTimeout()
{
    std::string msg(DGGA_MSG_RESET_TIMEOUT);
    {
        std::stringstream ss;
        boost::archive::text_oarchive oa(ss);
        oa << boost::serialization::make_nvp(DGGA_TIMEOUT_TAG, timeout_);
        
        msg += " ";
        msg += ss.str();
    }

    std::set<DGGATcpConnection::pointer>::iterator it;
    for (it = connections_.begin(); it != connections_.end(); ++it)
        if (!connectionIdle_[*it])
            (*it)->sendMessage(msg);
}
