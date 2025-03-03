//
// This file is part of DGGA.
// 
// The MIT License (MIT)
// 
// Copyright (c) 2015 Kevin Tierney and Josep Pon Farreny
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>

#include <stdexcept>
#include <string>

#include <boost/filesystem.hpp>

#include "dgga/dgga.hpp"
#include "gga.hpp"
#include "GGAExceptions.hpp"
#include "GGAOptions.hpp"
#include "GGAParameterTree.hpp"

// Function prototypes
//
void installSignalHandlers();
bool parseOptions(int argc, char** argv);
void printHelp();
void handleSignal(int signal);
void atExitHandler();


// Program entry point.
//
int main(int argc, char** argv) {
    OutputLog::setMasterPid();
    
    ::atexit(atExitHandler);
    installSignalHandlers();

    int retCode = 0; // >= 2: display help
    try {   
        const GGAOptions& opts = GGAOptions::instance();

        if (parseOptions(argc, argv)) {
            
            if (opts.master || opts.worker) 
                retCode = runDGGA();
            else
                retCode = runGGA();
        }

    } catch (GGAXMLParseException& e) {
        LOG_ERROR("[GGAXMLParseException] Error: " << e.what());
        retCode = 1;
    } catch (GGAMalformedInstanceException& e) {
        LOG_ERROR("[GGAMalformedInstanceException] Error: " << e.what());
        retCode = 2;
    } catch (GGAFileNotFoundException& e) {
        LOG_ERROR("[GGAFileNotFoundException] Error: " << e.what());
        retCode = 3;
    } catch (GGAOptionsException& e) {
        LOG_ERROR("[GGAOptionsException] Error: " << e.what());
        printHelp();
        retCode = 4;
    } catch (GGAParameterException& e) {
        LOG_ERROR("[GGAParameterException] Error: " << e.what());
        retCode = 5;
    } catch (GGAPopulationException& e) {
        LOG_ERROR("[GGAPopulationException] Error: " << e.what());
        retCode = 6;
    } catch (GGAScenarioFileException& e) {
        LOG_ERROR("[GGAScenarioFileException] Error: " << e.what());
        retCode = 7;
    }
    /*} catch(std::exception& e) {
        LOG_ERROR(e.what());
        retCode = 2;*/
    /*} catch(...) {
        LOG_ERROR("Unknown exception caught! Let the maintainer of GGA know what was going on when this happened!");
        retCode = 3;
    }*/

    return retCode;
}


/* =========== */


void installSignalHandlers()
{   // Using specific POSIX sigaction call for obvious reasons ;)
    struct sigaction saction;
    memset(&saction, 0, sizeof(struct sigaction));
    saction.sa_handler = handleSignal;
    sigfillset(&saction.sa_mask);   // Block all signals while on signal handler

    if (::sigaction(SIGTERM, &saction, NULL) != 0)
        std::runtime_error(
            std::string("Error installing SIGTERM: ") += std::strerror(errno));

    if (::sigaction(SIGINT, &saction, NULL) != 0)
        std::runtime_error(
            std::string("Error installing SIGINT: ") += std::strerror(errno));

    if (::sigaction(SIGQUIT, &saction, NULL) != 0)
        std::runtime_error(
            std::string("Error installing SIGQUIT: ") += std::strerror(errno));

    if (::sigaction(SIGTSTP, &saction, NULL) != 0)
        std::runtime_error(
            std::string("Error installing SIGTSTP: ") += std::strerror(errno));
}


bool parseOptions(int argc, char** argv)
{
    GGAOptions& opts = GGAOptions::mutableInstance();
    
    if (opts.parseHelpOptions(argc, argv)) {
        printHelp();
        return false;
    }

    opts.parseRemoteOptions(argc, argv);

    if (!opts.worker) {
        opts.parse(argc, argv);
        opts.loadScenarioFile();
    }

    return true;
}



void printHelp() 
{
    const GGAOptions& opts = GGAOptions::instance();

    LOG_ERROR_NOP("GGA Version: 1.3.2");
    LOG_ERROR_NOP(opts.getHelpMessage());
}   

/* === Signal handlers === */

void handleSignal(int signal)
{
    if(getpid() == OutputLog::masterPid()) {
        kill(0, signal);    // Propagate signal to process group.
        LOG_ERROR("Parent: received signal " << signal << "! Exiting.");
    } else {
        LOG_ERROR("Child: received signal " << signal << "! Exiting.");
    }

    exit(0); // Calls atExitHandler
}

/* === Exit handlers === */

void atExitHandler(void)
{   // If an exception can be thrown while exiting, there are two options to
    // make the exit secure:
    //    - Handle the exception in a try catch block
    //    - Set a terminate handler to deal with this situations
    if (getpid() == OutputLog::masterPid()) {
        // Clean up memory
        GGAParameterTree::deleteInstance();
        GGAOptions::deleteInstance();
        //
    }
}
