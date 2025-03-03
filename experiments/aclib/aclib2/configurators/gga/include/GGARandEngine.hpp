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

/*
 * Class: RandEngine
 * Author: Kevin Tierney
 * Purpose: Returns random numbers using tr1/random
 *
 * Modified: Josep Pon Farreny
 * Changed tr1 random to boost random, due to missing tr1 files in OS X.
 */

#include <boost/random.hpp>
#include <ctime>

#include "ggatypedefs.hpp"

#ifndef _GGA_RAND_ENGINE_HPP_
#define _GGA_RAND_ENGINE_HPP_

class GGARandEngine {
    private:
        // Public static variables
        //
        static bool hasSeed;
        static boost::minstd_rand0 eng;

        // construct
        GGARandEngine() {}

    public:
        static boost::minstd_rand0& getEngine();

        static int randInt(int rangeStart, int rangeEnd);
        static long randLong(long rangeStart, long rangeEnd);
        static double randDouble(double rangeStart, double rangeEnd);

        static bool coinFlip();        

        static int randGaussianInt(int mu, int rangeStart, int rangeEnd);
        static long randGaussianLong(long mu, long rangeStart, long rangeEnd);
        static double randGaussianDouble(double mu, double rangestart,
                                         double rangeEnd);
};


#endif // _GGA_RAND_ENGINE_HPP_
