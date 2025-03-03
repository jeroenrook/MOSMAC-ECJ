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


#include <cstdlib>

#include "GGAOptions.hpp"
#include "OutputLog.hpp"
#include "GGARandEngine.hpp"

bool GGARandEngine::hasSeed = false;
boost::minstd_rand0 GGARandEngine::eng;


boost::minstd_rand0& GGARandEngine::getEngine()
{
    if(!hasSeed) {
        const GGAOptions& opts = GGAOptions::instance();
        eng.seed(opts.seed);
        std::srand(opts.seed); // Just be sure...
        hasSeed = true;
		LOG_DEBUG("Setting GGARandEngine seed: " << opts.seed);
    }

    return eng;
}


/**
 * Returns uniformly distributed random int
 * rangeStart <= rangeEnd
 * x \in [rangeStart, rangeEnd]
 */
int GGARandEngine::randInt(int rangeStart, int rangeEnd)
{
    boost::uniform_int<int> dist(rangeStart, rangeEnd);
    boost::variate_generator< boost::minstd_rand0&, boost::uniform_int<int> > 
        r(getEngine(), dist);
    int tmp = r();
    return tmp;
}


/**
 * Returns uniformly distributed random long
 * rangeStart <= rangeEnd
 * x \in [rangeStart, rangeEnd]
 */
long GGARandEngine::randLong(long rangeStart, long rangeEnd)
{
    boost::uniform_int<long> dist(rangeStart, rangeEnd);
    boost::variate_generator< boost::minstd_rand0&, boost::uniform_int<long> > 
        r(getEngine(), dist);
    long tmp = r();
    return tmp;
}


/**
 * Returns a uniformly distributed random double
 * rangeStart <= rangeEnd
 * x \in [rangeStart, rangeEnd]
 */
double GGARandEngine::randDouble(double rangeStart, double rangeEnd)
{
    boost::uniform_real<> dist(rangeStart, rangeEnd);
    boost::variate_generator<boost::minstd_rand0&, boost::uniform_real<> >
        r(getEngine(), dist);

    /*LOG("DOUBLE: " << r());
    LOG("DOUBLE: " << r());*/

    return r();
}


/**
 *
 */
bool GGARandEngine::coinFlip()
{
    return randDouble(0.0, 1.0) < 0.5;
}


/**
 *
 */
int GGARandEngine::randGaussianInt(int mu, int rangeStart, int rangeEnd)
{
    double randVal = randGaussianDouble(static_cast<double>(mu),
        static_cast<double>(rangeStart), static_cast<double>(rangeEnd));
    return static_cast<int>(randVal);
}


/**
 *
 */
long GGARandEngine::randGaussianLong(long mu, long rangeStart, long rangeEnd)
{
    double randVal = randGaussianDouble(static_cast<double>(mu),
        static_cast<double>(rangeStart), static_cast<double>(rangeEnd));
    return static_cast<long>(randVal);
}


/**
 *
 */
double GGARandEngine::randGaussianDouble(double mu, double rangeStart,
                                         double rangeEnd)
{
    const GGAOptions& opts = GGAOptions::instance();
    
    double sigma = (rangeEnd - rangeStart) * opts.sigma_pct;
    boost::normal_distribution<double> dist(mu, sigma);
    boost::variate_generator<
                    boost::minstd_rand0&,
                    boost::normal_distribution<double> > r(getEngine(), dist);
    double rval = r();

    int preventInf = 0;
    while ((rval < rangeStart || rval > rangeEnd) && preventInf < 400) {
        rval = r();
        preventInf += 1;
    }

    if(preventInf >= 400) {
        // TODO Exception??
        LOG_ERROR("Unable to randomly generate a value in the given range from"
                  " a Gaussian distribution after 400 tries. This should not"
                  " happen.");
        rval = mu;
    }
    
    return rval;
}
