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


#include <algorithm>
#include <sstream>

#include "GGAGenome.hpp"
#include "GGAOptions.hpp"
#include "GGARandEngine.hpp"
#include "OutputLog.hpp"

#include "GGAPopulation.hpp"

//==============================================================================
// GGAPopulation public methods

/**
 *
 */
GGAPopulation::GGAPopulation()
    : m_popN()
    , m_popC()
{ }


/**
 *
 */
GGAPopulation::~GGAPopulation() 
{ }


/**
 *
 */
void GGAPopulation::addRandomGenomes(const GGAParameterTree& ptree, int howMany)
{
    const GGAOptions& opts = GGAOptions::instance();
    int maxAge = opts.genome_age;
    
    for(int i = 0; i < howMany; ++i) {
        GGAGenome toAdd(ptree);
        toAdd.age(GGARandEngine::randInt(1, maxAge));
        addGenome(toAdd);
    }
}


/**
 *
 */
void GGAPopulation::addGenomes(const GGAGenomeVector& genomes) {
    GGAGenomeVector::const_iterator itr;
    for(itr = genomes.begin(); itr != genomes.end(); ++itr)
        addGenome(*itr);
}


/*
 * Adds a genome to the population of the gender specified within the Genome,
 * taking ownership of the pointer.
 */
void GGAPopulation::addGenome(const GGAGenome& genome) {
    if(genome.gender() == GGAGenome::COMPETITIVE)
        m_popC.push_back(genome);
    else
        m_popN.push_back(genome);
}


/**
 * Removes the genome from the population and deletes it.
 */
void GGAPopulation::removeGenome(const GGAGenome& genome) 
{
    if(genome.gender() == GGAGenome::COMPETITIVE) {
        GGAGenomeVector::iterator pos = std::find(m_popC.begin(), m_popC.end(),
                                               genome);
        if(pos != m_popC.end())
            m_popC.erase(pos);
        //else
            // TODO Exception/error

    } else {
        GGAGenomeVector::iterator pos = std::find(m_popN.begin(), m_popN.end(),
                                               genome);
        if(pos != m_popN.end())
            m_popN.erase(pos);
        // else
            // TODO Exception/error
    }
}


/**
 *
 */
void GGAPopulation::agePopulation()
{
    int count = 0;
    const GGAOptions& opts = GGAOptions::instance();
    int maxAge = opts.genome_age;
    
    GGAGenomeVector::iterator itr = m_popN.begin();
    while (itr != m_popN.end()) {
        itr->age(itr->age() + 1);

        if(itr->age() > maxAge) {
            ++count;
            itr = m_popN.erase(itr);
        } else {
            ++itr;
        }
    }
    
    itr = m_popC.begin();
    while (itr != m_popC.end()) {
        itr->age(itr->age() + 1);

        if(itr->age() > maxAge && !itr->bestInPopulation()) {
            ++count;
            itr = m_popC.erase(itr);
        } else {
            ++itr;
        }
    }
    LOG_ERROR("Killed: " << count);
}

/*;
*/
/**
 *
 */
void GGAPopulation::updatePopulation(const GGAGenome& genome)
{
    GGAGenomeVector::iterator it, itend;

    switch (genome.gender()) {
        case GGAGenome::COMPETITIVE:
            it = std::find(m_popC.begin(), m_popC.end(), genome);
            itend = m_popC.end();
            break;
        case GGAGenome::NONCOMPETITIVE:
            it = std::find(m_popN.begin(), m_popN.end(), genome);
            itend = m_popN.end();
            break;
        default:            
            throw std::domain_error("[GGAPopulation::updatePopulation]"
                                    " unknown genome gender.");
    }

    if (it == itend) {
        std::stringstream ss;
        ss << "The genome: " << std::endl << genome.toString() 
           << std::endl << "Can not be updated because it is no present"
              " in this population.";
        throw std::out_of_range(ss.str());
    }

    *it = genome;
}

/**
 *
 */
void GGAPopulation::updatePopulation(const GGAGenomeVector& genvec)
{
    GGAGenomeVector::const_iterator it;
    for (it = genvec.begin(); it != genvec.end(); ++it)
        updatePopulation(*it);
}

/**
 *
 */
std::string GGAPopulation::toString() const 
{
    std::stringstream ss;
    ss << "[GGAPopulation (" << m_popC.size() + m_popN.size() 
       << ")]" << std::endl << "[Competitive (" << m_popC.size() << ")]" 
       << std::endl;

    GGAGenomeVector::const_iterator itr;
    for(itr = m_popC.begin(); itr != m_popC.end(); ++itr) {
        ss << itr->toString() << std::endl;
    }

    ss << "[Non-competitive (" << m_popN.size() << ")]" << std::endl;
    for(itr = m_popN.begin(); itr != m_popN.end(); ++itr) {
        ss << itr->toString() << std::endl;
    }
    return ss.str();
}


