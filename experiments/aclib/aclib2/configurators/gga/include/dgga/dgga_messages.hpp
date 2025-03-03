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

#ifndef _DGGA_MESSAGES_HPP_
#define _DGGA_MESSAGES_HPP_

#include <string>

// communication constants
//
extern const std::string DGGA_MSG_ACK;
extern const std::string DGGA_MSG_POLL;
extern const std::string DGGA_MSG_HELLO;
extern const std::string DGGA_MSG_READY;
extern const std::string DGGA_MSG_WORKING;

extern const std::string DGGA_MSG_RESULTS_BEG;
extern const std::string DGGA_MSG_RESULTS_END;
extern const std::string DGGA_MSG_TOURNEY_BEG;
extern const std::string DGGA_MSG_TOURNEY_END;

extern const std::string DGGA_MSG_GET_CONFIGURATION;
extern const std::string DGGA_MSG_CONFIGURATION_BEG;
extern const std::string DGGA_MSG_CONFIGURATION_END;

extern const std::string DGGA_MSG_RESET_TIMEOUT;

// boost serialization nvp names
extern const char* DGGA_EVALS_TAG;
extern const char* DGGA_WINNERS_TAG;
extern const char* DGGA_TIMEOUT_TAG;
extern const char* DGGA_INSTANCES_TAG;
extern const char* DGGA_PARTICIPANTS_TAG;
extern const char* DGGA_SELECT_RESULT_TAG;
extern const char* DGGA_INSTANCES_STATISTICS_TAG;


#endif // _DGGA_MESSAGES_HPP_