#!/usr/bin/env python
# -*- coding: utf-8 -*-



import pandas as pd
import csv
import sys
import numpy
import codecs
import re


import unicodedata

def remove_diacritic(input):
    '''
    Accept a unicode string, and return a normal string (bytes in Python 3)
    without any diacritical marks.
    '''
    return unicodedata.normalize('NFKD', input).encode('ASCII', 'ignore')


def replaceSpecialCharacters (input):
    return re.sub(r'[\W_]', '_', input)




def unicode_csv_reader (utf8_data, dialect=csv.excel, **kwargs):
    csv_reader = csv.reader (utf8_data, dialect=dialect, **kwargs)
    for row in csv_reader:
        yield [unicode(cell, 'utf-8') for cell in row]


        
data_path = "./"
print ("arguments: "+str (sys.argv));

filename = data_path+"corporateNames.csv"
#filename = data_path+"testNames.csv"
#filename = data_path+"testNames2.csv"

# Load the data
#texts  = pd.read_csv (data_path+"corporateNames.csv")

reader = unicode_csv_reader (open (filename))
data = [txt[0] for txt in reader if len(txt) > 0]



#print data





class Trigram:
    def __init__ (self):
        self._grams = {1:{}, 2:{}, 3:{}}
        #self._sum = {1:0.0, 2:0.0, 3:0.0}
        self._numStrings = 0.0

        
    def __add__ (self, toadd):
        if type (toadd) is unicode or type (toadd) is str: # add string
            self.addString (toadd)
        else:
            for s in toadd:
                self.addString (s)
        return self

    def addString (self, input):
        self._numStrings += 1.0
        for n in xrange (1,4):
            grams = self.ngramify (input, n)
            localGrams = {}
            for g in grams:
                localGrams.setdefault (g, 0.0)
                localGrams[g] += 1.0
            for g in localGrams:
                self._grams[n].setdefault (g, 0.0)
                self._grams[n][g] += 1.0


            
    def ngramify (self, input, n):
        input = u' '+replaceSpecialCharacters (remove_diacritic (input.lower ().strip ()))+u' '
        # return zip (*[input[i:] for i in xrange(n)]) # n for ngrams
        # return [u''.join (tri).encode ('utf-8').strip () for tri in zip (*[input[i:] for i in xrange(n)])] # n for ngrams
        return [u''.join (tri).encode ('utf-8') for tri in zip (*[input[i:] for i in xrange(n)])] # n for ngrams

    
    def correspondingGramCount (self, gram, direction):
        length = len (gram)
        if length <= 1:
            return 0

        sub = gram[1:] if direction == 1 else gram[:-1]
        count = 0.0
        mapTo = 0.0
        nxtDict = {}
        for g, value in self._grams[length].iteritems ():
            fullG = g
            g = g[:-1] if direction == 1 else g[1:]
            if sub == g:
                count += 1.0
                mapTo += value
                nxtDict.setdefault (fullG, 0.0)
                nxtDict[fullG] += value
        return count, mapTo, nxtDict
            


    def removeStartEnd (self, grams, startsWith, endsWith):

        if not startsWith:
            grams = grams [1:]
        if not endsWith:
            grams = grams[:-1]

        return grams


    def count3 (self, tag, **kwargs):
        startsWith = kwargs.get ('startsWith', False)
        endsWith   = kwargs.get ('endsWith', False)

        tagLength = len (tag) + (1 if startsWith else 0) + (1 if endsWith else 0)
        
        if tagLength == 0: # too short for everything
            return self._numStrings, " empty input, return everything"
        elif tagLength == 1: # unigrams
            grams = self.ngramify (tag, 1) # make unigrams
            grams = self.removeStartEnd (grams, startsWith, endsWith)
            return self._grams[1].get (grams[0], 0.0), " from unigrams"
        elif tagLength == 2: #bigrams
            grams = self.ngramify (tag, 2) # make bigrams
            grams = self.removeStartEnd (grams, startsWith, endsWith)
            return self._grams[2].get (grams[0], 0.0), " from bigrams"

        grams = self.ngramify (tag, 3) # make trigrams
        grams = self.removeStartEnd (grams, startsWith, endsWith)

        index = 0
        p = [self._grams[3].get (t, 0.0) for t in grams]
        index = p.index (min (p))
        
        baseProbability = p[index]/self._numStrings

        condProbabilityRight = 1.0;
        condProbabilityLeft = 1.0;

        nextCounts = [self.correspondingGramCount (t, 1)[0] for t in grams[:-1]]
        prevCounts = [self.correspondingGramCount (t, -1)[0] for t in grams[1:]]

        #nextCounts = [self.nextTrigramCount (t)[1] for t in grams[:-1]]
        #prevCounts = [self.previousTrigramCount (t)[1] for t in grams[1:]]
        
        # print "index = ",index
        # print nextCounts
        # print prevCounts

        idxR = max(0,index-1)
        idxL = index
        condProbabilityRight = reduce (lambda acc, (mapTo,mapFrom) : acc * (min(mapTo,mapFrom)/max(mapTo,mapFrom)), zip (nextCounts, prevCounts), 1.0)
        #condProbabilityRight = reduce (lambda acc, (mapTo,mapFrom) : acc * min((mapTo/mapFrom), 1.0), zip (nextCounts, prevCounts), 1.0)
        #condProbabilityRight = reduce (lambda acc, (mapTo,mapFrom) : acc * min((mapFrom/mapTo), 1.0), zip (nextCounts, prevCounts), 1.0)
        
        #condProbabilityRight = reduce (lambda acc, (mapTo,mapFrom) : acc * min(mapTo/mapFrom,1.0), zip (nextCounts[idxR:], prevCounts[idxR:]), 1.0)
        #condProbabilityLeft = reduce (lambda acc, (mapTo,mapFrom) : acc * min(mapFrom/mapTo, 1.0), zip (nextCounts[:idxL], prevCounts[:idxL]), 1.0)

        #condProbabilityRight = reduce (lambda acc, (BC_,_BC,ABC,BCE) : acc * (ABC*BCE/(BC_*_BC)), zip (nextCounts[idxR:], prevCounts[idxR:], p[idxR:-1], p[idxR+1:]), 1.0)
        #condProbabilityLeft = reduce (lambda acc, (BC_,_BC,ABC,BCE) : acc * (ABC*BCE/(BC_*_BC)), zip (prevCounts[idxL::-1], nextCounts[idxL::-1], p[idxL::-1], p[idxL+1::-1]), 1.0)
#        condProbabilityLeft = reduce (lambda acc, (mapTo,mapFrom) : acc * (mapFrom/mapTo), zip (nextCounts[:idxL], prevCounts[:idxL]), 1.0)

        probability = baseProbability * condProbabilityRight * condProbabilityLeft
        comment = "p_base = "+str (baseProbability)+ " p_c_r = "+str (condProbabilityRight)+ " p_c_l = "+str (condProbabilityLeft)

        probability = min (probability, baseProbability)
        return probability * self._numStrings, comment


    def getMostProbableNextGram (self, tag):
        length = len (tag)
        if length < 1:
            return 0

        sub = tag[-3:]
        if len (sub) > 2:
            sub = sub[1:]
        length = len(sub) + 1
        count = 0.0
        mapTo = 0.0
        nxtDict = {}
        bestGram = None
        bestGramValue = 0
        for g, value in self._grams[length].iteritems ():
            fullG = g
            if g[:-1] == sub:
                if value > bestGramValue:
                    bestGram = g
                    bestGramValue = value
        return bestGram, bestGramValue
        

    def getMostProbableWord (self, tag, maxLength):
        probableWord = tag
        gram = tag
        for i in xrange (maxLength):
            nextGram, nextGramValue = self.getMostProbableNextGram (gram)
            if not nextGram:
                break
            ng = nextGram
            if len (ng) > 1:
                ng = ng[-1:]
            probableWord += ng
            gramLen = min (len (probableWord), 3)
            gram = probableWord[-gramLen:]
        return probableWord

    
    
    def __str__ (self):
        s = ""
        for t, v in self._grams[3].iteritems ():
            line = u"".join ([unicode(x) for x in t]) 
            line += u" --> "+str (v)
            s += line+"\n"
        return s
    

    
def testTag (trigrams, tagIncoming):
    tag = tagIncoming.lower ()
    startsWith = False
    if tag.startswith (u'*'):
        tag = tag.strip (u'*')
        found = [txt for txt in data if tag in txt.lower ()]
    else:
        found = [txt for txt in data if txt.lower ().startswith (tag)]
        startsWith = True

    # probability world
    count, comment = trigrams.count3 (tag, startsWith = startsWith)
    
    return tagIncoming,len(found),"%1.1f"%count, "   ",comment
    


def test ():
    t = Trigram ()
    t.addString ("hallo ich bin's ")
    t += data
    return len (t._grams[3])


#print test ()

tri = Trigram ()
tri += data
#print tri


print testTag (tri, u"*sich")
print testTag (tri, u"*versich")
print testTag (tri, u"*versicherung")
print testTag (tri, u"*aba")

    

def trigramData (tags):
    for t in l:
        print " "
        print u' '+t,"   #words          --> ",tri._grams[3].get (t, 0.0)
        print u'_'+t[:-1]+u'  ',"  #TG,#prev words --> ",tri.correspondingGramCount (t, -1)
        print u' '+t[:-1]+u'  ',"  #2-gram         --> ",tri._grams[2].get (t[:-1])
        print u'  '+t[1:]+u'_',"  #TG,#next words --> ",tri.correspondingGramCount (t, 1)
        print u'  '+t[1:]+u' ',"  #2-gram         --> ",tri._grams[2].get (t[1:])



#l = [u'Bor',u'ore',u'rea',u'eal',u'ali',u'lis']
#l = [u'ver',u'ers',u'rsi',u'sic',u'ich',u'che',u'her',u'eru',u'run',u'ung']
l = [u'sic']


trigramData (l)

    
ore = tri.correspondingGramCount (u'ore', 1)
rea = tri.correspondingGramCount (u'rea', -1)

common = {}
for o, ov in ore[2].iteritems ():
    for r, rv in rea[2].iteritems ():
        if o[2] == r[0]:
            common[o[2]] = (ov, rv)

print "common = ",common

#print tri
print tri.getMostProbableWord (u'a',10)

