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


#ifndef _GGA_OS_PROBER_HPP_
#define _GGA_OS_PROBER_HPP_

#if defined(__linux) || defined(__linux__) || defined(linux)
# define OS_LINUX 1
#elif defined(__WIN32__) || defined(__WIN64__)
# define OS_WINDOWS 1
#elif defined(__APPLE__) || defined(__MACH__)
# define OS_OSX 1
#elif defined(__BSD__)
# define OS_BSD 1
#else
# error Unknown OS, check system macros in GGAOSProber.hpp
#endif

#endif // _GGA_OS_PROBER_HPP_
