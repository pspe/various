
from matplotlib.colors import LogNorm
import matplotlib.pyplot as plt
import numpy as np
import numpy.random as rnd
from matplotlib.patches import Polygon
from math import *


class MeanVariance:
    
    def __init__ (self):
        self._sumweight = 0.0
        self._mean = 0.0
        self._M2 = 0.0
        self._n = 0.0

    def add (self, x, weight = 1.0):
        self._n += 1
        temp = self._sumweight + weight
        delta = x - self._mean
        R = delta * weight / temp
        self._mean += R
        self._M2 += self._sumweight * delta * R
        self._sumweight = temp

    def __add__ (self, values):
        try:
            for x in values:
                self.add (x)
        except TypeError, te:
            self.add (values)

        return self
            
        
    def mean (self):
        return self._mean

    def variance (self):
        return self._M2 / self._sumweight

    def variance_corr (self):
        if self._n < 2:
            return self.variance ()
        return self.variance () * self._n / (self._n - 1.0)

    def clear (self):
        self._sumweight = 0
        self._mean = 0
        self._M2 = 0
        self._n = 0

    def stdDev (self):
        return sqrt (self.variance ())

    def stdDev_corr (self):
        return sqrt (self.variance_corr ())
    
    



class Block:

    def __init__ (self, **kwargs): # "unique", "multiplicity", "marker"
        self._unique = float (kwargs.get ('unique', 1.0))
        self._multiplicity = float (kwargs.get ('multiplicity', 0.0))
        self._marker = int (kwargs.get ('marker', -1))
        self._color = kwargs.get ('color', 'r')

    def color (self):
        return self._color

    def marker (self):
        return self._marker
    
    def unique (self):
        return self._unique

    def setUnique (self, _unique):
        self._unique = _unique

    def multiplicity (self):
        return self._multiplicity

    def setMultiplicity (self, _multiplicity):
        self._multiplicity = _multiplicity

    def size (self):
        return self.unique () * self.multiplicity ()

    def __eq__(self, other):
        if isinstance(other, self.__class__): # 'other' is a block
            return self.unique () == other.unique () and self.multiplicity () == other.multiplicity ()
        return False
    
    def __str__ (self):
        s = "u = "+str(self.unique ())+ " m = "+str(self.multiplicity ()) + "    R = "+str(self.size ()) + "\n"
        return s


class SIDE:
    LEFT, RIGHT = range(2)

    @staticmethod
    def other (value):
        if value == SIDE.LEFT:
            return SIDE.RIGHT
        return SIDE.LEFT
    

class OPERATOR:
    AND, OR = range(2)

    

class EstimationMatrix:

    
    def __init__ (self):
        self._blocks = {SIDE.LEFT : [], SIDE.RIGHT : []}

    def scale (self, side, scaleTo):
        self.filter (side, scaleTo)
        
        
    def add (self, side, block):
        self._blocks[side].append (block)
        
    def unique (self, side):
        count = 0
        for block in self._blocks [side]:
            if block.multiplicity () == 0:
                continue
            count += block.unique ()
        return count

    def full (self, side):
        count = 0
        for block in self._blocks [side]:
            count += block.unique ()
        return count

    def size (self, side):
        count = 0
        for block in self._blocks [side]:
            count += block.size ()
        return count
    
    
    def fullToUnique (self, side, target):
        f = float (self.full (side))
        u = float (self.unique (side))

        #remove all non-connected blocks
        self._blocks [SIDE.LEFT] = [item for item in self._blocks[SIDE.LEFT] if item.multiplicity () != 0.0]
        self._blocks [SIDE.RIGHT] = [item for item in self._blocks[SIDE.RIGHT] if item.multiplicity () != 0.0]
        
        return target * u/f
        
    
    def filter (self, side, target, **kwargs): # 'include'  = [20, 17, ...], 'exclude' = [20, 19, ...] --> block markers

        # handle the first side
        remove = 0 # none removed so far

        R = self.size (side)
        
        include_markers = kwargs.get ('include', [])
        exclude_markers= kwargs.get ('exclude', [])
        op = kwargs.get ('operator', OPERATOR.AND)

        assert (include_counts == None or target >= len (include_counts))
        
        unique = self.unique (side)
        uniqueIncoming = unique

        include = []
        exclude = []
        blocks = []

        blocks = self._blocks[side]
        
        #excludes
        for idx in reversed(xrange(len(exclude_markers))):
            for idx_block in reversed (xrange(len(blocks))):
                if blocks[idx_block].marker () == exclude_markers[idx]:
                    exclude.append (blocks[idx_block])
                    del blocks[idx_block]
                    del exclude_markers[idx]
                    break

        #includes
        for idx in reversed(xrange(len(include_markers))):
            for idx_block in reversed (xrange(len(blocks))):
                if blocks[idx_block].marker () == include_marker[idx]:
                    include.append (blocks[idx_block])
                    del blocks[idx_block]
                    del include_marker[idx]
                    break
                    
        
        # compute size of all must-be-included and all other valid blocks
        unique_include = sum (block.unique () for block in include if block.multiplicity () > 0.0)
        unique_blocks = sum (block.unique () for block in blocks if block.multiplicity () > 0.0)
        
        unique_wo_include = unique_blocks - unique_include
        factor = (target - unique_include) / unique_wo_include

#        if factor >= 1.0:
#            return 0
        
        for block in exclude:
            remove += block.size ()
            
        #for block in include:
        #    R -= block.size ()

        additionalBlocks = []
        for block in blocks:
            if factor > 1.0 and block.unique () == 1.0:
                remove += (block.unique () * (1.0 - factor)) * block.multiplicity ()
                #block.setUnique (block.unique () * factor)
                additionalBlocks.append (Block (unique =block.unique () * (factor-1.0), multiplicity=block.multiplicity ()))
            else:
                remove += (block.unique () * (1.0 - factor)) * block.multiplicity ()
                block.setUnique (block.unique () * factor)
        blocks.extend (additionalBlocks)
                
#        blocks = [b for b in blocks if b.unique () > 0]
            
        self._blocks [side] = include
        self._blocks [side].extend (blocks)

        # now handle the second side
        otherSide = SIDE.other (side)

        factor = 1.0 - remove/R # factor for remaining relations

        for block in self._blocks [otherSide]:
            # how many relations are to be removed from this block
            uniqueBlock = block.unique ()
            if uniqueBlock <= 0.0:
                continue
            
            multiplicityBlock = block.multiplicity ()
            if multiplicityBlock <= 0.0:
                continue


            sizeBlock = block.size ()

            if factor >= 1.0:
                # derived from:
                # x*y = R
                # x'*y' = R'
                # with alpha = angle given by (x-1) and (y-1)
                # because the angle should be 0 if the block is e.g. [unique=100, multiplicity=1]
                # and the angle should be pi/2 if the block is e.g. [unique=1, multiplicity = 100]
                # --> x' = (1 + axisFactor*cos(alpha))
                # --> y' = (1 + axisFactor*sin(alpha))
                # multiply and solve quadratic formula for axisFactor
                if uniqueBlock < 1.0 or multiplicityBlock < 1.0:
                    continue

                unit = [uniqueBlock-1.0, multiplicityBlock-1.0]
                length = sqrt (sum ([pow (x,2.0) for x in unit]))
                unit = [x/length for x in unit]

                alpha = atan2 (unit[1], unit[0])

                epsilon = 0.0001
                axisFactor = 0.0
                if abs (alpha-0.0) < epsilon or abs(alpha-pi/2.0) < epsilon:
                    axisFactor = factor - 1.0
                else:
                    A = sin (alpha)*cos (alpha)
                    B = cos(alpha) + sin(alpha)
                    C = 1.0 - factor

                    axisFactor = (-B + sqrt (pow (B, 2) - 4*A*C))/(2*A)

                block.setUnique (block.unique () * (1.0+axisFactor*cos(alpha)))
                block.setMultiplicity (block.multiplicity () * (1.0+axisFactor*sin(alpha)))
                continue

            block.setUnique (uniqueBlock * (1.0 - pow (1.0 - factor, multiplicityBlock)))
            if (block.unique () > 0):
                block.setMultiplicity ((sizeBlock * factor)/block.unique ())
                multiplicityBlock = block.multiplicity ()
                if multiplicityBlock < 1:
                    block.setUnique (block.unique ()*multiplicityBlock)
                    block.setMultiplicity (1.0)


                    
        
    def __str__ (self):
        str = "---"
        str = "LEFT\n"
        for block in self._blocks[SIDE.LEFT]:
            str += block.__str__ ()
        str += "RIGHT\n"
        for block in self._blocks[SIDE.RIGHT]:
            str += block.__str__ ()
        return str
        

    def create (self, relations): # relations is a list of tuples with source id to target id for the relation
        leftDict = {}
        rightDict = {}
        for r in relations:
            leftDict.setdefault (r[0], set ()).add (r[1])
            rightDict.setdefault (r[1], set ()).add (r[0])

        self.createSide (SIDE.LEFT, leftDict)
        self.createSide (SIDE.RIGHT, rightDict)
        

    def createSide (self, side, relations): # dict of relations of one side
        multiplicities = []
        for key, value in relations.iteritems ():
            multiplicities.append (len (value))

        multiplicities.sort ()

        mean = sum (multiplicities)/len (multiplicities)
        var = sum (map (lambda x: (x-mean)**2,l))
        stdDev = sqrt (var)

        maxExtreme = 10
        for idx in reversed(xrange(len(multiplicities))): #loop from highest to lowest
            if maxExtreme <= 0:
                break
            
            if multiplicities[idx] > mean + 3*stdDev:
                self.add (side, Block (unique = 1, multiplicity = multiplicities[idx]))
                del multiplicities[idx]
                maxExtreme -= 1

        length = len(multiplicities)
        mean = sum(multiplicities)/length

        self.add (side, Block (unique = length, multiplicity = mean))
        


def meanList (data):
    stdDevs = []
    means = []
    mv = MeanVariance ()
    startData = data[0]
    endData = data[-1]
    lenData = len (data)
    for idx, x in enumerate (data):
        mv += x

        means.append (mv.mean ())
        stdDevs.append (mv.stdDev ())
    return means, stdDevs


        

def bucket (data, **kwargs): 
    limit = kwargs.get ('limit',0.1)
    depth = kwargs.get ('depth',3)
    minHeight = kwargs.get ('minHeight',1.0)
    individualSigma = kwargs.get ('individualSigma',3.0)
    maxExtremas = kwargs.get ('maxExtremas',0)

    forward = { "mean" : [], "stdDev" : []}

    forward["mean"],forward["stdDev"] = meanList (data)

        
    totalMean = forward["mean"][-1]
    totalStdDev = forward["stdDev"][-1]

    
    lengthMeans = len (forward["mean"])
    if totalMean < minHeight:
        return [Block (unique = lengthMeans * (minHeight - totalMean), multiplicity = 0.0), Block (unique = lengthMeans*totalMean, multiplicity = minHeight)]
        
#    if totalStdDev/totalMean < limit  or depth == 0:
    if totalStdDev < limit  or depth < 1.0: # <===================================== nur totalStdDev statt totalStdDev/totalMean
        return [Block (unique = len (forward["mean"]), multiplicity = totalMean)]

    # go into detail mode
    extremaBuckets = []
    if maxExtremas > 0:
        extremaLimit = totalMean + totalStdDev * individualSigma
        for x in list(reversed(data)):
            if x > extremaLimit and maxExtremas > 0:
                extremaBuckets.insert (0, Block (unique = 1.0, multiplicity=x, color='g'))
                maxExtremas -= 1
            
    numExtrema = len (extremaBuckets)
    data = data[:lengthMeans-numExtrema]
    forward["mean"] = forward["mean"][:lengthMeans-numExtrema]
    forward["stdDev"] = forward["stdDev"][:lengthMeans-numExtrema]

    totalMean = forward["mean"][-1]
    totalStdDev = forward["stdDev"][-1]
    
                
    
    backward = { "mean" : [], "stdDev" : []}
    backward["mean"],backward["stdDev"] = meanList (list(reversed(data)))

        
    backward["mean"] = list(reversed(backward["mean"]))
    backward["stdDev"] = list(reversed(backward["stdDev"]))

    sumVariance = []
    totalLength = float(len(forward["stdDev"]))
    for idx in xrange (len(forward["stdDev"])):
        try:
#            sumVariance.append ((forward["stdDev"][idx] * idx + backward["stdDev"][idx+1] * (totalLength - idx))/totalLength)
            sumVariance.append ((forward["stdDev"][idx] * (idx+1) + backward["stdDev"][idx+1] * (totalLength - (idx+1)))/totalLength)  # <===================================== 
        except:
            sumVariance.append (forward["stdDev"][idx] * idx/totalLength)

    minIndex = sumVariance.index (min (sumVariance))

    # if no split
    if minIndex == 0 or minIndex == len(forward["mean"]):
        return [Block (unique = len (forward["mean"]), multiplicity = totalMean)]

    lenData = len(data)
    
    leftDepth = (depth * (float(minIndex)/lenData))
    rightDepth = (depth * (float(lenData-minIndex))/lenData)

    buckets = bucket (data[0:minIndex+1], limit=limit, depth=leftDepth)
    buckets.extend (bucket (data[minIndex+1:], limit=limit, depth=rightDepth))
    buckets.extend (extremaBuckets)
    
    return buckets


        

def testFlat ():
    print "-----------------------"
    print "--- test flat ---"
    m = EstimationMatrix ()
    m. add (SIDE.LEFT, Block (unique = 50, multiplicity = 3))
    m. add (SIDE.RIGHT, Block (unique = 50, multiplicity = 3))

    print m

    print "filter left 20"
    m.filter (SIDE.LEFT, 20)

    print m

def testFlatHigh ():
    print "-----------------------"
    print "--- test flat-high ---"
    m = EstimationMatrix ()
    m. add (SIDE.LEFT, Block (unique = 50, multiplicity = 3))
    m. add (SIDE.RIGHT, Block (unique = 50, multiplicity = 3))
    m. add (SIDE.LEFT, Block (unique = 10, multiplicity = 15))
    m. add (SIDE.RIGHT, Block (unique = 10, multiplicity = 15))

    print m

    print "filter left 30"
    m.filter (SIDE.LEFT, 30)

    print m
    

def testLopsided ():
    print "-----------------------"
    print "--- test lopsided ---"
    m = EstimationMatrix ()
    m. add (SIDE.LEFT, Block (unique = 50, multiplicity = 3))
    m. add (SIDE.LEFT, Block (unique = 10, multiplicity = 20))

    m. add (SIDE.RIGHT, Block (unique = 350, multiplicity = 1))

    print m

    print "filter left 30"
    m.filter (SIDE.LEFT, 30)

    print m
    
def testLopsidedSlim ():
    print "-----------------------"
    print "--- test lopsided slim ---"
    m = EstimationMatrix ()
    m. add (SIDE.LEFT, Block (unique = 50, multiplicity = 3))
    m. add (SIDE.LEFT, Block (unique = 10, multiplicity = 20))

    m. add (SIDE.RIGHT, Block (unique = 175, multiplicity = m.size (SIDE.LEFT)/175))

    print m

    print "filter left 30"
    m.filter (SIDE.LEFT, 30)

    print m

def testLopsidedSlim_include ():
    print "-----------------------"
    print "--- test lopsided include ---"
    m = EstimationMatrix ()
    m. add (SIDE.LEFT, Block (unique = 50, multiplicity = 3))
    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 20, marker = 111))
    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 20))

    m. add (SIDE.RIGHT, Block (unique = 45, multiplicity = m.size (SIDE.LEFT)/45))
    print m

    print "filter left unique-20 include block with marker 111"
    m.filter (SIDE.LEFT, m.unique (SIDE.LEFT)-20, include = [111])
    print m

    print "filter left 1 include block with marker 111"
    m.filter (SIDE.LEFT, 1, include = [111])
    print m
    

def testLopsidedSlim_exclude ():
    print "-----------------------"
    print "--- test lopsided exclude ---"
    m = EstimationMatrix ()
    m. add (SIDE.LEFT, Block (unique = 50, multiplicity = 3))
    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 20, marker = 111))
    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 20))

    m. add (SIDE.RIGHT, Block (unique = 45, multiplicity = m.size (SIDE.LEFT)/45))
    print m

    # print "filter left unique-20 include block with marker 111"
    # m.filter (SIDE.LEFT, m.unique (SIDE.LEFT)-20, include = [111])
    # print m

    print "filter remove 1, exclude block with marker 111"
    m.filter (SIDE.LEFT, m.unique (SIDE.LEFT) - 1, exclude = [111])
    print m


def testBroadAndFlat ():
    print "-----------------------"
    print "--- test broad and flat ---"
    m = EstimationMatrix ()

    m. add (SIDE.LEFT, Block (unique = 5000, multiplicity = 6))
    m. add (SIDE.RIGHT, Block (unique = 30000, multiplicity = 1))
    print m

    print "filter left to target 1"
    m.filter (SIDE.LEFT, 1)
    print m


def testRemoveNothing ():
    print "-----------------------"
    print "--- test remove nothing ---"
    m = EstimationMatrix ()

    m. add (SIDE.LEFT, Block (unique = 800, multiplicity = 1))
    m. add (SIDE.RIGHT, Block (unique = 20, multiplicity = 40))
    print m

    print "filter left to target 800"
    m.filter (SIDE.LEFT, 800)
    print m

def testInconsistentLeftRight ():
    print "-----------------------"
    print "--- test inconsistent left right ---"
    m = EstimationMatrix ()

    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 0))
    m. add (SIDE.LEFT, Block (unique = 1000, multiplicity = 1))

    m. add (SIDE.RIGHT, Block (unique = 1, multiplicity = 0))
    m. add (SIDE.RIGHT, Block (unique = 80, multiplicity = 10))
    print m

    print "filter left to target 1000"
    m.filter (SIDE.LEFT, 1000)
    print m
    
    print "filter right to target 80"
    m.filter (SIDE.RIGHT, 80)
    print m
    

def testFilterIssue ():
    print "-----------------------"
    print "--- test filter issue ---"
    m = EstimationMatrix ()

    m. add (SIDE.LEFT, Block (unique = 80, multiplicity = 0))
    m. add (SIDE.LEFT, Block (unique = 20, multiplicity = 8))

    m. add (SIDE.RIGHT, Block (unique = 200, multiplicity = 0))
    m. add (SIDE.RIGHT, Block (unique = 800, multiplicity = 1))
    print m

    print "filter right to target 400"
    m.filter (SIDE.RIGHT, 400)
    print m
    
def testFilterRightTo1OnInconsistent ():
    print "-----------------------"
    print "--- test filter right to 1 ---"
    m = EstimationMatrix ()

    m. add (SIDE.LEFT, Block (unique = 0, multiplicity = 0))
    m. add (SIDE.LEFT, Block (unique = 1000, multiplicity = 1))

    m. add (SIDE.RIGHT, Block (unique = 20, multiplicity = 0))
    m. add (SIDE.RIGHT, Block (unique = 80, multiplicity = 10))
    print m

    print "filter right to target 1"
    m.filter (SIDE.RIGHT, 1)
    print m
    
def testRemoveOneLarge ():
    print "-----------------------"
    print "--- test remove 1 large ---"
    m = EstimationMatrix ()

    m. add (SIDE.LEFT, Block (unique = 20, multiplicity = 0))
    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 31, marker = 111))
    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 29))
    m. add (SIDE.LEFT, Block (unique = 80, multiplicity = 10))
    m. add (SIDE.LEFT, Block (unique = 200, multiplicity = 0))
    m. add (SIDE.LEFT, Block (unique = 860, multiplicity = 1))

    m. add (SIDE.RIGHT, Block (unique = 200, multiplicity = 0))
    m. add (SIDE.RIGHT, Block (unique = 860, multiplicity = 1))
    m. add (SIDE.RIGHT, Block (unique = 20, multiplicity = 0))
    m. add (SIDE.RIGHT, Block (unique = 1, multiplicity = 31, marker = 222))
    m. add (SIDE.RIGHT, Block (unique = 80, multiplicity = 10))
    print m

    print "filter left to target 1"
    m.filter (SIDE.LEFT, 1, include = [111])
    print m


def testFilterOneOfLargeAverage ():
    print "-----------------------"
    print "--- test remove 1 large ---"
    m = EstimationMatrix ()

    m. add (SIDE.LEFT, Block (unique = 10000, multiplicity = 4))
    m. add (SIDE.RIGHT, Block (unique = 10, multiplicity = 4000))
    print m

    print "filter right to target 1"
    m.filter (SIDE.RIGHT, 1, include = [111]) # marker not present
    print m


def testFilterRemovalOf31To1Target ():
    print "-----------------------"
    print "--- test removal of 31 To 1 Target large ---"
    m = EstimationMatrix ()

    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 31, marker = 111))
    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 29))
    m. add (SIDE.LEFT, Block (unique = 80, multiplicity = 10))
    m. add (SIDE.RIGHT, Block (unique = 860, multiplicity = 1))
    print m

    print "filter right to target 1"
    m.filter (SIDE.LEFT, 1, include = [111])
    print m



def testScaleEqual ():
    print "-----------------------"
    print "--- test scale equal ---"
    m = EstimationMatrix ()
    m. add (SIDE.LEFT, Block (unique = 50, multiplicity = 50))
    m. add (SIDE.RIGHT, Block (unique = 50, multiplicity = 50))

    print m

    print "scale left by 2.0"
    m.scale (SIDE.LEFT, m.full (SIDE.LEFT) * 2.0)

    print m

    print "scale right by 2.0"
    m.scale (SIDE.RIGHT, m.full (SIDE.RIGHT) * 2.0)

    print m

    

def testScaleLopsided ():
    print "-----------------------"
    print "--- test scale lopsided ---"
    m = EstimationMatrix ()
    m. add (SIDE.LEFT, Block (unique = 50, multiplicity = 3))
    m. add (SIDE.LEFT, Block (unique = 10, multiplicity = 20))

    m. add (SIDE.RIGHT, Block (unique = 350, multiplicity = 1))

    print m

    print "scale left by 1.5"
    m.scale (SIDE.LEFT, m.full (SIDE.LEFT) * 1.5)

    print m

    print "scale right by 2.0"
    m.scale (SIDE.RIGHT, m.full (SIDE.RIGHT) * 2.0)

    print m


def testScaleFlatHigh ():
    print "-----------------------"
    print "--- test scale flat high ---"
    m = EstimationMatrix ()
    m. add (SIDE.RIGHT, Block (unique = 100, multiplicity = 1))

    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 40))
    m. add (SIDE.LEFT, Block (unique = 9, multiplicity = (m.size (SIDE.RIGHT)-m.size(SIDE.LEFT))/9.0))

    print m

    m.filter (SIDE.LEFT, m.full (SIDE.LEFT) * 2.0)
    
    print m

def testScaleFlatHigh_otherSide ():
    print "-----------------------"
    print "--- test scale flat high other side ---"
    m = EstimationMatrix ()
    m. add (SIDE.RIGHT, Block (unique = 100, multiplicity = 1))

    m. add (SIDE.LEFT, Block (unique = 1, multiplicity = 40))
    m. add (SIDE.LEFT, Block (unique = 9, multiplicity = (m.size (SIDE.RIGHT)-m.size(SIDE.LEFT))/9.0))

    print m

    m.filter (SIDE.RIGHT, m.full (SIDE.RIGHT) * 2.0)

    print m



    
    
def test ():
    #testFlat ()
    #testFlatHigh ()
    #testLopsided ()
    #testLopsidedSlim ()
    #testLopsidedSlim_include ()
    #testLopsidedSlim_exclude ()
    #testBroadAndFlat ()
    #testRemoveNothing ()
    #testInconsistentLeftRight ()
    #testFilterIssue ()
    #testFilterRightTo1OnInconsistent ()
    #testRemoveOneLarge ()
    #testFilterOneOfLargeAverage ()
    #testFilterRemovalOf31To1Target ()
    testScaleLopsided ()
    testScaleFlatHigh ()
    testScaleFlatHigh_otherSide ()
    testScaleEqual ()

    
def draw (data, **kwargs):
    #data [:] = data[::-1]
    maxLen = len(data)
    t = np.arange(0, maxLen, 1)

    fig = plt.figure(figsize=(12, 6))
    vax = fig.add_subplot(111)

    vax.plot(t, data, '-b^')
#    vax.vlines(t, [0], data, lw = 2, linestyles='dashed')
    vax.set_xlabel('index')
    vax.set_title('Number of relations')


    forward = { "mean" : [], "stdDev" : []}
    mv = MeanVariance ()
    for x in data:
        mv += x
        forward["mean"].append (mv.mean ())
        forward["stdDev"].append (mv.stdDev ())

    print " "
    print forward["mean"]
    print " "

    backward = { "mean" : [], "stdDev" : []}
    mv = MeanVariance ()
    for x in list(reversed(data)):
        mv += x
        backward["mean"].append (mv.mean ())
        backward["stdDev"].append (mv.stdDev ())

    print " "
    print backward["mean"]
    print " "

    backward["mean"] = list(reversed(backward["mean"]))
    backward["stdDev"] = list(reversed(backward["stdDev"]))

    #plt.errorbar (t, forward["mean"], yerr = forward["stdDev"], capsize =7, color='r', linestyle = 'dotted')
    #plt.errorbar (t, backward["mean"], yerr = backward["stdDev"], capsize =7, color='g', linestyle = 'dotted')

    sumStdDev = []
    for idx in xrange (len(forward["stdDev"])):
        try:
            #sumStdDev.append (forward["stdDev"][idx] + backward["stdDev"][idx+1])
            totalLength = float(len(forward["stdDev"]))
            sumStdDev.append ((forward["stdDev"][idx] * idx + backward["stdDev"][idx+1] * (totalLength - idx))/totalLength)
        except:
            sumStdDev.append (forward["stdDev"][idx] * idx/totalLength)
    
    plt.errorbar (t, sumStdDev, yerr = [0]*len(sumStdDev), capsize =1, color='m', linestyle = 'dashdot')

    
    blocks = list (kwargs.get ('blocks', []))
    
    
    # Make the shaded region
    currentPosition = 0
    for block in blocks:
        nextPosition = currentPosition + block.unique ()
        verts = [(currentPosition,0),(nextPosition,0),(nextPosition,block.multiplicity()),(currentPosition,block.multiplicity())]
        #poly = Polygon(verts, facecolor='0.9', edgecolor='0.5', color='r', alpha=0.2)
        poly = Polygon(verts, color=block.color (), alpha=0.2)
        vax.add_patch(poly)
        currentPosition = nextPosition
    
    plt.show()



def testDraw ():
    #data = np.array ([8,5,4,2])
    data0 = rnd.exponential (10, 30)
    data1 = rnd.triangular (4, 5, 5, 20)
    data2 = rnd.triangular (10,11,11, 10)
    data3 = rnd.triangular (14,14,15, 5)
    data4 = rnd.triangular (0,1,1, 10)
    data = np.concatenate ((data0,data1,data2,data3,data4), axis=0)
    #data = data0
    
    #data0 = rnd.triangular (5, 5, 10, 100)
    #data1 = rnd.triangular (10, 20, 20, 100)
    #data2 = rnd.triangular (20, 21, 21, 0)
    #data = np.concatenate ((data0, data1, data2), axis=0)

    #data = rnd.triangular (0, 10, 15, 200)
    
    data = np.sort (data)
    data = np.ceil (data, None)
    blocks = bucket (data, limit=0.1, depth=10, individualSigma = 2.0, maxExtremas=50)
    draw (data, blocks=blocks)
    

def testDrawVarying ():
    #data = np.array ([8,5,4,2])
    data0 = rnd.exponential (10, 30)
    data1 = rnd.triangular (4, 5, 5, 20)
    data2 = rnd.triangular (10,11,11, 10)
    data3 = rnd.triangular (14,14,15, 5)
    data4 = rnd.triangular (0,1,1, 10)
    data = np.concatenate ((data0,data1,data2,data3,data4), axis=0)
    #data = data0

    data = rnd.beta (5,2,1000)
    data = [10 * x for x in data]

    #data0 = rnd.triangular (5, 5, 10, 100)
    #data1 = rnd.triangular (10, 20, 20, 100)
    #data2 = rnd.triangular (20, 21, 21, 0)
    #data = np.concatenate ((data0, data1, data2), axis=0)

    #data = rnd.triangular (0, 10, 15, 200)
    
    data = np.sort (data)
    #data = np.ceil (data, None)
    
    blocks = bucket (data, limit=0.1, depth=3, individualSigma = 2.0, maxExtremas=0, minHeight=0.0)
    draw (data, blocks=blocks)


    

def testMeanVariance ():
    data = rnd.exponential (10, 20)
    data = np.sort (data)
    data = np.ceil (data, None)

    mv = MeanVariance ()
    mv += data

    print " "
    print "data = ",data
    print "mean = ",mv.mean ()
    print "variance = ",mv.variance_corr ()
    print "stddev = ",mv.stdDev_corr ()
    
    mv2 = MeanVariance ()
    data = list (reversed(data))
    mv2 += data
    
    print " "
    print "data = ",data
    print "mean = ",mv.mean ()
    print "variance = ",mv.variance_corr ()
    print "stddev = ",mv.stdDev_corr ()
    

    
if __name__ == "__main__":
    #test ()
    testDraw ()
    #testDrawVarying ()
    #testMeanVarianceSimple ()
    #testMeanVariance ()



    
        
