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

#include <cstdlib>

#include <algorithm>
#include <sstream>
#include <string>

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "ggatypedefs.hpp"
#include "GGACommand.hpp"
#include "GGAExceptions.hpp"
#include "GGAOptions.hpp"
#include "GGAParameter.hpp"
#include "GGAUtil.hpp"
#include "OutputLog.hpp"

/* Local typedefs */
typedef boost::char_separator<char> CharSeparator;
typedef boost::tokenizer<CharSeparator> CharTokenizer;

//==============================================================================
// GGACommand private static variables

StringVector* GGACommand::s_protectedNames = NULL;

//==============================================================================
// GGACommand public static methods

/**
 *
 */
StringVector GGACommand::protectedNames() {
    if(s_protectedNames == NULL) {
        s_protectedNames = new StringVector();
        s_protectedNames->push_back("seed");
        s_protectedNames->push_back("instance");
        s_protectedNames->push_back("cutoff");
        s_protectedNames->push_back("extra0");
        s_protectedNames->push_back("extra1");
        s_protectedNames->push_back("extra2");
        s_protectedNames->push_back("extra3");
    }
    return *s_protectedNames;
}

//==============================================================================
// GGACommand public methods

/*
 * Parses a string into a list of objects that allow fast insertion
 * of variables into the string.
 * Warning: Variable names are not automatically checked to see if they were defined
 * in the parameter tree. Call 
 * bool validateVarNames(std::vector<GGAParameter*> params)
 * for validation.
 */
GGACommand::GGACommand(const std::string& cmd) 
    : m_execCommand()
    , m_rawCommand()
    , m_commandArgs()
    , m_parseSuccessful(false)
{
    // If a scenario file was specified and the "algo" param passed in, we must prepend this.
    const GGAOptions& opts = GGAOptions::instance();
    std::stringstream ss;
    ss << opts.nc_prepend_cmd << " " << boost::trim_copy(cmd);
    m_rawCommand = ss.str();
    
    parseCommand(m_rawCommand);
}


/**
 *
 */
GGACommand::~GGACommand() 
{
    delete s_protectedNames;
    s_protectedNames = NULL;
}


/**
 *
 */
bool GGACommand::isOrPathOk(const GGAParameter::pointer param, 
                            const GenomeMap& genome) const
{
    if(param.get() == NULL)
        return true;

    bool orPathOk = true;
    GGAParameter::pointer orParent = param->orParent();

    if(orParent.get() != NULL && param->orValue().hasValue())
        orPathOk = mapAt(genome, orParent->name()) == param->orValue();

    return orPathOk && isOrPathOk(param->orParent(), genome);
}


/**
 * Tests whether all the variables of this command are defined or not in the
 * given parameters map.
 * @return true if all the variables are defined in the parameters map, false
 *         otherwise.
 */
bool GGACommand::validateVarNames(const ParameterMap& params)
{
    bool retval = true;
    StringVector pNames = GGACommand::protectedNames();
    CommandArgVector::const_iterator itr;

    for(itr = m_commandArgs.begin(); itr != m_commandArgs.end(); ++itr) {
        if(itr->type() == GGACommandArg::VARIABLE) {
            std::string varName = itr->name();
            StringVector::iterator pnf = std::find(pNames.begin(), pNames.end(),
                                                   varName);

            if(params.find(varName) == params.end() && pnf == pNames.end()) {
                LOG_ERROR("Error: Command contains a variable \"" << varName << 
                          "\" that is not defined in the parameter map!");
                retval = false;
            }
        }
    }

    return retval;
}


// TODO Allow escaping of dollar signs so that they can be used in the command
/**
 * @throw GGACommandException
 */
void GGACommand::parseCommand(const std::string& cmd)
{
    std::string str_regex1("\\$\\s*(\\S+)");
    std::string str_regex2("\\$\\s*\\{\\s*(\\S+)\\s*\\}");

    boost::regex var_regex1(str_regex1);
    boost::regex var_regex2(str_regex2);
    boost::smatch match;

    CharSeparator separators(" \t\n\v");
    CharTokenizer tok(cmd, separators);
    size_t tok_count = std::distance(tok.begin(), tok.end());

    for (CharTokenizer::iterator it = tok.begin(); it != tok.end(); ++it) {
        std::string token = boost::trim_copy(*it);

        if (boost::regex_search(token, match, var_regex1) ||
            boost::regex_search(token, match, var_regex2)) 
        {
            std::string var_name(match[1].first, match[1].second);
            m_commandArgs.push_back(
                GGACommandArg(GGACommandArg::VARIABLE, var_name));
        } else if (token.find("$") == std::string::npos) {
            if (m_execCommand.empty())  // Executable name should be the first
                m_execCommand = token;  // FIXED argument

            m_commandArgs.push_back(
                    GGACommandArg(GGACommandArg::FIXED, token));
        } else {
            throw GGACommandException("The command token \"" + token + "\" does"
                "not match any of the command regex (" + str_regex1 + " | " + 
                str_regex2 + "), but contains the $ sign.");
        }

        tok_count -= 1; // Precomputed (performance reasons).
        if (tok_count > 0)
            m_commandArgs.push_back(GGACommandArg(GGACommandArg::FIXED, " "));
    }
}


/**
 *
 */
std::string GGACommand::str() const
{
    std::stringstream ss;
    ss << m_rawCommand << std::endl;
    std::vector<GGACommandArg>::const_iterator itr;

    for(itr = m_commandArgs.begin(); itr != m_commandArgs.end(); ++itr)
        ss << itr->str() << std::endl;

    return ss.str();
}


//==============================================================================
// GGACommand private methods

GGACommand::GGACommand() 
    : m_execCommand()
    , m_rawCommand()
    , m_commandArgs()
    , m_parseSuccessful(false)
{ }


// === GGACommandArg ===


/**
 *
 */
GGACommandArg::GGACommandArg() 
    : m_type(FIXED)
    , m_name()
    , m_fixedValue()
    , m_protected(false)
{ }


/**
 * Parameters:
 *  if type = FIXED, val is the fixed string value
 *  if type = VARIABLE, val is the name of the parameter
 */
GGACommandArg::GGACommandArg(ArgType type, const std::string& val) 
    : m_type(type)
    , m_name(type == VARIABLE ? val : "")
    , m_fixedValue(type == VARIABLE ? "" : val)
    , m_protected(false)
{
    if(type == VARIABLE) {
        StringVector pNames = GGACommand::protectedNames();
        m_protected = std::find(pNames.begin(), pNames.end(), val) 
                        != pNames.end();
    }
}


/**
 *
 */
GGACommandArg::GGACommandArg(const GGACommandArg& other)
    : m_type(other.m_type)
    , m_name(other.m_name)
    , m_fixedValue(other.m_fixedValue)
    , m_protected(other.m_protected)
{ }


/**
 *
 */
GGACommandArg::~GGACommandArg()
{ }


/**
 *
 */
std::string GGACommandArg::str() const
{
    std::stringstream ss;
    ss << "[GGACommandArg: Type: ";
    if(m_type == FIXED) {
        ss << "Fixed; Value: " << m_fixedValue;
    } else {
        ss << "Variable; Name: " << m_name;
    }
    ss << "]";
    return ss.str();
}
