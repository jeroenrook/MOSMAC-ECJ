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

#include "dgga/dgga_messages.hpp"

// communication constants
//
const std::string DGGA_MSG_ACK               ("DGGA_ACK");
const std::string DGGA_MSG_POLL              ("DGGA_POLL");
const std::string DGGA_MSG_HELLO             ("DGGA_HELLO");
const std::string DGGA_MSG_READY             ("DGGA_READY");
const std::string DGGA_MSG_WORKING           ("DGGA_WORKING");

const std::string DGGA_MSG_RESULTS_BEG       ("DGGA_RESULTS_BEG");
const std::string DGGA_MSG_RESULTS_END       ("DGGA_RESULTS_END");
const std::string DGGA_MSG_TOURNEY_BEG       ("DGGA_TOURNEY_BEG");
const std::string DGGA_MSG_TOURNEY_END       ("DGGA_TOURNEY_END");

const std::string DGGA_MSG_GET_CONFIGURATION ("DGGA_GET_CONFIGURATION");
const std::string DGGA_MSG_CONFIGURATION_BEG ("DGGA_CONFIGURATION_BEG");
const std::string DGGA_MSG_CONFIGURATION_END ("DGGA_CONFIGURATION_END");

const std::string DGGA_MSG_RESET_TIMEOUT     ("DGGA_RESET_TIMEOUT");

// boost serialization nvp names
const char* DGGA_EVALS_TAG                ("dgga_evals");
const char* DGGA_WINNERS_TAG              ("dgga_winners");
const char* DGGA_TIMEOUT_TAG              ("dgga_timeout");
const char* DGGA_INSTANCES_TAG            ("dgga_instances");
const char* DGGA_PARTICIPANTS_TAG         ("dgga_participants");
const char* DGGA_SELECT_RESULT_TAG        ("dgga_select_result");
const char* DGGA_INSTANCES_STATISTICS_TAG ("dgga_instances_statistics");
