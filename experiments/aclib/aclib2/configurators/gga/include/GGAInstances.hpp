/* 
 * File:   GGAInstances.hpp
 * Author: Josep Pon Farreny
 */
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

#ifndef GGAINSTANCES_HPP
#define	GGAINSTANCES_HPP

#include <vector>

#include <boost/unordered_map.hpp>

#include "GGAInstance.hpp"

class GGAInstances {
public:
    GGAInstances();
    GGAInstances(const GGAInstances& orig);
    virtual ~GGAInstances();
    
    // Access all the internal instances
    const std::vector<GGAInstance>& getAllInstances() const;
    const GGAInstance& getInstance(unsigned index) const;
    const std::vector<unsigned>& getCluster(unsigned index) const;
    
    unsigned getNumberOfInstances() const;
    unsigned getNumberOfClusters() const;
    
    bool hasClusters() const;
    bool hasInstances() const;
    
    // Load data from files
    void loadInstancesFile(const std::string& file_path);
    void loadInstanceClustersFile(const std::string& file_path);
    
private:
    std::vector<GGAInstance> m_instances;
    boost::unordered_map<std::string, std::size_t> m_instances_index;
    std::vector< std::vector<unsigned> > m_clusters;
    
};

//==============================================================================
// GGAInstances public inline methods

inline const std::vector<GGAInstance>& GGAInstances::getAllInstances() const
{
    return m_instances;
}

inline const GGAInstance& GGAInstances::getInstance(unsigned index) const
{
    return m_instances[index];
}

inline const std::vector<unsigned>& GGAInstances::getCluster(
        unsigned index) const
{
    return m_clusters[index];
}
    
inline unsigned GGAInstances::getNumberOfInstances() const
{
    return m_instances.size();
}

inline unsigned GGAInstances::getNumberOfClusters() const 
{
    return m_clusters.size();
}

inline bool GGAInstances::hasInstances() const
{
    return m_instances.size() > 0;
}

inline bool GGAInstances::hasClusters() const 
{
    return m_clusters.size() > 0;
}

#endif	/* GGAINSTANCES_HPP */

