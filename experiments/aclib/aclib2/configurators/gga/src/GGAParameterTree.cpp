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



#include <cstring>
#include <cstdio>

#include <algorithm>
#include <exception>
#include <fstream>
#include <map>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include "GGAExceptions.hpp"
#include "GGAParameterTree.hpp"
#include "GGAValue.hpp"
#include "GGARandEngine.hpp"

// Local typedefs
typedef boost::char_separator<char> CharSeparator;
typedef boost::tokenizer<CharSeparator> CharTokenizer;

//==============================================================================
// constants local to this file public constants

static const std::string XML_ROOT               ("algtune");
static const std::string XML_NODE               ("node");
static const std::string XML_CMD                ("cmd");
static const std::string XML_ERR_REGEX          ("error_re");
static const std::string XML_RES_REGEX          ("result_re");
static const std::string XML_SEEDGENOME         ("seedgenome");
static const std::string XML_SG_VARIABLE        ("variable");
static const std::string XML_FORBIDDEN          ("forbidden");
static const std::string XML_FORBIDDEN_BLOCK    ("forbid");
static const std::string XML_FORBIDDEN_SETTING  ("setting");

// Read boost::property_tree::xml_parser documentation to understand these tags
static const std::string XML_ATTR               ("<xmlattr>");
static const std::string XML_ATTR_TYPE          ("type");
static const std::string XML_ATTR_NAME          ("name");
static const std::string XML_ATTR_TRAJNAME      ("trajname");
static const std::string XML_ATTR_START         ("start");
static const std::string XML_ATTR_END           ("end");
static const std::string XML_ATTR_PREFIX        ("prefix");
static const std::string XML_ATTR_CATEGORIES    ("categories");
static const std::string XML_ATTR_SG_NAME       ("name");
static const std::string XML_ATTR_SG_VALUE      ("value");

static const std::string XML_OPT_AND            ("and");
static const std::string XML_OPT_OR             ("or");

static const std::string BAD_CHARS              ("\t\n\r $");

//==============================================================================
// Help functions local to this file

static GGATreeNode::NodeType getNodeType(const std::string& str_type)
{
    if (boost::iequals(str_type, XML_OPT_AND))
        return GGATreeNode::AND;
    else if (boost::iequals(str_type, XML_OPT_OR))
        return GGATreeNode::OR;

    throw GGAXMLParseException(
        "[getNodeType] Invalid node type value: " + str_type);
}


static GGAParameter::pointer createParameter(const std::string& name,
    const std::string& prefix, const std::string& start,
    const std::string& end, const std::string& categories)
{
    if (!categories.empty() && start.empty() && end.empty()) {
        StringVector domain;
        CharSeparator sep(",");
        CharTokenizer tok(categories, sep);
        for (CharTokenizer::iterator it = tok.begin(); it != tok.end(); ++it)
            domain.push_back(*it);

        return GGAParameter::pointer(new GGAParameter(domain, name, prefix));

    // 
    } else if (categories.empty() && !start.empty() && !end.empty()) {
        GGAValue valStart = GGAValue::getValueFromStr(start);
        GGAValue valEnd = GGAValue::getValueFromStr(end);
        
        if (valStart.isDouble() || valEnd.isDouble()) {
            valStart = GGAValue::getValueFromStr(start, GGAValue::DOUBLE);
            valEnd = GGAValue::getValueFromStr(end, GGAValue::DOUBLE);
        }

        GGAParameter::Type paramtype = GGAParameter::UNKNOWN;
        if (valStart.isDouble())
            paramtype = GGAParameter::CONTINUOUS;
        else
            paramtype = GGAParameter::DISCRETE;

        return GGAParameter::pointer(new GGAParameter(paramtype, valStart,
            valEnd, name, prefix));

    // FLAG
    } else if (categories.empty() && start.empty() && end.empty()) {
        GGAParameter::pointer param(new GGAParameter(GGAParameter::CONTINUOUS,
            GGAValue(0L), GGAValue(1L), name, prefix));
        param->isFlag(true);
        return param;

    } else {
        throw GGAXMLParseException("[createParameter] Invalid configuration for"
            " node '" + name + "'.");
    }
}

// =======================
// ===== GGATreeNode =====
// =======================

/**
 *
 */
GGATreeNode::GGATreeNode()
    : m_type(AND)
    , m_param(static_cast<GGAParameter*>(NULL))
    , m_parent(static_cast<GGATreeNode*>(NULL))
    , m_children()
    , m_const_children()
{ }


/**
 *
 */
GGATreeNode::GGATreeNode(GGATreeNode::NodeType type, GGATreeNode* parent,
                         GGAParameter* param)
    : m_type(type)
    , m_param(param)
    , m_parent(parent)
    , m_children()    
    , m_const_children()
{
    if(m_parent != NULL)
        m_parent->addChild(pointer(this));
}


/**
 *
 */
GGATreeNode::~GGATreeNode()
{ }


/**
 *
 */
std::string GGATreeNode::str()
{
    std::stringstream ss;
    ss << "[GGATreeNode(" << this << "); " << m_param->info() 
       << ", m_parent(" << m_parent << "), children(";
    std::vector<GGATreeNode::pointer>::iterator itr;

    for(itr = m_children.begin(); itr != m_children.end(); ++itr) {
        ss << *itr;
        if(itr + 1 != m_children.end())
            ss << ",";
    }
    ss << ")]";

    return ss.str();
}


// ============================
// ===== GGAParameterTree =====
// ============================


//==============================================================================
// GGAParameterTree private static variables

GGAParameterTree* GGAParameterTree::s_pInstance = NULL;


//==============================================================================
// GGAParameterTree public static methods


//==============================================================================
// GGAParameterTree public methods

/**
 *
 */
GGAParameterTree::~GGAParameterTree() 
{
    delete m_cmd;
}


/*
 * Parses a parameter tree file.
 */
void GGAParameterTree::parseTreeFile(const std::string& filename) 
{
    try {
        boost::property_tree::iptree pt; // case-insensitive
        boost::property_tree::read_xml(filename, pt,
            boost::property_tree::xml_parser::no_comments);

        buildTree(pt);
    } catch (boost::property_tree::xml_parser_error& e) {
        throw GGAXMLParseException(e.what());
    }
   
    if (!m_cmd->validateVarNames(m_parameters))
        throw GGAXMLParseException("Command contains a variable that is not "
                                "defined in the parameters map!");

    validateSeededGenomes();
    propagateOrPaths();
}


/**
 *
 */
std::string GGAParameterTree::toString() const
{
    std::stringstream ss;
    ss << "[Root]" << m_root->str() << std::endl;
    ss << strParamTreeHelper(m_root);
    return ss.str();
}


//==============================================================================
// GGAParameterTree private methods

/**
 *
 */
GGAParameterTree::GGAParameterTree() 
    : m_root(static_cast<GGATreeNode*>(NULL))
    , m_cmd(NULL)
    , m_errRegEx("")
    , m_resRegEx("")
    , m_parameters()
    , m_seededGenomes()
    , m_paramnames()
    , m_forbidden()
{ }


/**
 * Builds the GGAParameterTree from the XML 
 */
void GGAParameterTree::buildTree(const boost::property_tree::iptree& pt)
{
    if (pt.count(XML_ROOT) != 1)
        throw GGAXMLParseException("[GGAParameterTree::buildTree]"
            "There must be exactly one '" + XML_ROOT + "' tag (root).");

    using boost::property_tree::iptree;
    const iptree& root = pt.get_child(XML_ROOT);
    buildTreeCommand(root);
    buildTreeErrRegex(root);
    buildTreeResRegex(root);
    buildTreeNode(root);

    BOOST_FOREACH (iptree::value_type const& v, root) {
        if (boost::iequals(v.first, XML_SEEDGENOME)) 
            m_seededGenomes.push_back(parseSeededGenome(v.second));
        if (boost::iequals(v.first, XML_FORBIDDEN))
            parseForbiddenBlocks(v.second);
    }    
}

/**
 *
 */
void GGAParameterTree::buildTreeCommand(const boost::property_tree::iptree& pt)
{
    if (pt.count(XML_CMD) == 0) // Mandatory
        throw GGAXMLParseException("[GGAParameterTree::buildTreeCommand]"
            " Missing command tag: " + XML_CMD);
    if (pt.count(XML_CMD) > 1) // Unique
        throw GGAXMLParseException("[GGAParameterTree::buildTreeCommand] There can"
            " only be one '" + XML_CMD + "' tag in the whole xml file.");

    m_cmd = new GGACommand(pt.get<std::string>(XML_CMD));
}


/**
 *
 */
void GGAParameterTree::buildTreeErrRegex(const boost::property_tree::iptree& pt)
{   // Mandatory?
    using boost::property_tree::iptree;

    if (pt.count(XML_ERR_REGEX) == 0)
        return;
    if (pt.count(XML_ERR_REGEX) > 1)
        throw GGAXMLParseException("[GGAParameterTree::buildTreeErrRegex] There can"
            " only be one '" + XML_ERR_REGEX + "' tag in the whole xml file.");

    m_errRegEx = pt.get<std::string>(XML_ERR_REGEX);
}


/**
 *
 */
void GGAParameterTree::buildTreeResRegex(const boost::property_tree::iptree& pt)
{   // Mandatory?
    using boost::property_tree::iptree;

    if (pt.count(XML_RES_REGEX) == 0)
        return;
    if (pt.count(XML_RES_REGEX) > 1)
        throw GGAXMLParseException("[GGAParameterTree::buildTreeResRegex] There can"
            " only be one '" + XML_RES_REGEX + "' tag in the whole xml file.");
    
    m_resRegEx = pt.get<std::string>(XML_RES_REGEX);
}


/**
 *
 */
void GGAParameterTree::buildTreeNode(const boost::property_tree::iptree& pt)
{   
    using boost::property_tree::iptree;
    if (pt.count(XML_NODE) != 1)
        throw GGAXMLParseException("[GGAParameterTree::buildTreeNodes] There must"
            "be exactly one '" + XML_NODE + "' tag under '" + XML_ROOT + "'");

    m_root = parseNodes(pt.get_child(XML_NODE));
}


/**
 * Parameter: pt = <seedgenome> node
 * Parses a seeded genome, that is, a mapping of variable names to values
 * that define a single genome.
 * XML Format:
 * <seedgenome>
 *  <variable name="varname" value="value" />
 *  ...
 * </seedgenome>
 */
GenomeMap GGAParameterTree::parseSeededGenome(
    const boost::property_tree::iptree& pt)
{
    using boost::property_tree::iptree;
    GenomeMap genome;

    if (pt.size() != pt.count(XML_SG_VARIABLE))
        throw GGAXMLParseException("[GGAParameterTree::parseSeededGenome] '" +
            XML_SEEDGENOME + "'' can only contain '" + XML_SG_VARIABLE + "'"
            " nodes.");

    BOOST_FOREACH (iptree::value_type const& v, pt) {
        if (v.second.count(XML_ATTR) != 1)
            throw GGAXMLParseException("[GGAParameterTree::parseSeededGenome] Tag" 
                " '" + XML_SG_VARIABLE + "' must have attributes.");        
        const iptree& v_attr = v.second.get_child(XML_ATTR);

        if (v_attr.count(XML_ATTR_SG_NAME) != 1)
            throw GGAXMLParseException("[GGAParameterTree::parseSeededGenome]"
                " Variable tag has to contain exactly one 'name' tag.");
        if (v_attr.count(XML_ATTR_SG_VALUE) != 1)
            throw GGAXMLParseException("[GGAParameterTree::parseSeededGenome]"
                " Variable tag has to contain exactly one 'value' tag.");

        std::string name = v_attr.get<std::string>(XML_ATTR_SG_NAME);
        std::string value = v_attr.get<std::string>(XML_ATTR_SG_VALUE);

        genome[name] = GGAValue::getValueFromStr(value);
    }

    return genome;
}


/**
 * Parses forbidden combinations of parameters into the m_forbidden member
 * variable.
 *
 * @param pt = <forbidden> ... </forbidden>
 */
void GGAParameterTree::parseForbiddenBlocks(
    const boost::property_tree::iptree& pt) 
{
    using boost::property_tree::iptree;
    BOOST_FOREACH (iptree::value_type const& v, pt)
    {
        if (!boost::iequals(v.first, XML_FORBIDDEN_BLOCK))
            throw GGAXMLParseException("[GGAParameterTree::parseForbiddenBlocks] '"
                + XML_FORBIDDEN + "'' can only contain '" 
                + XML_FORBIDDEN_BLOCK + "' nodes.");

        parseForbiddenBlock(v.second);        
    }
}


/**
 * Parses a forbidden combination of parameters into the m_forbidden member
 * variable.
 *
 * @param pt = <forbid> ... </forbid>
 */
void GGAParameterTree::parseForbiddenBlock(
    const boost::property_tree::iptree& pt)
{
    using boost::property_tree::iptree;
    GenomeMap forbidgm;

    BOOST_FOREACH (iptree::value_type const& v, pt) {
        if (!boost::iequals(v.first, XML_FORBIDDEN_SETTING))
            throw GGAXMLParseException(
                "[GGAParameterTree::parseForbiddenBlocks] '"
                + XML_FORBIDDEN_BLOCK + "'' can only contain '"
                + XML_FORBIDDEN_SETTING + "' nodes.");    

        if (v.second.count(XML_ATTR) != 1)
            throw GGAXMLParseException("[GGAParameterTree::parseNode] Tag '" 
                + XML_FORBIDDEN_SETTING + "' must have attributes.");
     
        const iptree& pt_attr = v.second.get_child(XML_ATTR);

        if (pt_attr.count(XML_ATTR_NAME) != 1)
            throw GGAXMLParseException(
                "[GGAParameterTree::parseForbiddenCombo]"
                + XML_FORBIDDEN_SETTING + " tag, has to contain excatly one"
                " 'name' tag.");

        if (pt_attr.count(XML_ATTR_SG_VALUE) != 1)
            throw GGAXMLParseException("[GGAParameterTree::parseForbiddenCombo]"
                + XML_FORBIDDEN_SETTING + " tag, has to contain excatly one"
                " 'value' tag.");

        std::string name = pt_attr.get<std::string>(XML_ATTR_SG_NAME);
        std::string value = pt_attr.get<std::string>(XML_ATTR_SG_VALUE);
        
        if (m_parameters[name]->type() == GGAParameter::CATEGORICAL)
            forbidgm[name] = GGAValue(value);
        else
            forbidgm[name] = GGAValue::getValueFromStr(value);
    }
    m_forbidden.push_back(forbidgm);
}

/**
 * Parses all the node tags and generates a GGATreeNode tree.
 *
 * @param pt The unique <node> ... </node> under the document root.
 */
GGATreeNode::pointer GGAParameterTree::parseNodes(
    const boost::property_tree::iptree& pt)
{
    return parseNode(pt, GGATreeNode::pointer(static_cast<GGATreeNode*>(NULL)));
}


/**
 * Handles an individual <node> tag, parsing its attributes
 * and calling parseNode on its children
 */
GGATreeNode::pointer GGAParameterTree::parseNode(
    const boost::property_tree::iptree& pt, const GGATreeNode::pointer parent)
{
    if (pt.count(XML_ATTR) != 1)
        throw GGAXMLParseException("[GGAParameterTree::parseNode] Tag '" 
            + XML_NODE + "' must have attributes.");
    const boost::property_tree::iptree& pt_attr = pt.get_child(XML_ATTR);

    GGATreeNode::pointer node(new GGATreeNode());
    std::string name, trajname, valStart, valEnd, categories, prefix;

    if (pt_attr.count(XML_ATTR_TYPE) != 1) {
        throw GGAXMLParseException(
            "[GGAParameterTree::parseNode] A node must have exactly one type.");
    } else {
        node->type(getNodeType(pt_attr.get<std::string>(XML_ATTR_TYPE)));        
    }

    if (pt_attr.count(XML_ATTR_NAME)) {
        name = pt_attr.get<std::string>(XML_ATTR_NAME);
        if (!isNameValid(name))
            throw GGAXMLParseException(
                "[GGAParameterTree::parseNode] Invalid node name: " + name);
    } else {
        name = GGAParameter::genUniqueVarName();        
    }

    if (pt_attr.count(XML_ATTR_TRAJNAME))
        trajname = pt_attr.get<std::string>(XML_ATTR_TRAJNAME);
    else
        trajname = name;

    if (pt_attr.count(XML_ATTR_START))
        valStart = pt_attr.get<std::string>(XML_ATTR_START);
    if (pt_attr.count(XML_ATTR_END))
        valEnd = pt_attr.get<std::string>(XML_ATTR_END);
    if (pt_attr.count(XML_ATTR_CATEGORIES))
        categories = pt_attr.get<std::string>(XML_ATTR_CATEGORIES);

    if (pt_attr.count(XML_ATTR_PREFIX))
        prefix = pt_attr.get<std::string>(XML_ATTR_PREFIX);

    GGAParameter::pointer param = createParameter(name, prefix, valStart,
        valEnd, categories);

    m_parameters[name] = param;
    param->trajName(trajname);
    node->parameter(param);
    node->parent(parent);

    // Parse children <node>
    using boost::property_tree::iptree;
    BOOST_FOREACH (iptree::value_type const& v, pt) {
        if (boost::iequals(v.first, XML_NODE))
            parseNode(v.second, node);
        else if (!boost::iequals(v.first, XML_ATTR))
            throw GGAXMLParseException("[GGAParameterTree::parseNode] '" 
                + XML_NODE + "' tag can only have '" + XML_NODE + "' tags as"
                " subnodes.");
    }

    return node;
}


/**
 *
 */
void GGAParameterTree::validateSeededGenomes()
{
    std::vector<GenomeMap> replaceMap;
    std::vector<GenomeMap>::iterator sgItr;
    int count = 1;
    for(sgItr = m_seededGenomes.begin(); sgItr != m_seededGenomes.end(); ++sgItr) {
        GenomeMap& sGenome = *sgItr;
        ParameterMap::iterator paramItr;

        for(paramItr = m_parameters.begin(); paramItr != m_parameters.end(); ++paramItr) {
            bool resetParam = false;
            GGAParameter::pointer param = paramItr->second; 

            if(sGenome.find(paramItr->first) == sGenome.end()) {
                LOG_ERROR("Warning: Parameter " << paramItr->first 
                        << " was not found in seeded genome #" << count 
                        << ". Setting to a random value in its range.");
				sGenome[paramItr->first] = param->valueInRange();
            } else {
                // Ensure that the categorical parameter in the seeded genome is a string
                if(param->type() == GGAParameter::CATEGORICAL) {
                    GGAValue& oldValue = sGenome[paramItr->first];
                    if(!oldValue.isString()) {
                        GGAValue newValue(oldValue.toString());
                        sGenome[paramItr->first] = newValue;
                    }
                }
                if(!param->isValueInRange(sGenome[paramItr->first])) {
                    LOG_ERROR("Warning: Parameter " << paramItr->first 
                            << " is not in its specified range. Setting to "
                            << "a random value in its range.");
                    LOG_ERROR(sGenome[paramItr->first]);
                    LOG_ERROR(param->rangeStart().getDouble());
                    LOG_ERROR(param->rangeEnd().getDouble());
                    resetParam = true;
                }
            }
            if(resetParam) {
                sGenome[paramItr->first] = param->valueInRange();
            }
        }
        replaceMap.push_back(sGenome);
        ++count;
    }
    m_seededGenomes = replaceMap;
}


/**
 * Validates a node name checking whether it is duplicated or contains invalid characters
 */
bool GGAParameterTree::isNameValid(const std::string& name) 
{
    bool retval = true;
    try {
        // Check for disallowed characters
        for(unsigned int i = 0; i < BAD_CHARS.size(); ++i) {
            if(name.find(BAD_CHARS[i]) != std::string::npos) {
                LOG_ERROR("Invalid character (\"" << BAD_CHARS[i] 
                        << "\") found in name \"" << name << "\".");
                retval = false;
            }
        }
        StringVector pNames = GGACommand::protectedNames();
        if(std::find(pNames.begin(), pNames.end(), name) != pNames.end()) {
           LOG_ERROR("Invalid variable name \"" << name << "\". \"" 
                    << name << "\" is a protected variable name. "
                    << "Please choose a different one.");
            retval = false;
        }
        // Ensure the name is unique
        if(retval && m_parameters.find(name) != m_parameters.end()) {
            LOG_ERROR("Variable name \"" << name 
                    << "\" already exists! Please ensure all names are unique.");
            retval = false;
        }
    } catch(...) {
        LOG_ERROR("Unknown exception when parsing node name " << name);
        retval = false;
    }
    return retval;
}


/**
 * Sets GGAParameter or-path settings so that variables are correctly
 * displayed or hidden in the GGACommand
 */
void GGAParameterTree::propagateOrPaths() 
{
    GGATreeNode::pointer cur = m_root;
    GGAParameter::pointer orParent(static_cast<GGAParameter*>(NULL));
    bool selfOr = false;

    std::vector<GGATreeNode::pointer> nodes;
    nodes.push_back(cur);

    while(!nodes.empty()) {
        cur = nodes.back();
        nodes.pop_back();
        GGAParameter::pointer curParam = cur->parameter();
        selfOr = false;

        if(cur->type() == GGATreeNode::OR) {
            orParent = cur->parameter();
            selfOr = true;
        } else {
            orParent = curParam->orParent();
        }

        long i = 0;
        std::vector<GGATreeNode::pointer> children = cur->children();        
        std::vector<GGATreeNode::pointer>::iterator itr;
        for(itr = children.begin(); itr != children.end(); ++itr, ++i) {
            GGAParameter::pointer childParam = (*itr)->parameter();
            childParam->orParent(orParent);

            if(selfOr) {

                if(curParam->type() == GGAParameter::CATEGORICAL) {
                    childParam->orValue(
                        GGAValue(curParam->categoricalDomain()[i]));
                } else if(curParam->type() == GGAParameter::DISCRETE) {
                    long start = curParam->rangeStart().getLong();
                    childParam->orValue(GGAValue(start + i));
                } else {
                    childParam->orValue(GGAValue());
                }
                
            } else {
                childParam->orParent(curParam->orParent());
                childParam->orValue(curParam->orValue());
            }
            nodes.push_back(*itr);
        }
        LOG(curParam->info());
    }
}


/**
 *
 */
std::string GGAParameterTree::strParamTreeHelper(GGATreeNode::pointer cur) const
{
    if(cur == NULL) {
        LOG_ERROR("Error: Parameter tree contains a NULL pointer.");
        return "";
    }
    std::stringstream ss;
    std::vector<GGATreeNode::pointer>::const_iterator itr;
    for(itr = cur->children().begin(); itr != cur->children().end(); ++itr) {
        ss << (*itr)->str() << std::endl;
        ss << strParamTreeHelper(*itr);
    }
    return ss.str();
}



