//
// This file is part of DGGA.
// 
// The MIT License (MIT)
// 
// Copyright (c) 2015 Kevin Tierney and Josep Pon
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

#include <limits>
#include <map>
#include <queue>
#include <sstream>

#include "ggatypedefs.hpp"
#include "GGAGenome.hpp"
#include "GGAOptions.hpp"
#include "GGAParameter.hpp"
#include "GGAParameterTree.hpp"
#include "GGARandEngine.hpp"
#include "GGAUtil.hpp"
#include "GGAValue.hpp"
#include "OutputLog.hpp"


//==============================================================================
// Public static methods

/**
 *
 */
bool GGAGenome::compareObjValueAscendingOrder(const GGAGenome& first,
                                              const GGAGenome& second)
{
    double fObj = first.objValue();   
    double sObj = second.objValue();
    
    return fObj < sObj;
}

//==============================================================================
// Public methods

/**
 * Creates an empty genome that is usless until it is assigned by using the 
 * operator= or by calling the initializeRandom method
 */
GGAGenome::GGAGenome()
    : m_genome()
    , m_age(0)
    , m_gender(randGender())
    , m_performance()
    , m_highlander(false)
    , m_objValue(std::numeric_limits<double>::max())
{ }


/**
 *
 */
GGAGenome::GGAGenome(const GGAGenome& other)
    : m_genome(other.m_genome)
    , m_age(other.m_age)
    , m_gender(other.m_gender)
    , m_performance(other.m_performance)
    , m_highlander(other.m_highlander)
    , m_objValue(other.m_objValue)
{ }


/**
 * Creates a new feasible genome without taking ownership of the paramTree pointer.
 */
GGAGenome::GGAGenome(const GGAParameterTree& paramTree)
    : m_genome()
    , m_age(0)
    , m_gender(randGender())
    , m_performance()
    , m_highlander(false)
    , m_objValue(std::numeric_limits<double>::max()) 
{
    initializeRandom(paramTree);
}


/**
 * Creates a new feasible genome without taking ownership of the paramTree
 * pointer and assigns this genome the given genome.
 */
GGAGenome::GGAGenome(const GGAParameterTree& paramTree, GenomeMap genome)
    : m_genome(genome)
    , m_age(0)
    , m_gender(randGender())
    , m_performance()
    , m_highlander(false)
    , m_objValue(std::numeric_limits<double>::max()) 
{
    makeFeasible(paramTree);
}


/**
 *
 */
GGAGenome::~GGAGenome() 
{ }


/**
 *
 */
void GGAGenome::mutate(const GGAParameterTree& ptree) 
{
    const GGAOptions& opts = GGAOptions::instance();
    ParameterMap params = ptree.parameters();

    ParameterMap::iterator itr;
    for(itr = params.begin(); itr != params.end(); ++itr) {
        if(GGARandEngine::randDouble(0.0, 1.0) < opts.mutation_rate) {
            GGAValue& val = m_genome[itr->first];
            GGAParameter::pointer param = itr->second;
            mutateParameter(val, param);
        }
    }

    makeFeasible(ptree);
}


/**
 *
 */
void GGAGenome::initializeRandom(const GGAParameterTree& ptree)
{
    ParameterMap params = ptree.parameters();

    ParameterMap::iterator itr;
    for(itr = params.begin(); itr != params.end(); ++itr)
        m_genome[itr->first] = itr->second->valueInRange();

    makeFeasible(ptree);
}


/**
 * Checks whether the current genome assignment is feasible according to
 * the forbidden settings provided in the XML file.
 */
bool GGAGenome::isFeasible(const GGAParameterTree& ptree) const
{
    const std::vector<GenomeMap>& forbidden = ptree.forbiddenSettings();
    for(std::vector<GenomeMap>::const_iterator itr = forbidden.begin(); itr != forbidden.end(); ++itr) {
        if(forbiddenSetViolated(*itr)) {
            return false;
        }
    }
    return true;
}


/**
 * Randomly performs mutations on the parts of the genome that are
 * infeasible until it becomes feasible.
 */
void GGAGenome::makeFeasible(const GGAParameterTree& ptree)
{
    const std::vector<GenomeMap>& forbidden = ptree.forbiddenSettings();
    int giveUp = 0; // just to prevent infinite loops.. we still have to exit GGA if this happens, though TODO just kill the genome and replace it?
    int maxItrs = 5000;
    // Note: I realize this isn't really the best way to go about repairing the genome as with some probability we could be looping for a long time... but it is easy to implement and will usually be fast :)
    while(!isFeasible(ptree) && giveUp < maxItrs) {
        for(std::vector<GenomeMap>::const_iterator itr = forbidden.begin(); itr != forbidden.end(); ++itr) {
            int innerGiveUp = 0;
            while(forbiddenSetViolated(*itr) && innerGiveUp < maxItrs) {
                GenomeMap::const_iterator gmitr = itr->begin();
                std::advance(gmitr, GGARandEngine::randInt(0, itr->size() - 1));
                mutateParameter(m_genome[gmitr->first], mapAt(ptree.parameters(), gmitr->first));
                innerGiveUp++;
            }
            if(innerGiveUp >= maxItrs) {
                LOG_ERROR("Unable to find feasible setting for genome. The constraints are too hard for GGA, please consider relaxing them or handling them somehow in the wrapper.");
                exit(1);
            }
        }
        giveUp++;
    }
    if(giveUp >= maxItrs) {
        LOG_ERROR("Unable to find feasible setting for genome. The constraints are too hard for GGA, please consider relaxing them or handling them somehow in the wrapper.");
        exit(1);
    }
}


/**
 *
 */
std::string GGAGenome::toString() const
{
    std::stringstream ret;
    ret << "[GGAGenome: " << this << "; Gender: " 
        << (m_gender == COMPETITIVE ? "C" : "N")
        << "; Age: " << m_age << "; Genome: " 
        << OutputLog::genomeMapToString(m_genome)
        << "]";
    return ret.str();
}


//==============================================================================
// Private methods

/**
 *
 */
GGAGenome::Gender GGAGenome::randGender() 
{
    return GGARandEngine::coinFlip() ? GGAGenome::COMPETITIVE : 
                                       GGAGenome::NONCOMPETITIVE;
}

/**
 *
 */
// TODO This is putting too much probability mass on the sides of small integer ranges
void GGAGenome::mutateParameter(GGAValue& val, GGAParameter::pointer param) 
{
    if(val.isLong()) { // Gaussian
        val.setValue(GGARandEngine::randGaussianLong(
                                                val.getLong(), 
                                                param->rangeStart().getLong(),
                                                param->rangeEnd().getLong()));
    } else if(val.isDouble()) { // Gaussian
        val.setValue(GGARandEngine::randGaussianDouble(
                                            val.getDouble(), 
                                            param->rangeStart().getDouble(),
                                            param->rangeEnd().getDouble()));
    } else if(val.isString()) { // Uniform
        long newIndex = GGARandEngine::randLong(param->rangeStart().getLong(),
                                                param->rangeEnd().getLong());
        val.setValue(param->categoricalDomain()[newIndex]);
    } else { // Don't mutate
        LOG_ERROR("Warning: GGAGenome contains a value that is not supported. Name: " << param->name());
        LOG_ERROR("VALUE: " << val.toString());
    }
}

/**
 *
 */
bool GGAGenome::forbiddenSetViolated(const GenomeMap& settings) const 
{
    bool allMatch = true;
    for(GenomeMap::const_iterator gmitr = settings.begin(); gmitr != settings.end() && allMatch; ++gmitr) {
        allMatch = allMatch && m_genome.at(gmitr->first) == gmitr->second;
    }
    return allMatch;
}


//==============================================================================
// GGAGenome non-class/non-friend functions


/**
 *
 */
void resetHighlander(GGAGenomeVector& genvec)
{
    GGAGenomeVector::iterator it;

    for (it = genvec.begin(); it != genvec.end(); ++it)
        it->bestInPopulation(false);
}


/**
 *
 */
void resetObjectiveValue(GGAGenomeVector& genvec)
{
    GGAGenomeVector::iterator it;

    for (it = genvec.begin(); it != genvec.end(); ++it)
        it->objValue(std::numeric_limits<double>::max());
}


/**
 *
 */
GGAGenome crossover(const GGAParameterTree& ptree, const GGAGenome& gen1, 
                    const GGAGenome& gen2)
{
    if (gen1.gender() == gen2.gender())
        LOG_ERROR("Warning: Mating two genomes of the same gender. This"
                  " probably shouldn't happen.");
    
    GenomeMap childGenome;
    std::map<GGATreeNode::const_pointer, GGATreeNode::Label> node_label;
    
    bool gen1_competitive = gen1.gender() == GGAGenome::COMPETITIVE;
    const GenomeMap& nGenome = gen1_competitive ? gen1.genome() : gen2.genome();
    const GenomeMap& cGenome = gen1_competitive ? gen2.genome() : gen1.genome();

    GGATreeNode::const_pointer cur = ptree.root();
    GGATreeNode::const_pointer childNode = cur;
    GGAParameter::const_pointer curParam = cur->parameter();
    std::string paramName = curParam->name();
    
    // Label the root node
    if(cur->type() == GGATreeNode::AND 
       || mapAt(nGenome, paramName) == mapAt(cGenome, paramName)) 
    {
        node_label[cur] = GGATreeNode::O;
        childGenome[paramName] = GGAValue(mapAt(cGenome, paramName));
    } else {
        if(GGARandEngine::coinFlip()) {
            node_label[cur] = GGATreeNode::C;
            childGenome[paramName] = GGAValue(mapAt(cGenome, paramName));
        } else {
            node_label[cur] = GGATreeNode::N;
            childGenome[paramName] = GGAValue(mapAt(nGenome, paramName));
        }
    }

    const GGAOptions& opts = GGAOptions::instance();    
    std::queue<GGATreeNode::const_pointer> unvisited;
    unvisited.push(cur);
    
    while(!unvisited.empty()) {
        cur = unvisited.front();
        unvisited.pop();

        for(size_t i = 0; i < cur->children().size(); ++i) {
            childNode = cur->children()[i];
            unvisited.push(childNode);            
            curParam = childNode->parameter();

            paramName = curParam->name();

            GGAValue useValue;
            GGAValue altValue;
            GGATreeNode::Label useLabel;
            GGATreeNode::Label altLabel;

            if(node_label[cur] == GGATreeNode::O) {
                if(mapAt(cGenome, paramName) == mapAt(nGenome, paramName))
                {
                    childGenome[paramName] = GGAValue(
                                                    mapAt(cGenome, paramName));
                    node_label[childNode] = GGATreeNode::O;
                } else {
                    if(GGARandEngine::coinFlip()) {
                        childGenome[paramName] = GGAValue(
                                                    mapAt(cGenome, paramName));
                        node_label[childNode] = GGATreeNode::C;
                    } else {
                        node_label[childNode] = GGATreeNode::N;
                        childGenome[paramName] = GGAValue(
                                                    mapAt(nGenome, paramName));
                    }
                }

            } else { 
                if(node_label[cur] == GGATreeNode::C) {
                    useValue = GGAValue(mapAt(cGenome, paramName));
                    useLabel = GGATreeNode::C;
                    altValue = GGAValue(mapAt(nGenome, paramName));
                    altLabel = GGATreeNode::N;
                } else {
                    useValue = GGAValue(mapAt(nGenome, paramName));
                    useLabel = GGATreeNode::N;
                    altValue = GGAValue(mapAt(cGenome, paramName));
                    altLabel = GGATreeNode::C;
                }
                // With some probability, don't use the parameters from the
                // same gender as the parent
                if(GGARandEngine::randDouble(0.0, 1.0) < opts.subtree_split) {
                    childGenome[paramName] = altValue;
                    node_label[childNode] = altLabel;
                } else {
                    childGenome[paramName] = useValue;
                    node_label[childNode] = useLabel;
                }
            }
        }
    }

    return GGAGenome(ptree, childGenome);
}
