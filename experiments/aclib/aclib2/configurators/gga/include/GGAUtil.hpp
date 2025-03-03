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

#ifndef _GGA_UTIL_HPP_
#define _GGA_UTIL_HPP_

#include <stdexcept>

#include "ggatypedefs.hpp"
#include "GGAGenome.hpp"
#include "GGAInstances.hpp"
#include "GGALearningStrategies.hpp"
#include "GGAParameterTree.hpp"


// Build a command from the genome
std::string makeCommand(const GGAParameterTree& ptree, const GenomeMap& genome,
                        const GGAInstance& inst); 

std::string makeCommand(const GGAParameterTree& ptree, const GenomeMap& genome,
                        const StringMap& progValues);

// Balance tournament workload
UIntVector balanceTournaments(size_t participants, size_t tourny_size);

// Returns a learning strategy (based on the option specified in GGAOptions)
GGALearningStrategy* createLearningStrategy(const GGAInstances& instances);

// Appends trajectory information to the trajectory file specified in options
void outputTrajectory(const GGAParameterTree& ptree, const GenomeMap& genome,
                      double utime, double wtime);

// Prints all the options in GGAOptions
void printOptions();


/**
 * Workaround to retrieve constant references from std::map in C++03, this 
 * problem is solved in C++11 (map::at()).
 *
 * @throw std::out_of_range if k is not the key of an element in the map.
 */
template <class Map>
const typename Map::mapped_type& mapAt(const Map& map, 
                                       const typename Map::key_type& k)
{
    typename Map::const_iterator it = map.find(k);
    
    if (it == map.end())
        throw std::out_of_range("The given map does not contain any value"
                                " associated to the specified key.");

    return it->second;
}

#endif // _GGA_UTIL_HPP_
