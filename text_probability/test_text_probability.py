#!/usr/bin/env python
# -*- coding: utf-8 -*-



import pandas as pd
import csv
import sys
import numpy


def unicode_csv_reader (utf8_data, dialect=csv.excel, **kwargs):
    csv_reader = csv.reader (utf8_data, dialect=dialect, **kwargs)
    for row in csv_reader:
        yield [unicode(cell, 'utf-8') for cell in row]


        
data_path = "./"
print ("arguments: "+str (sys.argv));

filename = data_path+"corporateNames.csv"
#filename = data_path+"testNames.csv"

# Load the data
#texts  = pd.read_csv (data_path+"corporateNames.csv")

reader = unicode_csv_reader (open (filename))
data = [txt[0] for txt in reader if len(txt) > 0]




def incrementValueForKey (d, key):
    try:
        val = d[key]
        d[key] = val + 1.0
    except:
        d[key] = 1.0



        
def extractProbabilities (data):
    dictFirstLetter = {}
    dictLengths = {}
    dictNextLetter = {}
    for txt in data:
        txt = txt.lower ()
        # --- text lengths
        length = len (txt)
        incrementValueForKey (dictLengths, length)

        # --- first letter probability 
        firstLetter = u''
        try:
            firstLetter = txt.lower ()[0]
        except:
            pass

        if len(firstLetter) > 0:
            incrementValueForKey (dictFirstLetter, firstLetter)

        # --- next letter probability
        if length > 1:
            for letter in txt[1:]:
                incrementValueForKey (dictNextLetter, letter.lower ())


    total = sum (dictFirstLetter.values ())
    dictFirstLetter = {key: value/total for key,value in dictFirstLetter.iteritems ()}

    total = sum (dictNextLetter.values ())
    dictNextLetter = {key: value/total for key,value in dictNextLetter.iteritems ()}
    
    total = sum (dictLengths.values ())
    # make cumulative lengths
    dictLengths = {key: sum ({k: v for k,v in dictLengths.iteritems () if k >= key}.values ())/total for key,value in dictLengths.iteritems ()}

    largestLength = max (dictLengths.keys ())

    dictTrailingStrings = {}
    for currLength in xrange (0,largestLength):
        d = {}
        for txt in data:
            if len (txt) >= currLength:
                key = txt.lower ()[:currLength]
                value = txt.lower ()[currLength:]
                d.setdefault (key, []).append (value)
        lengths = [len(set (v)) for v in d.values ()]
        if len (lengths) != 0:
            dictTrailingStrings[currLength] = float(max (lengths))
    total = len (data)
    dictTrailingStrings = {key: value/total for key,value in dictTrailingStrings.iteritems ()}


    key_length_limit = 3
    dictRemainingStrings = {}
    dictSpecialStrings = {}
    total = 0
    for currLength in xrange (largestLength-1,0,-1):
        d = {}
        for txt in data:
            length = len (txt)
            if length > currLength:
                keylist = []
                for pos in xrange (0, length-currLength+1):
                    key = txt.lower ()[pos:pos+currLength]
                    keylist.append (key)

                for key in set (keylist):
                    d.setdefault (key, 0)
                    d[key] += 1

        
        if len (d.values ()) > 0:
            combo = zip (d.values (), d.keys ())
            combo = sorted (combo, key = lambda val: val[0], reverse = True)
            val_mean = numpy.mean (d.values ())
            val_std = numpy.std (d.values ())
            val_limit = 6
            values = []
            for c in combo:
                k = c[1]
                length = len(k)
                value = c[0]

                specialKeys = set (dictSpecialStrings.keys ())
                #specialKeys = [sk for sk in specialKeys if len(sk) == length + 1 and k in sk]
                specialKeys = [sk for sk in specialKeys if k in sk]
                inSpecial = len(specialKeys) > 0
                
                if inSpecial:
                    sumSpecial = 0.0
                    for sk in specialKeys:
                        sumSpecial += dictSpecialStrings[sk]
                    value -= sumSpecial
                    #dictSpecialStrings[k] = sumSpecial
                else:
                    if value > val_limit and length > key_length_limit:
                        dictSpecialStrings[k] = value
                        continue

                if value > 0:
                    values.append (value)
                
            if len(values) > 0:
                dictRemainingStrings[currLength] = float(max (values))
                #dictRemainingStrings[currLength] = float(numpy.mean (values))


    
    keys = dictSpecialStrings.keys ()
    for k in keys:
        length = len (k)
        value = dictSpecialStrings[k]
        for sublen in xrange (length-1,0,-1):
            for pos in xrange (0, length-sublen+1):
                subkey = k.lower ()[pos:pos+sublen]
                dictSpecialStrings.setdefault (subkey, 0)
                dictSpecialStrings[subkey] += value

    # print dictRemainingStrings
    # print dictSpecialStrings

    # sys.exit ()
            

                
    total = len (data)
    #dictRemainingStrings = {key: value/total for key,value in dictRemainingStrings.iteritems ()}
    #dictSpecialStrings = {key: float(value)/total for key,value in dictSpecialStrings.iteritems ()}

    
#    print dictTrailingStrings
    print dictRemainingStrings
    
    return dictLengths,dictFirstLetter,dictNextLetter,dictTrailingStrings,dictRemainingStrings, dictSpecialStrings



def getProbability (dicts, word):
    dictLengths = dicts[0]
    dictFirstLetter = dicts[1]
    dictNextLetter = dicts[2]
    dictTrailingStrings = dicts[3]
    dictRemainingStrings = dicts[4]
    dictSpecialStrings = dicts[5]
    prob = 1.0
    word = word.lower ()
    comment = ""
    
    if False:
        length = len (word)
        if word.startswith (u'*'):
            length = length - 1.0
        try:
            val = dictLengths[length]
            print "word length prob = ",val
            prob = prob * val
        except:
            pass
    
    if True:
        if len(word) > 0 and not word.startswith (u'*'):
            firstLetter = word[0]
            #print firstLetter
            try:
                #print dictFirstLetter
                val = dictFirstLetter[firstLetter]
                #print "first letter prob = ",val
                prob = prob * val
            except:
                pass
            word = word[1:]
            
    # don't consider the first letter for the following computation
    #if word.startswith (u'*'):
    #    word = word[1:] # either there is a "*" too much or the first letter has already been tackled
    #print word

            
    if False:
        for letter in word:
            try:
                val = dictNextLetter[letter]
                #print "next letter prob = ",val
                prob = prob * val
            except:
                pass

    if False:
        val = 0.2
        for letter in word:
            prob = prob * val
            #print "prob = ",prob

    if word.startswith (u'*'):
        length = len (word)
        length = length - 1.0
        w = word[1:]
        val = 0
        try:
            val += dictSpecialStrings[w]
            comment += "special string "+"%1.4f"%val
        except:
            pass
            
        val += dictRemainingStrings.get (length,0)
        prob = prob * val
    else:
        length = len (word)
        val = dictTrailingStrings.get (length,0)
        #sys.exit ()
        prob = prob * val
            
    return prob, comment


def testWord (data, dicts, wordIncoming):
    # real world
    word = wordIncoming.lower ()
    if word.startswith (u'*'):
        word = word.strip (u'*')
        found = [txt for txt in data if word in txt.lower ()]
    else:
        found = [txt for txt in data if txt.lower ().startswith (word)]

    # probability world
    prob, comment = getProbability (dicts, wordIncoming)
    quantity = max(1.0,len (data) * prob)
    
    return wordIncoming,len(found),"%1.1f"%quantity, "%1.4f"%prob, comment



dicts = extractProbabilities (data)

#print data
print testWord (data, dicts, u"*GAG")
print testWord (data, dicts, u"GAG")
