
from matplotlib.colors import LogNorm
import matplotlib.pyplot as plt
import numpy as np
import numpy.random as rnd
from matplotlib.patches import Polygon
from math import *
from bisect import *
from copy import *


def isMultipleOf (larger, smaller):
    if smaller == 0:
        return False
    quotient = larger/smaller
    if modf (quotient)[0] == 0.0: # larger value is a multiple of the smaller value
        return True
    return False

def adjustToDiscrete (original, proposed, width):
    diff = proposed - original
    if isMultipleOf (diff, width) or diff == 0: # if proposed cuts at a discrete value
        return proposed, width

#    if diff < 0: # potential cut into a discrete range, but causing a smaller step width
    if isMultipleOf (width, abs (diff)):
        return proposed, abs (diff)
#    else:
#        return proposed, 0.0
    
    # doesn't cut at a discrete value
    split = modf (diff/width)
    return original + width * (split[1] + 1 if split[0] >= 0.5 else 0), width


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

    def __init__ (self, **kwargs):
        self._binWidth = float (kwargs.get ('binWidth', 1.0))
        self._startValue = float (kwargs.get ('startValue', 0.0))
        self._endValue = float (kwargs.get ('endValue', 0.0))
        self._discrete = float (kwargs.get ('discrete', False))
        self._stepWidth = float (kwargs.get ('stepWidth', 0))
        self._color = kwargs.get ('color', 'r')
        self.EqualFloatMode = kwargs.get ('equalFloatMode', "float")

    def color (self):
        return self._color

    def binWidth (self):
        return self._binWidth

    def setBinWidth (self, _binWidth):
        self._binWidth = _binWidth

    def startValue (self):
        return self._startValue

    def endValue (self):
        return self._endValue

    def setMinValue (self, _startValue):
        self._startValue = _startValue

    def setMaxValue (self, _endValue):
        self._endValue = _endValue
        
    def setDiscrete (self, _discrete):
        self._discrete = _discrete
        
    def interpolate (self, x):
        if self.binWidth () == 0.0:
            return self.endValue ()
        return self.startValue () + ((self.endValue () - self.startValue ())/self.binWidth ())*x

    def estimateLowEqualHigh (self, y):
        localY = y - self.startValue ()
        diff = self.endValue () - self.startValue ()
        if diff == 0.0:
            return 0.0, self.binWidth (), 0.0

        binWidthPerY = self.binWidth ()/diff

        lower = 0
        equal = 0
        higher = 0
        if self._discrete:
            if self._stepWidth == 0:
                equal = self.binWidth ()
            else:
                numDiscretes = floor (abs ((diff+self._stepWidth) / self._stepWidth))
                if isMultipleOf (localY, self._stepWidth):
                    equal = self.binWidth () / numDiscretes
                else:
                    equal = 0.0

                remaining = self.binWidth () - equal
                numDiscretesBelow = floor (localY/self._stepWidth)
                if equal == 0.0:
                    numDiscretesBelow += 1.0
                lower =  numDiscretesBelow * (self.binWidth () / numDiscretes)
                higher = remaining - lower
                return lower, equal,  higher
                    
        else:
            if self.EqualFloatMode == "1":
                equal = 1.0
            elif self.EqualFloatMode == "0":
                equal = 0.0
            else:
                #        binWidth / length of the hypothenuse
                equal = self.binWidth () / sqrt (pow (self.binWidth (), 2) + pow (diff, 2))
            
        f_lower = localY * binWidthPerY
        f_higher = (diff - localY) * binWidthPerY

        remaining = self.binWidth () - equal
        lower = remaining * f_lower/(f_lower+f_higher)
        higher = remaining * f_higher/(f_lower+f_higher)

        return lower, equal, higher


    def sub (self, yStart, yEnd): # get sub-block from start to end value
        yStart = max (yStart, self.startValue ())
        yEnd = min (yEnd, self.endValue ())
        width = self._stepWidth
        
        if self._discrete:
            yStart, width = adjustToDiscrete (self.startValue (), yStart, width)
            yEndTmp, width = adjustToDiscrete (self.endValue (), yEnd, width)
            yEnd = max (yStart, yEndTmp)

        estEnd = self.estimateLowEqualHigh (yEnd)
        estStart = self.estimateLowEqualHigh (yStart)

        binWidth = estStart[2] - estEnd[2]

        return Block (binWidth = binWidth, startValue = yStart, endValue = yEnd, discrete = self._discrete, stepWidth = width, color=self._color, equalFloatMode = self.EqualFloatMode)
    
    def add (self, other):
        stepWidth = 0.0;
        discrete = False
        if self._discrete and other._discrete:
            discrete = True
            stepWidth = min (self._stepWidth, other._stepWidth)
            
        return Block (binWidth = self._binWidth + other._binWidth, startValue = self._startValue, stepWidth = stepWidth, endValue = self.endValue (), color = self._color, equalFloatMode = self.EqualFloatMode)


                      
    def __eq__(self, other):
        if isinstance(other, self.__class__): # 'other' is a block
            return self.binWidth () == other.binWidth () and self.startValue () == other.startValue () and self.endValue () == other.endValue ()
        return False
    
    def __str__ (self):
        return "("+str(self._startValue)+", ... B:"+str(self._binWidth)+" D:"+str(bool(self._discrete))+" S:"+str(self._stepWidth)+" ... "+str(self._endValue)+"]"




        
    
    

col = ['r','g','b','c','m','y']
        

def meanList (data):
    stdDevs = []
    mv = MeanVariance ()
    startData = data[0]
    endData = data[-1]
    lenData = len (data)
    for idx, x in enumerate (data):
        # interpolation = startData
        # if idx > 0:
        #     interpolation = interpolation + ((endData - startData)/(lenData-1)) * idx
        # adjustedValue = x - interpolation
        #mv += adjustedValue
        mv += x

        stdDevs.append (mv.stdDev ())
    return stdDevs







def bucket (data, **kwargs): 
    limit = kwargs.get ('limit',0.1)
    depth = kwargs.get ('depth',3)

    print "data : ",data
    currCol = 'r'
    try:
        currCol = col[int(ceil(depth))]
    except:
        pass

    lenData = len(data)

    distinctValues = set (data)
    stepWidth = 0
    isDiscrete = False
    if len (distinctValues) > 1: # contains more than one distinct value
        distinctValues = list (distinctValues)
        distinctValues.sort ()
        if len (distinctValues) == 2: # exactly two values
            isDiscrete = True
            stepWidth = distinctValues[1] - distinctValues[0]
        else:
            distances = [nxt - prv for prv, nxt in zip (distinctValues[:-1], distinctValues[1:])]
            stepWidth = min (distances)
            isDiscrete = not True in [isMultipleOf(d, stepWidth) for d in distances]
            if not isDiscrete:
                stepWidth = 0.0

    forward = { "mean" : [], "stdDev" : []}
    forward['stdDev'] = meanList (data)

    totalStdDev = forward["stdDev"][-1]

    
    if totalStdDev < limit  or depth < 1.0:
        print "block of data ",data,"   startValue = ",data[0], "   endValue = ",data[-1]
        return [Block (binWidth = lenData,  startValue = data[0], endValue = data[-1], color = currCol, discrete = isDiscrete, stepWidth = stepWidth)]

                
    backward = { "mean" : [], "stdDev" : []}
    backward['stdDev'] = list(reversed(meanList (list(reversed(data)))))

    
    sumVariance = []
    totalLength = lenData
    for idx in xrange (lenData):
        try:
#            sumVariance.append ((forward["stdDev"][idx] * idx + backward["stdDev"][idx+1] * (totalLength - idx))/totalLength)
            sumVariance.append ((forward["stdDev"][idx] * (idx+1) + backward["stdDev"][idx+1] * (totalLength - (idx+1)))/totalLength)
        except:
            sumVariance.append (forward["stdDev"][idx] * idx/totalLength)


    minIndex = sumVariance.index (min (sumVariance))
    #print "minIndex = ",minIndex,"  lenData= ",lenData,"  depth=",depth,"  data= ",data

    # if no split
    if minIndex == 0 or minIndex+1 == lenData:
        print "block of data ",data,"   startValue = ",data[0], "   endValue = ",data[-1]
        return [Block (binWidth = lenData, startValue = data[0], endValue = data[-1], color = currCol, discrete = isDiscrete, stepWidth = stepWidth)]

    # depth = depth -1
    leftDepth = (depth * (float(minIndex)/lenData))
    rightDepth = (depth * (float(lenData-minIndex))/lenData)
    # print "depth = ",depth,"  leftdepth = ",leftDepth,"  rightDepth = ",rightDepth,"  minIndex=",minIndex
    buckets = bucket (data[0:minIndex+1], limit=limit, depth=leftDepth)
    buckets.extend (bucket (data[minIndex+1:], limit=limit, depth=rightDepth))
    
    return buckets



def merge (adaptive, adaptiveOther):
    itBlock = iter (adaptive._blocks)
    itOther = iter (adaptiveOther._blocks)

    # fetch the first blocks of each
    objBlock = None
    objOther = None

    result = []
    while True:
        # if there is no remaining block, fetch the next one
        if not objBlock:
            try: 
                objBlock = next (itBlock)
            except StopIteration:
                pass

        if not objOther:
            try:
                objOther = next (itOther)
            except StopIteration:
                pass


        if not objBlock and not objOther:
            break

        if objBlock and not objOther:
            result.append (deepcopy (objBlock))
            objBlock = None
            continue

        if objOther and not objBlock:
            result.append (deepcopy (objOther))
            objOther = None
            continue
        
        if (objBlock.endValue () < objOther.startValue ()): # objBlock is strictly left of objOther
            result.append (deepcopy (objBlock))
            objBlock = None
        elif (objOther.endValue () < objBlock.startValue ()): # objOther is strictly left of objBlock
            result.append (deepcopy (objOther))
            objOther = None
        else:
            # the two blocks intersect
            # get the leftmost parts of the intersection

            # does one of the two blocks start earlier?
            if objBlock.startValue () < objOther.startValue (): # objBlock starts earlier
                # get this part of the block
                fromValue = objBlock.startValue ()
                toValue = objOther.startValue ()
                result.append (objBlock.sub (fromValue, toValue)) # extract the part of the block before the overlap
                objBlock = objBlock.sub (toValue, objBlock.endValue ()) # extract the overlapping part of the block
                continue # next step
            
            elif objOther.startValue () < objBlock.startValue (): # objOther starts earlier
                # get this part of the block
                fromValue = objOther.startValue ()
                toValue = objBlock.startValue ()
                result.append (objOther.sub (fromValue, toValue)) # extract the part of the block before the overlap
                objOther = objOther.sub (toValue, objOther.endValue ()) # extract the overlapping part of the block
                continue # next step

            else: # both start at the same value
                if objBlock.endValue () == objOther.endValue (): # both blocks end at the same position
                    result.append (objBlock.add (objOther))
                    objBlock = None
                    objOther = None
                    continue

                else: # blocks end at a different position
                    if objBlock.endValue () < objOther.endValue (): # block ends before other
                        result.append (deepcopy (objBlock.add (objOther.sub (objOther.startValue (), objBlock.endValue ()))))
                        objOther = objOther.sub (objBlock.endValue (), objOther.endValue ())
                        objBlock = None
                        continue
                    
                    else: # other ends before block
                        result.append (deepcopy (objOther.add (objBlock.sub (objBlock.startValue (), objOther.endValue ()))))
                        objBlock = objBlock.sub (objOther.endValue (), objBlock.endValue ())
                        objOther = None
                        continue
            
    return result;


        
class AdaptiveBinning:
    
    numUnbinned = 20

    
    def __init__(self, data, **kwargs): # sort=True/False (enable/disable sorting of the data)
        #data.sort ()
        self._doSort = kwargs.get ('sort', True)
        self._unbinned = []
        self._blocks = []
        self.__add__ (data)

    def __add__ (self, data):
        self._unbinned.extend (data)
        if self._doSort:
            self._unbinned.sort ()
        if len (self._unbinned) > AdaptiveBinning.numUnbinned:
            self.histogramify ()
        return self

    def histogramify (self):
        if len (self._unbinned) == 0:
            return
        blocks = bucket (self._unbinned, limit=0.05, depth=20)
        self._unbinned = []
        if len (self._blocks) == 0:
            self._blocks = blocks
        else:
            # merge histograms
            adaptiveOther = AdaptiveBinning ([], sort=True)
            adaptiveOther._blocks = blocks
            self._blocks = merge (self, adaptiveOther)

            
        
    def scale (self, factor):
        self.histogramify () # we need a histogram to be able to scale it
        
        for b in self._blocks:
            b.setBinWidth (b.binWidth () * factor)


    def count (self):
        cnt = 0
        for b in self._blocks:
            cnt += b.binWidth ()
        return cnt + len (self._unbinned)

    
    def value (self, x):
        self.histogramify () # we need a histogram to be able to interpolate
        
        lower = 0.0

        targetBlock = None
        for b in self._blocks:
            if x < lower:
                break;

            currBinWidth = b.binWidth ()
            if x < lower+currBinWidth:
                targetBlock = b;
                break;

            lower += currBinWidth

        if targetBlock:
            return targetBlock.interpolate (x-lower)
        
        return 0.0

    
    def minValue ():
        m = min ([min (b.startValue (), b.endValue ()) for b in self._blocks])
        return min (min (self._unbinned), m)
    
    def maxValue ():
        m = max ([max (b.startValue (), b.endValue ()) for b in self._blocks])
        return min (max (self._unbinned), m)
    
    
    def estimateLowEqualHigh (self, y):
        lower = 0.0
        higher = 0.0
        equal = 0.0

        if not self._doSort:
            return 0.0
        
        targetBlock = None
        for b in self._blocks:
            if b.startValue () > y:
                higher += b.binWidth ()
            elif b.endValue () < y:
                lower += b.binWidth ()
            elif not targetBlock:
                targetBlock = b

        if targetBlock:
            l,e,h = targetBlock.estimateLowEqualHigh (y)
            lower += l
            higher += h
            equal = e

        if len (self._unbinned) > 0:
            idx = bisect_left (self._unbinned, y)
            if idx:
                lower += idx
                higher += len (self._unbinned) - idx
                if idx < len (self._unbinned) and self._unbinned[idx] == y:
                    equal += 1
                    higher -= 1
            
        return lower,equal,higher
    
            
    def __str__ (self):
        s = ""
        for b in self._blocks:
            s += str (b)
        return s

        

    
def draw (data, **kwargs):
    #data [:] = data[::-1]
    maxLen = len(data)
    t = np.arange(0, maxLen, 1)

    fig = plt.figure(figsize=(12, 6))
    vax = fig.add_subplot(111)

    vax.plot(t, data, '-b^')
#    vax.vlines(t, [0], data, lw = 2, linestyles='dashed')
    vax.set_xlabel('index')
    vax.set_ylabel('value')
    vax.set_title('value')


    forward = { "mean" : [], "stdDev" : []}
    forward['stdDev'] = meanList (data)

    
    backward = { "mean" : [], "stdDev" : []}
        
    backward['stdDev'] = list(reversed(meanList (list(reversed(data)))))

    backward["stdDev"] = list(reversed(backward["stdDev"]))

    #plt.errorbar (t, forward["mean"], yerr = forward["stdDev"], capsize =7, color='r', linestyle = 'dotted')
    #plt.errorbar (t, backward["mean"], yerr = backward["stdDev"], capsize =7, color='g', linestyle = 'dotted')

    sumStdDev = []
    totalLength = float(len(forward["stdDev"]))
    for idx in xrange (len(forward["stdDev"])):
        try:
            sumStdDev.append ((forward["stdDev"][idx] * idx + backward["stdDev"][idx+1] * (totalLength - idx))/totalLength)
        except:
            sumStdDev.append (forward["stdDev"][idx] * idx/totalLength)
    
#    plt.errorbar (t, sumStdDev, yerr = [0]*len(sumStdDev), capsize =1, color='m', linestyle = 'dashdot')

    
    blocks = list (kwargs.get ('blocks', []))
    countEstimation = kwargs.get ('countEstimation', None)
    
    
    # Make the shaded region
    currentPosition = 0
    for block in blocks:
        nextPosition = currentPosition + block.binWidth ()
        #print "block sv = ",block.startValue (),"  ev = ",block.endValue ()
        #verts = [(currentPosition,0),(nextPosition,0),(nextPosition,block.interpolate (block.binWidth ()+1)),(currentPosition,block.interpolate (0))]
        verts = [(currentPosition,0),(nextPosition,0),(nextPosition,block.interpolate (block.binWidth ())),(currentPosition,block.interpolate (0))]
        #poly = Polygon(verts, facecolor='0.9', edgecolor='0.5', color='r', alpha=0.2)
        poly = Polygon(verts, color=block.color (), alpha=0.2)
        vax.add_patch(poly)
        currentPosition = nextPosition

    # for b in blocks:
    #     print b


    if countEstimation:
        xLow = []
        xInter = []
        xHigh = []
        y = []
        totCount = countEstimation.count ()

        origData = list(data)
        additionalData = zip (origData,origData[1:])
        additionalData = [(a+b)/2.0 for a,b in additionalData if a != b]
        extendedData = origData + additionalData
        #print "orig = ",origData
        #print "add  = ",additionalData
        extendedData = list (set (extendedData))
        extendedData.sort ()
        #print "ext  = ",extendedData

        extendedData = np.arange (0.0,11.0,0.2)
        
        for d in extendedData:
            y.append (d)
            low,equ,high = countEstimation.estimateLowEqualHigh (d)
            xLow.append (low)
            highCount = totCount - high
            xHigh.append (highCount)

        #idx = [x/2.0 for x in range (0, len (extendedData))]
        idx = [x/5.0 for x in range (0, int(countEstimation.count () * 5))]
        for i in idx:    
            xInter.append (countEstimation.value (i))
            
        plt.errorbar (xLow, y, yerr = [0]*len(y), capsize =1, color='r', linestyle = 'dashdot', fmt='o')
        plt.errorbar (xHigh, y, yerr = [0]*len(y), capsize =1, color='c', linestyle = 'dashdot', fmt='o')
        plt.errorbar (idx, xInter, yerr = [0]*len(idx), capsize =5, color='k', linestyle = '', fmt='x')

        #print "high ",zip (y,xHigh)
        
    plt.show()



    

def testDrawVarying ():
    #data = np.array ([8,5,4,2])
    data0 = rnd.exponential (10, 30)
    data1 = rnd.triangular (4, 5, 5, 20)
    data2 = rnd.triangular (10,11,11, 10)
    data3 = rnd.triangular (14,14,15, 5)
    data4 = rnd.triangular (0,1,1, 10)
    data = np.concatenate ((data0,data1,data2,data3,data4), axis=0)
    #data = data0

    data = rnd.beta (6,4,20)
    data = [10 * x -3 for x in data]

#    dataZero = rnd.triangular (0,0,1,1)
#    data0 = rnd.triangular (5, 5, 10, 100)
#    data1 = rnd.triangular (10, 20, 20, 100)
#    data2 = rnd.triangular (20, 21, 21, 0)
#    data = np.concatenate ((data0, data1, data2), axis=0)

#    data = rnd.triangular (0, 10, 15, 200)

#    data = np.concatenate ((dataZero,data), axis=0)

#    data = [1,1,2,3,4,4,4,4,4,5,5,5,7,7,7]
#    data = [2,2,2,4,4,6,8,10]
    data = [2,2,2,4,5,6,7,7,7,8]
#    data = [1,1,1,2,2,2,2,4,4,4,4]
    data = [-2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -0.0, -0.0, -0.0, -0.0, -0.0, -0.0, -0.0, -0.0, -0.0, -0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 4.0,4.0,4.0, 4.0, 5.0, 5.0, 5.0, 6.0, 6.0]

    #data = [sin(x/10.0) for x in xrange (0,200)]
    #print data
    #return data

    data = np.sort (data)
    #data = np.ceil (data, None)

    addData = [4,4,6,6,8,8,10,10]

    data = []
    addData = []
    
    data = [2,4]
    addData = [3]
    
    #blocks = bucket (data, limit=0.05, depth=3)
    countEstimation = AdaptiveBinning (list(data), sort=True)
    countEstimation.histogramify ()
    print str (countEstimation)

    np.concatenate ((data, addData), axis=0)

    countEstimation += addData
    countEstimation.histogramify ()
    print str (countEstimation)
    np.sort (data)

    blocks = countEstimation._blocks
    
    draw (data, blocks=blocks, countEstimation = countEstimation)
    #draw (data)


    

    
if __name__ == "__main__":
    testDrawVarying ()

    
        
