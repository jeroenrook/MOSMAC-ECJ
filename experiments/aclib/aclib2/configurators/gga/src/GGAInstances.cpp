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
//
// Author: Josep Pon Farreny

#include <fstream>

#include <boost/algorithm/string/trim.hpp>

#include "GGAExceptions.hpp"
#include "GGAInstances.hpp"
#include "OutputLog.hpp"

/**
 * @brief Creates a new empty instance of GGAInstances.
 */
GGAInstances::GGAInstances()
    : m_instances()
    , m_instances_index()
    , m_clusters()
{ }

/**
 * @brief Creates a new instance of GGAInstances as a copy of the given one.
 * @param orig
 */
GGAInstances::GGAInstances(const GGAInstances& orig)
    : m_instances(orig.m_instances)
    , m_instances_index(orig.m_instances_index)
    , m_clusters(orig.m_clusters)
{ }

/**
 * Virtual destructor.
 */
GGAInstances::~GGAInstances()
{ }

/**
 * @brief Loads instances from an instance-seed file.
 */
void GGAInstances::loadInstancesFile(const std::string& file_path)
{
    std::ifstream ifs(file_path.c_str());

    if (ifs.is_open()) {
        std::string line;

        for (int line_num = 1; std::getline(ifs, line); ++line_num) {
            boost::algorithm::trim(line);

            try {
                GGAInstance instance = GGAInstance::fromString(line);
                m_instances_index[instance.getInstance()] = m_instances.size();
                m_instances.push_back(instance);
            } catch (const GGAMalformedInstanceException& e) {
                throw GGAMalformedInstanceException(e.message(), file_path,
                                                 line_num);
            }
        }
        ifs.close();

    } else {
        throw GGAFileNotFoundException(file_path);
    }
}

/**
 * @brief Loads instance clusters from a clusters file.
 */
void GGAInstances::loadInstanceClustersFile(const std::string& file_path) 
{
    std::ifstream ifs(file_path.c_str());
    
    if (ifs.is_open()) {
        std::string line;
        std::vector<unsigned> cluster_instances_index;
        
        LOG_VERY_VERBOSE("==== Parsing clusters ====")
        for (int line_num = 1; std::getline(ifs, line); ++line_num) {
            boost::algorithm::trim(line);
            
            if (!line.empty()) {
                if (m_instances_index.count(line)) {
                    LOG_DEBUG(line << " Index: " << m_instances_index[line]);
                    cluster_instances_index.push_back(m_instances_index[line]);
                } else {
                    throw GGAException(
                            "[GGAInstances::loadInstanceClustersFile] Non"
                            " existen instance. Check the instances file.");
                }
            } else {
                if (!cluster_instances_index.empty()) {
                    m_clusters.push_back(cluster_instances_index);
                    LOG_VERY_VERBOSE("---- Parsed cluster with: " 
                                     << cluster_instances_index.size()
                                     << " instances ----");
                    LOG_DEBUG("---- New cluster ----");
                }
                cluster_instances_index.clear();
            }
        }
        LOG_VERY_VERBOSE("==== Parsing clusters ====");
        
        ifs.close();
    } else {
        throw GGAFileNotFoundException(file_path);
    }
}
