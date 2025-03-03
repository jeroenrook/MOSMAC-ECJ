//
// This file is part of DGGA.
// 
// The MIT License (MIT)
// 
// Copyright (c) 2015 Josep Pon Farreny
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


#include <iostream>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <boost/timer/timer.hpp>

#include "GGAOSProber.hpp"
#include "GGASystem.hpp"

#if defined(OS_LINUX)
# include <unistd.h>
#elif defined(OS_WINDOWS)
# include <windows.h>
#elif defined(OS_OSX)
# include <mach-o/dyld.h>
#elif defined(OS_BSD)
# include <sys/types.h>
# include <sys/sysctl.h>
#endif

#if defined(OS_LINUX) || defined(OS_OSX) || defined(OS_BSD)
# include <arpa/inet.h>
# include <net/if.h>
# include <sys/types.h>
# include <ifaddrs.h>
#endif

// Variables local to this file
//
static boost::timer::cpu_timer cpu_timer; ///< Constructor starts timers


/**
 * @return Wall clock time in seconds.
 */
double wallClockTime()
{
    return static_cast<double>(cpu_timer.elapsed().wall) / 1e9;
}


/**
 * @return User time of the current process plus the user time of all its 
 *         children.
 */
double userTime()
{   // Precision varies as with getrusage
    return static_cast<double>(cpu_timer.elapsed().user) / 1e9;
}

/**
 * @return System time of the current process plus the system time of all its
 *         children.
 */
double systemTime()
{   // Precision varies as with getrusage
    return static_cast<double>(cpu_timer.elapsed().system) / 1e9;
}


/**
 * @brief Suspends the execution of the calling thread until the specified time
 *        has elapsed. If a signal resumes the process, it is suspended again
 *        until the process has slept for the specified amount of time.
 *
 * @throw std::runtime_error If any of the system calls fails.
 */
void sleepFor(unsigned seconds, unsigned nanoseconds)
{
    static const unsigned MAX_NANOSEC = 999999999;
    seconds += nanoseconds / MAX_NANOSEC;
    nanoseconds %= MAX_NANOSEC;

#if defined(OS_LINUX) || defined (OS_OSX) || defined (OS_BSD)
    timespec req, rem;
    req.tv_sec = static_cast<time_t>(seconds);
    req.tv_nsec = static_cast<long>(nanoseconds);

    int ret = ::nanosleep(&req, &rem);
    while (ret == -1) {
        if (errno == EFAULT)
            throw std::runtime_error("[EFAULT] (nanosleep) Probem with copying"
                                     " information from user space");
        req = rem;
        ret = ::nanosleep(&req, &rem);                
    }
#else
# error Missing sleepFor implementation for you system.
#endif
}


/**
 * @brief Gets the absolute path to the executable file of the running
 *        program.
 * @return A std::string with the absolute path of the running program's
 *         exe file.
 */
std::string getExecutablePath()
{
    char *cstr_path = NULL;
    size_t size = 256;      // Base size

    bool done = false;
    do {
        delete[] cstr_path;
        cstr_path = new char[size]();
#if defined(OS_LINUX)
        int ret = ::readlink("/proc/self/exe", cstr_path, size);
        size <<= 1;
        done = (ret != -1); // Check ENAMETOOLONG
#elif defined(OS_WINDOWS)
        ::GetModuleFileName(NULL, cstr_path, size);
        size <<= 1;
        done = (::GetLastError() != ERROR_INSUFICIENT_BUFFER);
#elif defined(OS_OSX)
        int ret = ::_NSGetExecutablePath(cstr_path, (uint32_t*)&size);
        done = (ret != -1);
        if (done) {
            char* temp_str = new char[strlen(cstr_path) + 1];
            ::realpath(cstr_path, temp_str);
            ::strcpy(cstr_path, temp_str);
            delete[] temp_str;
        }
#elif defined(OS_BSD)
        int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
        int ret = ::sysctl(mib, 4, cstr_path, &size, NULL, 0);
        size <<= 1;
        done = (ret == 0);
#else
# error Missing getExecutablePath implementation for your system.
#endif
    } while(!done);

    std::string path(cstr_path);
    delete[] cstr_path;
    return path;
}


/**
 * @brief Retrieves the list of local ip addresses from the system.
 *
 * @return A std::vector of strings with the textual representation of the
 *         ips.
 *
 * @throw std::runtime_error If any of the underlying system calls fails.
 *
 * @todo IP version filter
 */
std::vector<std::string> getLocalIPAddrs()
{
    std::vector<std::string> local_ips;

#if defined(OS_LINUX) || defined(OS_OSX) || defined(OS_BSD)
    struct ifaddrs *myaddrs, *ifa;
    char buf[64]; // enough for ipv6 representation
    
    if (getifaddrs(&myaddrs) != 0) {            
        char errbuf[256];
        strerror_r(errno, errbuf, sizeof(errbuf)); // ignore warning
        throw std::runtime_error(errbuf);
    }

    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || !(ifa->ifa_flags & IFF_UP))
            continue;

        void* in_addr;
        switch (ifa->ifa_addr->sa_family) {
            case AF_INET:
                in_addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
                break;
            case AF_INET6:                    
                in_addr = &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;
                break;
            default:
                continue;
        }
        
        inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf));
        local_ips.push_back(std::string(buf));
    }

    freeifaddrs(myaddrs);
#else
# error There is no implementation of getLocalIPAddrs for your system.
#endif

    return local_ips;
}
