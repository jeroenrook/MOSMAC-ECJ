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


#ifndef _GGA_SYSTEM_HPP_
#define _GGA_SYSTEM_HPP_

#include <string>
#include <vector>


/**
 * @return Wall clock time in seconds.
 */
double wallClockTime();


/**
 * @return User time of the current process plus the user time of all its 
 *         children.
 */
double userTime();


/**
 * @return System time of the current process plus the system time of all its
 *         children.
 */
double systemTime();


/**
 * @brief Suspends the execution of the calling thread until the time specified
 *        has elapsed. If a signal resumes the process, it is suspended again
 *        until the process has slept for the specified amount of time.
 *
 * @throw std::runtime_error If any of the system calls fails.
 */
void sleepFor(unsigned seconds, unsigned nanoseconds);


/**
 * @brief Gets the absolute path to the executable file of the running
 *        program.
 * @return A std::string with the absolute path of the running program's
 *         exe file.
 */
std::string getExecutablePath();


/**
 * @brief Retrieves the list of local ip addresses from the system.
 *
 * @return A std::vector of strings with the textual representation of the
 *         ips.
 *
 * @throw std::runtime_error If any of the underlaying system calls fails.
 */
std::vector<std::string> getLocalIPAddrs();


#endif // _GGA_SYSTEM_HPP_
