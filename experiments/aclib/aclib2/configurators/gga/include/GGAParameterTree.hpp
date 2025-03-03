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


#ifndef _GGA_PARAMETER_TREE_HPP_
#define _GGA_PARAMETER_TREE_HPP_

#include <algorithm>
#include <map>
#include <vector>
#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/shared_ptr.hpp>

#include "ggatypedefs.hpp"
#include "GGACommand.hpp"
#include "GGAParameter.hpp"


/*
 * Represents a single node in the And-Or Tree structure. This class takes
 * ownership of all GGAParameters given to it. Upon deletion, the children of
 * the GGATreeNode will be deleted in a cascading effect from the root.
 */
class GGATreeNode 
    : public boost::enable_shared_from_this<GGATreeNode>
{
public:
    typedef boost::shared_ptr<GGATreeNode> pointer;
    typedef boost::shared_ptr<const GGATreeNode> const_pointer;

    enum NodeType {AND, OR};
    enum Label {O, C, N};

    // construct/destruct
    GGATreeNode();
    GGATreeNode(NodeType type, GGATreeNode* parent, GGAParameter* param);
    virtual ~GGATreeNode();
      
    void addChild(pointer child);
    void addChildren(std::vector<pointer>& children);    
    bool hasChild(pointer child);

    void parent(pointer parent);
    GGATreeNode::pointer parent();
    
    GGAParameter::const_pointer parameter() const;
    GGAParameter::pointer parameter();
    void parameter(GGAParameter::pointer param);

    const std::vector<GGATreeNode::const_pointer>& children() const;
    const std::vector<GGATreeNode::pointer>& children();

    void type(NodeType t);
    NodeType type() const;

    std::string str();

private:
    GGATreeNode(const GGATreeNode&); // Intentionally unimplemented
    GGATreeNode& operator=(const GGATreeNode&);
    
    // serialization
    friend class boost::serialization::access;
    template<class Archiver>void serialize(Archiver&, const unsigned int);

    //
    NodeType m_type;
    GGAParameter::pointer m_param;

    GGATreeNode::pointer m_parent;

    // those vectors are different objects and the replication is necessary
    // in order to have accest to const pointers.
    std::vector<GGATreeNode::pointer> m_children;
    std::vector<GGATreeNode::const_pointer> m_const_children;

    //Label m_label;
};



/*
 * Stores the And-Or Tree using the helper class GGATreeNode
 * Note that this object takes ownership of all GGAParameters given to it
 */
class GGAParameterTree 
{
public:

    // Singleton methods
    //
    static const GGAParameterTree& instance();
    static GGAParameterTree& mutableInstance();
    static void deleteInstance();

    // destructor
    virtual ~GGAParameterTree();

    void parseTreeFile(const std::string& file);
    
    // Access tree root
    GGATreeNode::const_pointer root() const;
    GGATreeNode::pointer root();

    const std::vector<GenomeMap>& seededGenomes() const;
    const GGACommand* command() const;
    const ParameterMap& parameters() const;

    const std::vector<GenomeMap>& forbiddenSettings() const { return m_forbidden; }
    const std::vector<std::string>& paramNames() const { return m_paramnames; }

    std::string toString() const;
    
private:
    // singleton
    GGAParameterTree();

    // intentionally unimplemented
    GGAParameterTree(const GGAParameterTree&); 
    GGAParameterTree& operator=(const GGAParameterTree&);
    
    //
    void buildTree(const boost::property_tree::iptree&);
    void buildTreeCommand(const boost::property_tree::iptree&);
    void buildTreeErrRegex(const boost::property_tree::iptree&);
    void buildTreeResRegex(const boost::property_tree::iptree&);
    void buildTreeNode(const boost::property_tree::iptree&);
    
    GenomeMap parseSeededGenome(const boost::property_tree::iptree&);
    void parseForbiddenBlocks(const boost::property_tree::iptree&);
    void parseForbiddenBlock(const boost::property_tree::iptree&);
    GGATreeNode::pointer parseNodes(const boost::property_tree::iptree&);

    GGATreeNode::pointer parseNode(const boost::property_tree::iptree& pt,
                                   const GGATreeNode::pointer parent);    

    void validateSeededGenomes();
    bool isNameValid(const std::string& name);
 
    void propagateOrPaths();

    std::string strParamTreeHelper(GGATreeNode::pointer cur) const;

    // serialization
    friend class boost::serialization::access;
    template<class Archiver>void serialize(Archiver&, const unsigned int);

    // static variables
    static GGAParameterTree* s_pInstance;

    //
    GGATreeNode::pointer m_root;
    GGACommand* m_cmd;
    std::string m_errRegEx;
    std::string m_resRegEx;
    ParameterMap m_parameters;
    std::vector<GenomeMap> m_seededGenomes;
    std::vector<std::string> m_paramnames; // provides an ordering of the non-trivial parameter names for use in feature vectors. Trivial parameters are those with a 0 size domain (e.g. root AND node, etc)
    std::vector<GenomeMap> m_forbidden;
};


//==============================================================================
// GGATreeNode public inline methods

/**
 * Adds the children to this without notifying them that their
 * parent has been set
 */
inline void GGATreeNode::addChild(GGATreeNode::pointer child)
{
    m_children.push_back(child);
    m_const_children.push_back(child);
}


/**
 *
 */
inline void GGATreeNode::addChildren(
                                std::vector<GGATreeNode::pointer>& children) 
{
    m_children.insert(m_children.end(), children.begin(), children.end());

    // In order to have accest to the vector of const children
    m_const_children.insert(m_const_children.end(), children.begin(), 
                            children.end());
}


/**
 *
 */
inline bool GGATreeNode::hasChild(GGATreeNode::pointer child) 
{
    return std::find(m_children.begin(), m_children.end(), child) != 
                                                            m_children.end();
}


/**
 * Sets the parent to the specified node and notifies
 * the parent that this is now a child
 */
inline void GGATreeNode::parent(GGATreeNode::pointer parent)
{
    // If instead shared_from_this, here we use pointer(this), the referene
    // counter of the original shared_ptr is not preserved and this will be 
    // deleted at the end of this function causing a SEGFAULT when the original
    // shared_ptr attemps to delete 'this'
    if (parent.get() == NULL)
        return; 
    
    m_parent = parent;
    if (!m_parent->hasChild(shared_from_this()))
        m_parent->addChild(shared_from_this());
}

/** */
inline GGATreeNode::pointer GGATreeNode::parent() 
{ return m_parent; }

/** */
inline GGAParameter::const_pointer GGATreeNode::parameter() const
{ return m_param; }

/** */
inline GGAParameter::pointer GGATreeNode::parameter() 
{ return m_param; }

/** */
inline void GGATreeNode::parameter(GGAParameter::pointer param) 
{ m_param = param; }


/** */
inline const std::vector<GGATreeNode::const_pointer>&
GGATreeNode::children() const
{ return m_const_children; }

/** */
inline const std::vector<GGATreeNode::pointer>& GGATreeNode::children() 
{ return m_children; }

/** */
inline void GGATreeNode::type(GGATreeNode::NodeType t)
{ m_type = t; }

/** */
inline GGATreeNode::NodeType GGATreeNode::type() const 
{ return m_type; }

//==============================================================================
// GGATreeNode private inline/template methods

template<typename Archiver>
void GGATreeNode::serialize(Archiver& ar, const unsigned int version)
{
    ar & m_children;
    ar & m_parent;
    ar & m_param;    
    ar & m_type;
}


//==============================================================================
// GGAParameterTree public inline static methods

/**
 *
 */
inline const GGAParameterTree& GGAParameterTree::instance()
{
    return *(s_pInstance != NULL ? 
                s_pInstance : (s_pInstance = new GGAParameterTree));
}

/**
 *
 */
inline GGAParameterTree& GGAParameterTree::mutableInstance()
{
    return const_cast<GGAParameterTree&>(GGAParameterTree::instance());
}

/**
 *
 */
inline void GGAParameterTree::deleteInstance()
{
    delete s_pInstance;
    s_pInstance = NULL;
}

//==============================================================================
// GGAParameterTree public inline methods

/** */
inline GGATreeNode::const_pointer GGAParameterTree::root() const
{ return m_root; }

/** */
inline GGATreeNode::pointer GGAParameterTree::root()
{ return m_root; }

/** */
inline const std::vector<GenomeMap>& GGAParameterTree::seededGenomes() const
{ return m_seededGenomes; }

/** */
inline const GGACommand* GGAParameterTree::command() const
{ return m_cmd; }

/** */
inline const ParameterMap& GGAParameterTree::parameters() const
{ return m_parameters; }


//==============================================================================
// GGAParameterTree private inline/template methods

template<typename Archiver>
void GGAParameterTree::serialize(Archiver& ar, const unsigned int version)
{
    ar & m_root;
    ar & m_cmd;
    ar & m_errRegEx;
    ar & m_resRegEx;
    ar & m_parameters;
    ar & m_seededGenomes;
    ar & m_paramnames;
    ar & m_forbidden;
}


#endif // _GGA_PARAMETER_TREE_HPP_
