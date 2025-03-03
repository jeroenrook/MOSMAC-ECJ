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

#ifndef _GGA_GENOME_HPP_
#define _GGA_GENOME_HPP_

#include <limits>
#include <vector>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>

#include "ggatypedefs.hpp"
#include "GGAValue.hpp"
#include "GGAInstance.hpp"
#include "GGAParameterTree.hpp"


// =================
// === GGAGenome ===
// =================

class GGAGenome 
{
public:
    enum Gender { COMPETITIVE, NONCOMPETITIVE };

    static bool compareObjValueAscendingOrder(const GGAGenome& first, 
                                              const GGAGenome& second);

    // construct/copy/destruct
    GGAGenome();
    GGAGenome(const GGAGenome&);
    GGAGenome(const GGAParameterTree& paramTree);
    GGAGenome(const GGAParameterTree& paramTree, GenomeMap genome);
    virtual ~GGAGenome();

    void mutate(const GGAParameterTree&);

    void initializeRandom(const GGAParameterTree&);

    
    bool isFeasible(const GGAParameterTree&) const;
    void makeFeasible(const GGAParameterTree&);

    const GenomeMap& genome() const;
    void setGenome(GenomeMap& genome);
    void setGenomeValue(std::string pname, GGAValue* val);

    int age() const;
    void age(int age);

    Gender gender() const;
    void gender(Gender gender);

    bool bestInPopulation() const;
    void bestInPopulation(bool best);

    double objValue() const;
    void objValue(double objValue);
    
    double performance(const GGAInstance& inst);
    const GGAInstancePerformanceMap& allPerformances() const;
    void setPerformance(const GGAInstance& inst, double perf);

    // Comparison operators (std::find, ...)
    GGAGenome& operator=(const GGAGenome&);
    bool operator==(const GGAGenome&) const;

    std::string toString() const;

private:

    Gender randGender();
    void mutateParameter(GGAValue& val, GGAParameter::pointer param);
    bool forbiddenSetViolated(const GenomeMap& settings) const;

    // Serialization
    friend class boost::serialization::access;
    template <class Archiver> void serialize(Archiver&, const unsigned int);

    // A genome is identifiable by its configuration, age and gender
    GenomeMap m_genome;
    int m_age;
    Gender m_gender;

    // This variables are used only to store genome results
    GGAInstancePerformanceMap m_performance;
    bool m_highlander; // THERE CAN BE ONLY ONE
    double m_objValue;
};

//==============================================================================
// GGAGenome Typedefs
typedef std::vector<GGAGenome> GGAGenomeVector;


//==============================================================================
// GGAGenome non-class/non-friend functions

void resetHighlander(GGAGenomeVector& genvec);

void resetObjectiveValue(GGAGenomeVector& genvec);

GGAGenome crossover(const GGAParameterTree& ptree, const GGAGenome& gen1,
                    const GGAGenome& gen2);

//==============================================================================
// GGAGenome public inline methods

/**
 *
 */
inline const GenomeMap& GGAGenome::genome() const 
{ return m_genome; }

/**
 *
 */
inline void GGAGenome::setGenome(GenomeMap& genome) 
{ m_genome = genome; }

/**
 *
 */
inline int GGAGenome::age() const 
{ return m_age; }

/**
 *
 */
inline void GGAGenome::age(int age) 
{ m_age = age; }

/**
 *
 */
inline GGAGenome::Gender GGAGenome::gender() const 
{ return m_gender; }

/**
 *
 */
inline void GGAGenome::gender(Gender gender) 
{ m_gender = gender; }

/**
 *
 */
inline bool GGAGenome::bestInPopulation() const 
{ return m_highlander; }

/**
 *
 */
inline void GGAGenome::bestInPopulation(bool best)
{ m_highlander = best; }

/**
 *
 */
inline double GGAGenome::objValue() const 
{ return m_objValue; }

/**
 *
 */
inline void GGAGenome::objValue(double objValue) 
{ m_objValue = objValue; }

/**
 *
 */
inline double GGAGenome::performance(const GGAInstance& inst)
{
    if(m_performance.find(inst) == m_performance.end())
        std::numeric_limits<double>::max(); // Exception??

    return m_performance[inst];
}

/**
 *
 */
inline const GGAInstancePerformanceMap& GGAGenome::allPerformances() const
{ return m_performance; }

/**
 *
 */
inline void GGAGenome::setPerformance(const GGAInstance& inst, double perf)
{ m_performance[inst] = perf; }


/**
 *
 */
inline GGAGenome& GGAGenome::operator=(const GGAGenome& other)
{
    m_genome = other.m_genome;
    m_age = other.m_age;
    m_gender = other.m_gender;

    m_performance = other.m_performance;
    m_highlander = other.m_highlander;
    m_objValue = other.m_objValue;

    return *this;
}


/**
 *
 */
inline bool GGAGenome::operator==(const GGAGenome& o) const
{ return m_genome == o.m_genome && m_age == o.m_age && m_gender == o.m_gender; }


//==============================================================================
// Serialization

template <class Archiver>
void GGAGenome::serialize(Archiver& ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(m_genome);
    ar & BOOST_SERIALIZATION_NVP(m_performance);
    ar & BOOST_SERIALIZATION_NVP(m_age);
    ar & BOOST_SERIALIZATION_NVP(m_gender);
    ar & BOOST_SERIALIZATION_NVP(m_highlander);
    ar & BOOST_SERIALIZATION_NVP(m_objValue);
}

#endif
