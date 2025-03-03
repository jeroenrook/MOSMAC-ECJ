#!/usr/bin/python

#
# paramils_convert.py
# Author: Kevin Tierney
# Version: 1.1.0
# Date: April 2, 2014
#

# Python script to convert the parameter file for ParamILS/SMAC into the XML
# file for GGA. Note that the handling of conditions in GGA is not so great --
# conditions with multiple values (e.g. param1 | param2 in {a,b,c} ) will
# result in multiple copies of the param1 variable. This cannot be avoided in
# an automatic conversion. The best way to handle this would be to deal with it
# in the wrapper by not passing param2 through when param1 is invalid.
#
# Note: I am assuming that all parameters have default values. If they do not,
# delete <seedgenome> </seedgenome> and everything in between.
#
# Note 2: There's a good chance this converter cannot handle complex conditionals.
#

import sys
import os.path
import io

stderr = False
if len(sys.argv) != 2:
    stderr = True

fexists = os.path.exists(sys.argv[1])

if stderr or not fexists:
    sys.stderr.write("Usage: ./paramils_convert.py <input ParamILS/SMAC file>\n")
    sys.stderr.write("Output is printed on STDOUT\n")
    if fexists:
        sys.stderr.write("File: %s not found." % sys.argv[1])
    sys.exit(1)

import re
import copy

class Parameter:
    def __init__(self):
        self.name = ''
        self.origName = None
        self.prefixName = ''
        self.rangeStart = None
        self.rangeEnd = None
        self.isInteger = False
        self.useScientific = False
        self.categories = None
        self.default = None
        self.orvalues = {} # Only receives values if the node is an or node; stores a list of the nodes depending on each value
        self.parent = None
        self.children = []

#     def copy(self, pother):
#         self.name = pother.name
#         self.prefixName = pother.prefixName
#         self.rangeStart = pother.rangeStart
#         self.rangeEnd = pother.rangeEnd
#         self.isInteger = pother.isInteger
#         self.useScientific = pother.useScientific
#         self.categories = pother.categories
#         self.default = pother.default
#         self.orvalues = pother.orvalues
#         self.children = pother.children

    def __deepcopy__(self, memo):
        retobj = Parameter()
        retobj.__dict__.update(self.__dict__)
        retobj.name = copy.deepcopy(self.name, memo)
        retobj.prefixName = copy.deepcopy(self.prefixName, memo)
        retobj.rangeStart = copy.deepcopy(self.rangeStart, memo)
        retobj.rangeEnd = copy.deepcopy(self.rangeEnd, memo)
        retobj.isInteger = copy.deepcopy(self.isInteger, memo)
        retobj.useScientific = copy.deepcopy(self.useScientific, memo)
        retobj.categories = copy.deepcopy(self.categories, memo)
        retobj.default = copy.deepcopy(self.default, memo)
        retobj.orvalues = copy.deepcopy(self.orvalues, memo)
        retobj.children = copy.deepcopy(self.children, memo)
        retobj.parent = copy.deepcopy(self.parent, memo)
        return retobj

    def toStr(self):
        if self.categories:
            return "%s {%s}[%s]" % (self.name, ",".join(self.categories), self.default)
        else:
            if self.isInteger:
                return "%s [%d,%d][%d]i" % (self.name, self.rangeStart, self.rangeEnd, self.default)
            else:
                return "%s [%f,%f][%f]" % (self.name, self.rangeStart, self.rangeEnd, self.default)

categoricalRe = re.compile(r'^(\S+)\s*{(.*)}\s*\[(.*)\].*$')
rangeRe = re.compile(r'^(\S+)\s*\[(.+),(.+)\]\s*\[(.*)\]([il]*).*$') 
condRe = re.compile(r'^(\S+)\s*\|\s*(\S+)\s+in\s+{(.*)}.*$')
forbidRe = re.compile(r'^\s*\{(.*)\}\s*$')

usedInOr = []

def categoryClean(cc):
    return cc.strip().replace("\"", "&quot;")

def commentStrip(cc):
    sp = cc.split("#")
    return sp[0].strip()

vnre = re.compile(r'^extra\d$')
def validName(nn):
    return not (nn in ["seed","cutoff","instance"] or vnre.match(nn))

params = {} # name -> param
forbidden = []
readParams = True
ii = 0
with open(sys.argv[1]) as fp:
    for line in fp:
        ii += 1
        line = commentStrip(line)
        if line.strip() == '' or line.startswith('#'):
            continue
        if "Conditionals" in line:
            readParams = False
            continue
        if readParams:
            if "|" in line:
                readParams = False
                sys.stderr.write("Warning: File does not declare start of conditionals section; assuming it starts at the first line with a pipe (line {0}).\n".format(ii))
                continue
            elif "{" in line:
                ccre = categoricalRe.match(line)
                if ccre:
                    if not validName(ccre.groups()[0]):
                        sys.stderr.write("Warning (line %d): Parameter name \"%s\" is reserved; ignoring. \n" % (ii, ccre.groups()[0]))
                        continue
                    pp = Parameter()
                    pp.name = ccre.groups()[0]
                    pp.prefixName = ccre.groups()[0]
                    pp.categories = ccre.groups()[1].split(",")
                    # Remove the next line if leading/ending whitespace is necessary
                    pp.categories = list(map(categoryClean, pp.categories))
                    seedVal = ccre.groups()[2]
                    if pp.categories:
                        seedVal = categoryClean(seedVal)
                    pp.default = seedVal
                    params[pp.name] = pp
                else:
                    sys.stderr.write("Warning (line %d): Unable to parse line as categorical: %s\n" % (ii, line.strip()))
            else:
                rre = rangeRe.match(line)
                if rre:
                    if not validName(rre.groups()[0]):
                        sys.stderr.write("Warning (line %d): Parameter name \"%s\" is reserved; ignoring. \n" % (ii, rre.groups()[0]))
                        continue
                    pp = Parameter()
                    pp.name = rre.groups()[0]
                    pp.prefixName = rre.groups()[0]
                    ilgrp = rre.groups()[4]
                    if not ilgrp or ilgrp == 'l': # Note that the "log-ness" of a parameter is ignored
                        if ilgrp == 'l':
                            sys.stderr.write("Warning (line %d): Ignoring log request.\n" % (ii))
                        pp.rangeStart = float(rre.groups()[1])
                        pp.rangeEnd = float(rre.groups()[2])
                        pp.default = float(rre.groups()[3])
                        if "e" in rre.groups()[1] or "e" in rre.groups()[2]:
                            pp.useScientific = True
                    else: # integer
                        pp.isInteger = True
                        pp.rangeStart = int(rre.groups()[1])
                        pp.rangeEnd = int(rre.groups()[2])
                        rangeSize = abs(pp.rangeEnd - pp.rangeStart)
                        if rangeSize > 2147483647:
                            sys.stderr.write("Warning (line %d): Range for parameter %s too large, reducing range end to fit within maximum range.\n" % (ii, pp.name))
                            pp.rangeEnd -= abs(rangeSize - 2147483647) + 1
                        pp.default = min(int(rre.groups()[3]), pp.rangeEnd)
                    params[pp.name] = pp
                else:
                    sys.stderr.write("Warning (line %d): Unable to parse line as range: %s\n" % (ii, line.strip()))
        else:
            core = condRe.match(line)
            if core:
                child = core.groups()[0]
                parent = core.groups()[1]
                orvalues = core.groups()[2].split(',')
                if child not in params:
                    sys.stderr.write("Warning (line %d): No parameter with name %s.\n" % (ii, child))
                    continue
                if parent not in params:
                    sys.stderr.write("Warning (line %d): No parameter with name %s.\n" % (ii, parent))
                    continue
                if len(orvalues) > 1:
                    sys.stderr.write("Warning (line %d): Multiple conditional values will result in multiple copies of the same parameter.\n" % ii)
                for jj, orval in enumerate(orvalues):
                    useParam = params[child]
                    if jj >= 1:
                        ppCopy = copy.deepcopy(useParam) 
#                         ppCopy = Parameter()
#                         ppCopy.copy(pp)
                        ppCopy.origName = ppCopy.name
                        ppCopy.name = "%s_%d" % (ppCopy.name, jj)
                        useParam = ppCopy
                        params[ppCopy.name] = ppCopy
                        useParam.parent = None

                    if useParam.parent: # we already assigned this particular parameter to part of the tree
                        useParam.parent.children.remove(useParam) # this really only handles the case where we assign higher up the tree first... TODO

                    parentParam = params[parent]
                    parentParam.orvalues.setdefault(orval, []).append(useParam)
                    usedInOr.append(useParam.name)
                    parentParam.children.append(useParam)
                    useParam.parent = parentParam
            else:
                ffmm = forbidRe.match(line)
                if ffmm:
                    fspl = ffmm.groups()[0].split(",")
                    forbid_map = {}
                    forbid_valid = True
                    for fsp in fspl:
                        eqsp = fsp.split("=")
                        pname = eqsp[0].strip()
                        pval = eqsp[1]
                        if params[pname].categories:
                            pval = categoryClean(pval)
                            if pval not in params[pname].categories:
                                sys.stderr.write("Warning (line %d): Forbidden setting %s not found in the categories of parameter %s.\n" % (ii, pval, pname))
                                forbid_valid = False
                                break
                        else:
                            fpval = float(pval) # doesn't need to be an int for range checking
                            if fpval < params[pname].rangeStart or fpval > params[pname].rangeEnd:
                                sys.stderr.write("Warning (line %d): Forbidden setting %s not found in the range of parameter %s.\n" % (ii, pval, pname))
                                forbid_valid = False
                                break
                        forbid_map[pname] = pval
                    if forbid_valid:
                        forbidden.append(forbid_map)
                else:
                    sys.stderr.write("Warning (line %d): Unable to parse line %s\n" % (ii, line.strip()))

# ParamILS/SMAC file loaded, print GGA XML file to stdout

# for cc in params["sp-resolution"].children:
#     print cc.name

numDummies = 0
alreadyPrinted = {} # kludge; prevents multiple printing of nodes TODO fix this

def printParam(paramOut, prefix, param):
    global numDummies
    global alreadyPrinted
    if alreadyPrinted.get(param.name, False):
        return
    else:
        alreadyPrinted[param.name] = True
    paramOut.write("%s<node type=\"" % prefix)
    if len(param.orvalues) == 0:
        paramOut.write('and')
    else:
        paramOut.write('or')
    if param.name != 'root':
        paramOut.write("\" name=\"%s\" prefix=\"-%s \" " % (param.name, param.prefixName))
        if param.origName:
            paramOut.write("trajname=\"%s\" " % param.origName)
    else:
        paramOut.write('" name="__dummy__root" ')
        
    if param.categories:
        paramOut.write("categories=\"%s\" " % (",".join(param.categories)))
    elif param.isInteger:
        paramOut.write("start=\"%d\" end=\"%d\" " % (param.rangeStart, param.rangeEnd))
    elif param.useScientific:
        paramOut.write("start=\"%e\" end=\"%e\" " % (param.rangeStart, param.rangeEnd))
    else:
        paramOut.write("start=\"%f\" end=\"%f\" " % (param.rangeStart, param.rangeEnd))
    if len(param.children) == 0:
        print("/>", file=paramOut)
    else:
        print(">", file=paramOut)
        if param.orvalues:
            for val in param.categories:
                if val not in param.orvalues:
    #                 pp = Parameter()
    #                 pp.name = "__dummy__%d" % numDummies
    #                 pp.rangeStart = 0
    #                 pp.rangeEnd = 0
    #                 pp.isInteger = True
    #                 params[pp.name] = pp
                    print("\t%s<node type=\"and\" name=\"__dummy__%d\" start=\"0\" end=\"0\" />" % (prefix, numDummies), file=paramOut)
                    numDummies += 1
                else:
                    orvs = param.orvalues[val]
                    if len(orvs) == 1:
                        printParam(paramOut, prefix + "\t", orvs[0])
                    else:
                        print("\t%s<node type=\"and\" name=\"__dummy__%d\" start=\"0\" end=\"0\">" % (prefix, numDummies), file=paramOut)
                        numDummies += 1
                        for orv in  orvs:
                            printParam(paramOut, prefix + "\t\t", orv)
                        print("\t%s</node>" % prefix, file=paramOut)
        else:
            for cc in param.children:
                printParam(paramOut, prefix + "\t", cc)
        print("%s</node>" % prefix, file=paramOut)

print("<algtune>")
print("<!-- Note: this command is meant for the CSSC; that is, it assumes a wrapper will be specified in a scenario file with the algo parameter. If this is not the case, please prepend your solver to the following command, and remove '$extra0 $cutoff 92233720 ' -->")
print("\t<cmd>$instance $extra0 $cutoff 92233720 $seed %s</cmd>" % (" ".join(["$%s" % x for x in [name for name in list(params.keys())]])))

usedInOrDefaults = {}
for uio in usedInOr:
    if uio in params:
        if params[uio].categories:
            usedInOrDefaults[uio] = params[uio].default
        elif params[uio].isInteger:
            usedInOrDefaults[uio] = "%d" % params[uio].default
        elif params[uio].useScientific:
            usedInOrDefaults[uio] = "%e" % params[uio].default
        else:
            usedInOrDefaults[uio] = "%f" % params[uio].default
        del params[uio]

print("\t<seedgenome>")
print('\t\t<variable name="__dummy__root" value="0" />')
for name, param in sorted(params.items()):
    sys.stdout.write("\t<variable name=\"%s\" value=\"" % name)
    if param.categories:
        sys.stdout.write(param.default)
    elif param.isInteger:
        sys.stdout.write("%d" % param.default)
    elif param.useScientific:
        sys.stdout.write("%e" % param.default)
    else:
        sys.stdout.write("%f" % param.default)
    print('\" />')
for uio in usedInOr:
    print("\t\t<variable name=\"%s\" value=\"%s\" />" % (uio, usedInOrDefaults[uio]))

root = Parameter()
root.name = 'root'
root.rangeStart = 0
root.rangeEnd = 0
root.default = 0
root.isInteger = True
root.children = list(params.values())

paramout = io.StringIO()
printParam(paramout, "\t", root)

for ii in range(numDummies):
    print("\t\t<variable name=\"__dummy__%i\" value=\"0\" />" % (ii))
print("\t</seedgenome>")

print(paramout.getvalue())
paramout.close()

if len(forbidden) > 0:
    print("\t<forbidden>")
    for fmap in forbidden:
        print("\t\t<forbid>")
        for pname, pval in sorted(fmap.items()):
            print("\t\t\t<setting name=\"%s\" value=\"%s\" />" % (pname, pval))
        print("\t\t</forbid>")
    print("\t</forbidden>")

# <forbidden>
#   <forbid>
#       <setting name="<param name>" value="<forbidden value>" />
#       ...
#   </forbid>
#   ...
# </forbidden>

print("</algtune>")

