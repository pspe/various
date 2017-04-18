#!/usr/bin/env python

import subprocess
import time
import re
import os
import ntpath
import sys
import cPickle as pickle
import getopt
import math

baseFiles = ""
baseFilesList = []

options, remainder = getopt.getopt (sys.argv[1:], 'hcl:snrtf:', ['help', 'create', 'symbols', 'libs=','needed','redundant','thin','filename='])

createNew = False
showSymbols = False
showNeeded = True
showRedundant = True
showThin = False
filename = "references"

for opt, arg in options:
    if opt in ('-h', '--help'):
        print """
        ./dependencies.py

analyzes the dependencies between shared libraries in the current directory.

Options:
  -h, --help      show the help text
  -c, --create    (re-)create the dependency analysis
  -s, --symbols   show each symbol in a separate edge
  -n, --needed    show needed links only
  -r, --redundant show redundant links only
  -t, --thin      draw thin edges regardless of the number of symbols which are referenced
  -l, --libs      show the dependencies for the libs provided here
  -f, --file      filename for the .dot output file which contains the graph information in the DOT format
"""
    elif opt in ('-c', '--create'):
        createNew = True
    elif opt in ('-l', '--libs'):
        baseFiles += arg+" "
        baseFiles = re.sub (' +',' ',baseFiles)
        baseFilesList.append (arg)
    elif opt in ('-s', '--symbols' ):
        showSymbols = True
    elif opt in ('-n', '--needed' ):
        showRedundant = False
    elif opt in ('-r', '--redundant' ):
        showNeeded = False
    elif opt in ('-t', '--thin' ):
        showThin = True
    elif opt in ('-f', '--filename' ):
        filename = arg

for arg in remainder:
    baseFiles += arg+" "
    baseFiles = re.sub (' +',' ',baseFiles)
    baseFilesList.append (arg)

if len(baseFiles) == 0:
    baseFiles = "*[^.msg]"

excludes = ['@@GLIBC','@@CXXABI']
excludes = [' | grep -v "%s"'%(s) for s in excludes]
excludes = " ".join (excludes)

cmdUndefined = 'nm %s -Coglu '%(baseFiles)+excludes
cmdDefined   = 'nm *[^.msg] -Cogl --defined-only '+excludes
cmdLibList   = "ls -1 %s"%(baseFiles)
cmdLibListAll   = 'ls -1 *[^.msg]'



def removePath (filename):
    return ntpath.basename (filename)


def shellExecute (cmd):
    print "command: ",cmd
    p = subprocess.Popen (cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, executable="/bin/bash")
    out, err = p.communicate ()
    while p.poll () is None:
        print ".",
        time.sleep (0.5)
    if len(err) > 0:
        print "error: ",err
    return out

## print shellExecute (["/bin/ls",os.getcwd()+"/*"])
## print shellExecute (["/usr/bin/nm",os.getcwd()+"/*"])


def extractUndefined (input):
    undef = input.split ("\n")

    undefined = {}

    for line in undef:
        tokens = re.sub (' +', ' ', line);
        tokens = tokens.split (" ", 2)
        if len(tokens) < 3:
            continue
        if tokens[1] != 'U':
            continue
        tokens[0] = tokens[0][:-1]

        sym = tokens[2]
        lib = tokens[0]
        lib = removePath (lib)

        if sym not in undefined.keys ():
            undefined[sym] = []
        undefined[sym].append (lib)
    return undefined


def extractDefined (inData):
    #pocl3knl:08738140 T TranslateStringArray::TranslateStringArray(PcsMutex*)	/home/developer/code/v7xx/obj/l3.debug/libpoc/../../../src/pocxxcom/omso.cpp:8148

    inData = inData.split ("\n")

    defined = {}

    for line in inData:
        tokens = line.split ("\t", 1)
        tokens[0] = re.sub (' +', ' ', tokens[0]);

        sub = tokens[0].split (" ", 2)
        subOne = sub[0].split (":", 1)

        if len (sub) < 3:
            continue
        symbol = sub[2]
        symType = sub[1]
        lib = subOne[0]
        lib = removePath (lib)

        defined[symbol] = lib

    return defined


def extractLinkage (listOfLibs = None):
    linkList = {}
    allLibs = shellExecute (cmdLibListAll)
    libs = shellExecute (cmdLibList)
    libs = libs.split ("\n")
    print "libs: ",libs
    for lib in libs:
        if len(lib) == 0:
            continue
        refLibList = shellExecute ("ldd "+lib)
        if "not a dynamic executable" in refLibList:
            continue
        refLibList = refLibList.split ("\n")
        for line in refLibList:
            match = re.match('\t(.*) =>',line)
            if match:
                refLib = match.group (1)
                refLib = re.sub ('.\d$','',refLib)
                if len (refLib) > 0:
                    if not refLib in allLibs:
                        continue
                    if not lib in linkList:
                        linkList[lib] = []
                    linkList[lib].append (refLib)
    return linkList



def extractLinkageReadelf (listOfLibs = None):
    linkList = {}
    allLibs = shellExecute (cmdLibListAll)
    libs = shellExecute (cmdLibList)
    libs = libs.split ("\n")
    print "libs: ",libs
    for lib in libs:
        if len(lib) == 0:
            continue
        refLibList = shellExecute ("readelf -d "+lib)
        if "Not and ELF file" in refLibList:
            continue
        refLibList = refLibList.split ("\n")
        for line in refLibList:
            line = re.sub (' +',' ',line)
            if not "(NEEDED)" in line:
                continue
            match = re.match ('.*\[(.*)\].*', line)
            if match:
                refLib = match.group (1)
                refLib = re.sub ('.\d$','',refLib)
                if len (refLib) > 0:
                    if not refLib in allLibs:
                        continue
                    if not lib in linkList:
                        linkList[lib] = []
                    linkList[lib].append (refLib)
    return linkList



def formatEdgeLabel (label):
    return label
    label = label.split ("(")
    label = [l.split(" ") for l in label]
    label = [x for sublist in label for x in sublist]
    label = "\\n".join (label)
    return label




data = None
if not createNew:
    try:
        data = pickle.load (open ("dependencies.saved", "rb"))
        linkage = data["linkage"]
        undefined = data["undefined"] 
        defined = data["defined"]
        references = data["references"]
        notFound = data["notFound"]
    except:
        pass
    
if createNew or not data:
    linkage = extractLinkageReadelf ()
    undefined = extractUndefined (shellExecute (cmdUndefined))
    defined = extractDefined (shellExecute (cmdDefined))

    references = {}
    notFound = []
    for sym in undefined:
        libs = undefined[sym]
        for lib in libs:
            #    print "undefined: lib: ",lib,"    sym: ",sym
            try:
                refLib = defined[sym]
                link = (lib, refLib)
                #        print "    link: ",link
                if link not in references.keys ():
                    references[link] = []
                references[link].append (sym)
            except:
                #        print "    not found: ",undef[2]," ",undef[0]
                notFound.append ([lib, sym])

    data = {"linkage" : linkage, "undefined" : undefined, "defined" : defined, "references" : references, "notFound" : notFound}
    pickle.dump (data, open ("dependencies.saved", "wp") )



diagram = "digraph Dependencies {\n"
for lib in linkage:
    if len(baseFilesList) > 0 and lib not in baseFilesList:
        continue
    for refLib in linkage[lib]:
        depSyms = False
        syms = []
        try:
            syms = references[(lib,refLib)]
            depSyms = True
        except:
            depSyms = False

        if depSyms and not showNeeded:
            continue

        if not depSyms and not showRedundant:
            continue

        color = "blue"
        if not depSyms:
            color = "red"

        width = 1;
        if showSymbols:
            if not depSyms:
                entry = '    "%s" -> "%s" [color=%s, penwidth=%s]; \n'%(lib,refLib, color, int(width))
                diagram += entry
            else:
                for s in syms:
                    entry = '    "%s" -> "%s" [color=%s, penwidth=%s, label="%s"]; \n'%(lib,refLib, color, int(width), formatEdgeLabel(s))
                    diagram += entry
        else:
            if not showThin:
                width = 1 + math.log(1+len(syms))*2
            if depSyms:
                entry = '    "%s" -> "%s" [color=%s, penwidth=%s, label="%s"]; \n'%(lib,refLib, color, int(width), len(syms))
            else:
                entry = '    "%s" -> "%s" [color=%s, penwidth=%s]; \n'%(lib,refLib, color, int(width))
            diagram += entry


diagram += "}\n"

try:
    f = open (filename+".dot", "w")
    f.write (diagram)
    f.close ()
except:
    pass


print "The following references could not be found"
for ref in notFound:
    print ref



