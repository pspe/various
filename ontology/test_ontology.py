## -*- coding: utf-8 -*-

#from __future__ import unicode_literals

import pygraphviz as pgv
from random import *
from uuid import *
#import inspect
import itertools



def unique (a):
    return list(set(a))

def intersect (a,b):
    return list(set(a) & set(b))

def union (a,b):
    return list(set(a) | set(b))





        




class RelationMatrix:

    def __init__ (self):
        self.elements = ({},{})

    def __add__ (self, relation):
        try:
            self.elements[0][relation[0]].append (relation[1])
        except:
            self.elements[0][relation[0]] = [relation[1]]

        try:
            self.elements[1][relation[1]].append (relation[0])
        except:
            self.elements[1][relation[1]] = [relation[0]]
        return self

    def _getVector (self, elements0, elements1, index0 = None, index1 = None):
        if not index0 and not index1:
            return elements0.keys ()
        if not index0:
            index0 = elements0.keys()
        if not index1:
            index1 = elements1.keys()
        filteredWithIndices = []
        for i in index1:
            filteredWithIndices.extend ([e for e in elements1[i] if e in index0])
        return sorted(set(filteredWithIndices))

    
    def getLeft (self, indexOnLeft = None, indexOnRight = None):
        return self._getVector (self.elements[0], self.elements[1], indexOnLeft, indexOnRight)

    def getRight (self, indexOnLeft = None, indexOnRight = None):
        return self._getVector (self.elements[1], self.elements[0], indexOnRight, indexOnLeft)

    def _getMatrix (self, elements, switchPositions, index0, index1 = None):
        mat = []

        if switchPositions:
            if not index1:
                for line in index0:
                    mat.extend ([(e,line) for e in elements[line]])
            else:
                for line in index0:
                    mat.extend ([(e,line) for e in elements[line] if e in index1])
        else:
            if not index1:
                for line in index0:
                    mat.extend ([(line,e) for e in elements[line]])
            else:
                for line in index0:
                    mat.extend ([(line,e) for e in elements[line] if e in index1])

        return sorted(set(mat))

    def getMatrix (self, switchPositions=False):
        return self._getMatrix (self.elements[0], switchPositions, self.elements[0].keys())

    def getMatrixFilterOnRight (self, indexOnRight, switchPositions=False):
        return self._getMatrix (self.elements[1], not switchPositions, indexOnRight)

    def getMatrixFilterOnLeft (self, indexOnLeft, switchPositions=False):
        return self._getMatrix (self.elements[0], switchPositions, indexOnLeft)

    def getMatrixFiltered (self, indexOnLeft, indexOnRight, switchPositions=False):
        if not indexOnLeft and not indexOnRight:
            return self.getMatrix (switchPositions)
        if not indexOnLeft:
            return self.getMatrixFilterOnRight (indexOnRight, switchPositions)
        if not indexOnRight:
            return self.getMatrixFilterOnLeft (indexOnLeft, switchPositions)
        return self._getMatrix (self.elements[0], switchPositions, indexOnLeft, indexOnRight)


    def existsRelation (self, relation):
        try:
            return relation[1] in self.elements[0][relation[0]]
        except:
            return False


    def existsLeft (self, key):
        return self.elements[0].has_key (key)

    def existsRight (self, key):
        return self.elements[1].has_key (key)







def logger (executeRequest):
    def wrapped(*args, **kwargs):
        global logGraph
        global edgeCount

        caller = 'start'
        try:
            caller = kwargs['caller'].name
            
        except:
            pass

        showOnly = ['name','title']
        
        myself = args[0]
        request = args[1]
        primaryKeySets = None
        inpInfo = []
        try:
            primaryKeySets = args[2]
            inpInfo = [ (m, [dict((k,v) for (k,v) in myself.getData(myself.getUID(m,v)).iteritems()  if k in showOnly) for v in n]) for (m,n) in primaryKeySets.iteritems()]
        except:
            pass

        condrel = ''
        try:
            condrel = myself.condition
        except:
            pass
        try:
            condrel = myself.relation
        except:
            pass


        if not myself.name in logGraph.nodes():
            logGraph.add_node (myself.name, shape='rectangle', color='blue', style='filled', fill='cornflowerblue', fontcolor='white', label=myself.name+'\\n'+str(condrel))

        edgeLabel = 'e%s'%(str(edgeCount))+str(request)
        for i in inpInfo:
            edgeLabel += '\\n'+str(i)

        edgeKey = None
        try:
            edgeKey = kwargs['multipleCalls']
            edge = logGraph.get_edge (caller, myself.name, key=edgeKey)
            edge.attr['label'] += '\\n'+edgeLabel
        except:
            logGraph.add_edge (caller, myself.name, color='black', label = edgeLabel, fontcolor='black', fontsize=8, key=edgeKey)
        edgeCount += 1


        ## print "IN | ",myself.name, condrel, request #, [args[0].getData(args[0].getUID(k,v)) for (k,v) in args[2].iteritems()]
        ## for i in inpInfo:
        ##     print i

        

        # ---------
        retval = executeRequest(*args, **kwargs)
        # ---------


        
        info = [ [dict((k,v) for (k,v) in myself.getData(myself.getUID(r,rval[idx])).iteritems() if k in showOnly) for idx,r in enumerate(request)] for rval in retval ]

        edgeLabel = 'e%s'%(str(edgeCount))
        for i in info:
            edgeLabel += '\\n'+str(i)

        edgeKey = None
        try:
            edgeKey = kwargs['multipleCalls']
            edge = logGraph.get_edge (myself.name, caller, key=edgeKey)
            edge.attr['label'] += '\\n'+edgeLabel
        except:
            logGraph.add_edge (myself.name, caller, color='red', label = edgeLabel, fontcolor='red', fontsize=8, key=edgeKey)
        edgeCount += 1

        ## print "OUT| ",myself.name, condrel
        ## for i in info:
        ##     print i
        ## print " "
        return retval
    return wrapped




class Node: #(NodeInterface):

    
    def __init__(self, data, source, concept, condition, dataSourceKey, nonConstants, name):
        """ A ontology start node

        data   -- the common data source (outside data; can be accessed via UIDs; access to all data)
        source -- specific data input 
        """
        self.source = source
        self.condition = condition
        self.name = name
        self.dataSourceKey = dataSourceKey
        self.nonConstants = nonConstants

        self.concept = concept
        
        self.data = data
        self.uidList = []

        
    def clear(self):
        self.uidList = []


    @logger
    def executeRequest(self, request, primaryKeySets = None, **prms):
        if primaryKeySets:
            if primaryKeySets.keys() == [self.dataSourceKey]:
                return [[x] for x in primaryKeySets[self.dataSourceKey]]
            else:
                raise Exception ('Node','executeRequest: dataSourceKey of primaryKeySets not known')

        if not request:
            return []

        if self.source is None:
            self.uidList = self._create()
        else:
            for uid in self.source:
                if self.condition is None or self._check(uid):
                    self.uidList.append (uid)
        return [[x] for x in range(0, len(self.uidList))]


    def _create (self):
        uidList = []
        for entry in self.data.values():
            uid = entry['uid']
            if  self.condition is None or (entry['type'] == self.concept and self._check(uid)):
                uidList.append (uid)
        return uidList
        


    def _check(self, uid):
        chk = ""

        entry = self.getData (uid)
        if self.nonConstants[0]:
            chk = chk + 'entry["' + self.condition[0] + '"]'
        else:
            chk = chk + str(self.condition[0])
            
        chk = chk+ self.condition[1]
        
        if self.nonConstants[1]:
            chk = chk + 'entry["' + self.condition[2] + '"]'
        else:
            chk = chk + str(self.condition[2])
            
        try:
            return eval (chk)
        except:
            return False

    def getData (self, uid):
        return self.data[uid]

    def getUID (self, dataSourceKey, primaryKey):
        return self.uidList[primaryKey]

    def getName (self):
        return self.name

    def getAccessibleDataSources (self):
        return [self.dataSourceKey]

    def nodeInfo (self):
        return { 'name' : self.getName(), 'condition/relation' : self.condition, 'sources' : None, 'dataSourceKeys' : self.dataSourceKey, 'data' : self.data, 'dataSources' : self.getAccessibleDataSources(), 'length' : str(len(self.uidList)) }
        





class LinearFilteredNode:

    
    def __init__(self, data, source, dataSourceKeys, condition, name):
        self.name = name
        self.source = source
        self.condition = condition
        self.primaryKeyList = []
        self.usedDataSourceKey = [dk for dk in dataSourceKeys if dk is not None][0]
        self.dataSourceKeyLocations = dataSourceKeys

        self.data = data

    def clear(self):
        self.primaryKeyList = []
        if self.source is not None:
            self.source.clear ()


    @logger
    def executeRequest (self, request, primaryKeyLists = None, **prms): # primaryKeyLists : {1 : set([233,4,55,6,7,18]), 3 : set([222,223,66,7,5])}
        if not request:
            return []

        
        if not self.primaryKeyList:
            self._createPrimaryKeyList (self.usedDataSourceKey)


        if self.usedDataSourceKey in request and len(request)==1: # usedDataSourceKey is the one and only element of request
            if primaryKeyLists is None:
                return [[x] for x in self.primaryKeyList]
            if primaryKeyLists and (len(primaryKeyLists)==1 and primaryKeyLists.has_key(self.usedDataSourceKey)):
                    return [[x] for x in list(intersect(self.primaryKeyList,primaryKeyLists[self.usedDataSourceKey]))]
        
        refinedPrimaryKeyLists = {}
        if primaryKeyLists:
            refinedPrimaryKeyLists = dict((k,v) for (k,v) in primaryKeyLists.iteritems())

        if refinedPrimaryKeyLists.has_key (self.usedDataSourceKey):
            refinedPrimaryKeyLists[self.usedDataSourceKey] = intersect(self.primaryKeyList,refinedPrimaryKeyLists[self.usedDataSourceKey])
        else:
            refinedPrimaryKeyLists[self.usedDataSourceKey] = set(self.primaryKeyList)
        return self.source.executeRequest (request, refinedPrimaryKeyLists, caller=self)

            

    def _createPrimaryKeyList (self, usedDataSourceKey):

        multipleCallsID = uuid4().hex
        for primaryKeyList in self.source.executeRequest ([usedDataSourceKey], caller=self, multipleCalls=multipleCallsID):
            uid = self.source.getUID(usedDataSourceKey, primaryKeyList[0])
            if self._check(self.dataSourceKeyLocations, uid):
                self.primaryKeyList.append (primaryKeyList[0])

    
    def _check(self, dataSourceKeyLocations, uid):
        entry = self.getData (uid)
        chk = ""

        idx = 0
        if dataSourceKeyLocations[0] == None: # is a constant
            chk += str(self.condition[0])
        else:
            chk += 'entry["%s"]'%(self.condition[0])
            idx += 1

        chk += ' '+self.condition[1]+' '

        if dataSourceKeyLocations[1] == None: # is a constant
            if type(self.condition[2])==str:
                chk += '"%s"'%(self.condition[2])
            else:
                chk += str(self.condition[2])
        else:
            chk += 'entry["%s"]'%(self.condition[2])
        
        return eval (chk)
            
    def getData (self, uid):
        return self.data[uid]

    def getUID (self, dataSourceIndex, primaryKey):
        return self.source.getUID (dataSourceIndex, primaryKey)

    def getName (self):
        return self.name

    def getAccessibleDataSources (self):
        return self.source.getAccessibleDataSources()

    def nodeInfo (self):
        return { 'name' : self.getName(), 'condition/relation' : self.condition, 'sources' : [self.source], 'dataSourceKeys' : self.usedDataSourceKey, 'data' : self.primaryKeyList, "dataSources" : self.getAccessibleDataSources(), 'length' : str(len(self.primaryKeyList))}






class FilteredNode:
    def __init__ (self, data, source, dataSourceKeys, condition, name):
        self.usedDataSourceKeys = dataSourceKeys
        if len(self.usedDataSourceKeys) > 2:
            raise Exception('FilteredNode', 'too many data source keys')
        self.condition = condition
        self.name = name
        self.conditionMatrix = None
        self.source = source

        self.data = data

    def clear (self):
        self.conditionMatrix = None
        if self.source is not None:
            self.source.clear ()

    @logger
    def executeRequest (self, request, primaryKeyLists = None, **prms): # primaryKeyLists : {1 : set([233,4,55,6,7,18]), 3 : set([222,223,66,7,5])}

        if not request:
            return []

        if self.conditionMatrix is None:
            self._createConditionMatrix (self.usedDataSourceKeys)

        if set(request).issubset (self.usedDataSourceKeys): # only directly available elements (from the condition matrix) are requested
            if primaryKeyLists is None:
                if len (request) == 2: # fetch full matrix
                    switchPositions = True
                    if request == self.usedDataSourceKeys:
                        switchPositions = False
                    return self.conditionMatrix.getMatrix (switchPositions)
                elif request[0] == self.usedDataSourceKeys[0]:
                    return [[x] for x in self.conditionMatrix.getLeft ()]
                elif request[0] == self.usedDataSourceKeys[1]:
                    return [[x] for x in self.conditionMatrix.getRight ()]
            elif set(primaryKeyLists.keys()).issubset (self.usedDataSourceKeys):
                leftPrimaryKeyList  = None
                rightPrimaryKeyList = None
                try:
                    leftPrimaryKeyList = primaryKeyLists[self.usedDataSourceKeys[0]]
                except:
                    pass

                try:
                    rightPrimaryKeyList = primaryKeyLists[self.usedDataSourceKeys[1]]
                except:
                    pass
                if len (request) == 2: # fetch full matrix
                    switchPositions = False
                    if request == [self.rightDependency, self.leftDependency]:
                        switchPositions = True
                    return self.conditionMatrix.getMatrixFiltered (leftPrimaryKeyList, rightPrimaryKeyList, switchPositions)
                elif request[0] == self.usedDataSourceKeys[0]:
                    return [[x] for x in self.conditionMatrix.getLeft (leftPrimaryKeyList, rightPrimaryKeyList)]
                elif request[0] == self.usedDataSourceKeys[1]:
                    return [[x] for x in self.conditionMatrix.getRight (leftPrimaryKeyList, rightPrimaryKeyList)]

                          


        ##### go through all entries in the matrix and request them one by one. Then cross the returned lists out completely.

        leftFilter  = None
        rightFilter = None
        try:
            leftFilter  = primaryKeyLists[self.leftDependency]
        except:
            pass

        try:
            rightFilter = primaryKeyLists[self.rightDependency]
        except:
            pass
        
        throughPutPrimaryKeyLists  = {}
        if primaryKeyLists:
            throughPutPrimaryKeyList  = dict((k,v) for (k,v) in primaryKeyLists.iteritems() if k in [self.source.getAccessibleDataSources() ] and not k in self.usedDataSourceKeys)

        
        # check if both dependencies of the matrix are requested, then the full matrix is needed
        # connect resultLeft and resultRight via the conditionMatrix
        conditionMatrix = self.conditionMatrix.getMatrixFiltered (leftFilter, rightFilter)
        resultList = []
        multipleCallsID = uuid4().hex
        for cnd in conditionMatrix:
            leftKey  = cnd[0]
            rightKey = cnd[1]

            tmpKeyList = dict(throughPutPrimaryKeyLists)
            tmpKeyList[self.usedDataSourceKeys[0]] = [leftKey]
            tmpKeyList[self.usedDataSourceKeys[1]] = [rightKey]
            resultFromExecute  = self.source.executeRequest  (request,  tmpKeyList, caller=self, multipleCalls=multipleCallsID)

            for r in resultFromExecute:
                resultList.append ( list(r) )

        return resultList


    def _createConditionMatrix (self, dataSourceKeys):
        self.conditionMatrix = RelationMatrix ()

        multipleCallsID = uuid4().hex
        for primaryKeySet in self.source.executeRequest (dataSourceKeys, caller=self, multipleCalls=multipleCallsID):
            leftUID  = self.source.getUID (dataSourceKeys[0], primaryKeySet[0])
            rightUID = self.source.getUID (dataSourceKeys[1], primaryKeySet[1])
            if self._check (leftUID, rightUID):
                self.conditionMatrix += primaryKeySet
            


    def _check(self, leftUID, rightUID):
        chk = ""

        leftData  = self.source.getData (leftUID )
        rightData = self.source.getData (rightUID)

        leftAttr  = leftData [str(self.condition[0])]
        rightAttr = rightData[str(self.condition[2])]

        if type(leftAttr)==str:
            leftAttr = '"'+leftAttr+'"'
        if type(rightAttr)==str:
            rightAttr = '"'+rightAttr+'"'

        return eval(leftAttr+self.condition[1]+rightAttr)


    def getData (self, uid):
        return self.data[uid]

    def getUID (self, dataSourceKey, primaryKey):
        return self.source.getUID (dataSourceKey, primaryKey)

    def getName (self):
        return self.name

    def getAccessibleDataSources (self):
        return self.source.getAccessibleDataSources ()

    def nodeInfo (self):
        lenLeft = -1
        lenRight = -1
        lenMat = -1
        try:
            lenLeft = len(self.conditionMatrix.getLeft())
            lenRight = len(self.conditionMatrix.getRight())
            lenMat = len(self.conditionMatrix.getMatrix())
        except:
            pass
        return { 'name' : self.getName(), 'condition/relation' : self.condition, 'sources' : [self.source], 'dataSourceKeys' : self.usedDataSourceKeys, 'data' : self.conditionMatrix, "dataSources" : self.getAccessibleDataSources(), 'length' : 'l:%s,r:%s,m:%s'%(str(lenLeft),str(lenRight),str(lenMat))}


    




class RelationNode:

    
    def __init__(self, data, leftSource, rightSource, leftDependency, rightDependency, condition, relation, name):
        self.name = name
        self.condition = condition
        self.relation  = relation

        self.rebuildLeft  = False
        self.rebuildRight = False

        self.leftSource = leftSource
        self.rightSource = rightSource
        if leftSource is None:
            self.rebuildLeft = True
            self.leftSource = NodeFactory.createGenerator (data, None, None, None, leftDependency, (True, False), makeInstanceName (leftDependency))
        if rightSource is None:
            self.rebuildRight = True
            self.rightSource = NodeFactory.createGenerator (data, None, None, None, rightDependency, (True, False), makeInstanceName (rightDependency))
            

        self.leftDependency = leftDependency
        self.rightDependency = rightDependency

        self.relationMatrix = None
        self.data = data

    def clear (self):
        self.relationMatrix = None
        if self.leftSource is not None:
            self.leftSource.clear ()
        if self.rightSource is not None:
            self.rightSource.clear()


    @logger
    def executeRequest (self, request, primaryKeyList = None, **prms):
        
        if not request:
            return []


        # create the relation matrix if not yet existing
        if self.relationMatrix is None:
            if not self.rebuildLeft and not self.rebuildRight and self.leftSource and self.rightSource:
                self._createRelationMatrix ()
            elif self.rebuildLeft and self.condition=='allRelations':
                self._createRelationMatrixFromRight ()
            elif self.rebuildRight and self.condition=='allRelations':
                self._createRelationMatrixFromLeft ()
            else:
                raise Exception ('RelationNode','executeRequest/else/not implemented')


        # check if there are primaryKeyLists which concern this RelationNode
        leftKeyList  = None
        rightKeyList = None
        try:
            leftKeyList = primaryKeyList[self.leftDependency]
        except:
            pass

        try:
            rightKeyList = primaryKeyList[self.rightDependency]
        except:
            pass


        # check if the request needs more data or it can be executed directly from the relationMatrix
        if len(request) > len(intersect(request, [self.leftDependency,self.rightDependency])):
            # there is a dataSourceKey needed which is not in the matrix
            toTheLeft  = self.relationMatrix.getLeft  (leftKeyList, rightKeyList)
            toTheRight = self.relationMatrix.getRight (leftKeyList, rightKeyList)

            leftRequest  = [rq for rq in request if rq in self.leftSource.getAccessibleDataSources() ]
            rightRequest = [rq for rq in request if rq in self.rightSource.getAccessibleDataSources()]

            leftPrimaryKeyList  = {}
            rightPrimaryKeyList = {}
            if primaryKeyList:
                leftPrimaryKeyList  = dict((k,v) for (k,v) in primaryKeyList.iteritems() if k in self.leftSource.getAccessibleDataSources()  and not k in [self.leftDependency ])
                rightPrimaryKeyList = dict((k,v) for (k,v) in primaryKeyList.iteritems() if k in self.rightSource.getAccessibleDataSources() and not k in [self.rightDependency])

            # check if both dependencies of the matrix are requested, then the full matrix is needed
            hasToGoLeft  = len(intersect(request, self.leftSource.getAccessibleDataSources ())) > 0
            hasToGoRight = len(intersect(request, self.rightSource.getAccessibleDataSources())) > 0
            if hasToGoLeft and hasToGoRight:
                # connect resultLeft and resultRight via the relationMatrix
                relations = self.relationMatrix.getMatrixFiltered (leftKeyList, rightKeyList)
                resultList = []
                
                multipleCallsID = uuid4().hex
                
                for relation in relations:
                    leftKey  = relation[0]
                    rightKey = relation[1]

                    leftTmpKeyList = dict(leftPrimaryKeyList)
                    leftTmpKeyList[self.leftDependency] = [leftKey]
                    resultListLeft  = self.leftSource.executeRequest  (leftRequest,  leftTmpKeyList, caller=self, multipleCalls=multipleCallsID)

                    rightTmpKeyList = dict(rightPrimaryKeyList)
                    rightTmpKeyList[self.rightDependency] = [rightKey]
                    resultListRight  = self.rightSource.executeRequest  (rightRequest,  rightTmpKeyList, caller=self, multipleCalls=multipleCallsID)

                    for l in resultListLeft:
                        for r in resultListRight:
                            result = []
                            for rq in request:
                                if rq in leftRequest:
                                    result.append (list(l)[leftRequest.index(rq)])
                                if rq in rightRequest:
                                    result.append (list(r)[rightRequest.index(rq)])
                            resultList.append ( result )

                return resultList

            else: # only one side of the matrix is needed
                if hasToGoLeft:
                    leftVector  = self.relationMatrix.getLeft (leftKeyList, rightKeyList)
                    leftTmpKeyList = dict((k,v) for (k,v) in primaryKeyList.iteritems() if k in self.leftSource.getAccessibleDataSources())
                    leftTmpKeyList[self.leftDependency] = leftVector
                    return self.leftSource.executeRequest (leftRequest, leftTmpKeyList, caller=self)

                elif hasToGoRight:
                    rightVector  = self.relationMatrix.getRight (rightKeyList, rightKeyList)
                    rightTmpKeyList = dict((k,v) for (k,v) in primaryKeyList.iteritems() if k in self.rightSource.getAccessibleDataSources())
                    rightTmpKeyList[self.rightDependency] = rightVector
                    return self.rightSource.executeRequest (rightRequest, rightTmpKeyList, caller=self)


                else:
                    raise Exception ('RelationNode', 'dont know what to do')


        else:
            # all dataSourceKeys needed are in the matrix
            if len(request) == 2:
                # we want (just) the full matrix
                switchPositions = False
                if request == [self.rightDependency, self.leftDependency]:
                    switchPositions = True
                return self.relationMatrix.getMatrixFiltered (leftKeyList, rightKeyList, switchPositions)
                
            elif request[0] == self.leftDependency:
                return [[e] for e in self.relationMatrix.getLeft (leftKeyList, rightKeyList)]
            
            elif request[0] == self.rightDependency:
                return [[e] for e in self.relationMatrix.getRight (leftKeyList, rightKeyList)]

            else:
                raise Exception ('RelationNode','executeRequest: cannot process request')
            
        raise Exception ('RelationNode','executeRequest: request %s could not be processed'%request)
        


    def _createRelationMatrix (self):

        self.relationMatrix = RelationMatrix ()

        leftResult  = self.leftSource.executeRequest  ([self.leftDependency ], caller=self)
        rightResult = self.rightSource.executeRequest ([self.rightDependency], caller=self)
        
        for leftKey in leftResult:
            for rightKey in rightKeysList:
                leftData  = self.leftSource.getData  (self.leftSource.getUID(self.leftDependency,  leftKey[0] ))
                rightData = self.rightSource.getData (self.rightSource.getUID(self.rightDependency, rightKey[0]))
                leftUID  = self.leftSource.getUID(self.leftDependency,  leftKey[0] )
                rightUID = self.rightSource.getUID(self.rightDependency, rightKey[0])
                if self._check (leftUID, rightUID):
                    self.relationMatrix += (leftKey[0],rightKey[0])


                    
    def _createRelationMatrixFromLeft (self):

        leftResult  = self.leftSource.executeRequest  ([self.leftDependency ], caller=self)

        tmpRelationMatrix = RelationMatrix()
        for leftKey in leftResult:
            leftUID      = self.leftSource.getUID(self.leftDependency,  leftKey[0] )
            rightUIDList = self._getRelatedUIDs (leftUID)
            for rightUID in rightUIDList:
                tmpRelationMatrix += (leftKey[0],rightUID)

        self.relationMatrix = RelationMatrix ()
        rightUIDSet = tmpRelationMatrix.getRight ()
        rightPrimaryKey = 0
        for rightUID in rightUIDSet:
            for leftKey in tmpRelationMatrix.getLeft (None, [rightUID]):
                self.relationMatrix += (leftKey,rightPrimaryKey)
            rightPrimaryKey += 1

        self.rightSource = NodeFactory.createGenerator (self.data, rightUIDSet, None, None, self.rightDependency, (True, False), makeInstanceName(self.rightDependency))
        self.rightSource.executeRequest ([self.rightDependency], caller=self)


    def _createRelationMatrixFromRight (self):

        rightResult  = self.rightSource.executeRequest  ([self.rightDependency ], caller=self)

        tmpRelationMatrix = RelationMatrix()
        for rightKey in rightResult:
            rightUID      = self.rightSource.getUID(self.rightDependency,  rightKey[0] )
            leftUIDList   = self._getRelatedUIDs (rightUID)
            for leftUID in leftUIDList:
                tmpRelationMatrix += (leftUID, rightKey[0])

        self.relationMatrix = RelationMatrix ()
        leftUIDSet = tmpRelationMatrix.getLeft ()
        leftPrimaryKey = 0
        for leftUID in leftUIDSet:
            for rightKey in tmpRelationMatrix.getRight (None, [leftUID]):
                self.relationMatrix += (leftPrimaryKey, rightKey[0])
            leftPrimaryKey += 1

        self.leftSource = NodeFactory.createGenerator (self.data, leftUIDSet, None, None, self.leftDependency, (True, False), makeInstanceName(dataSourceKey))
        self.leftSource.executeRequest ([self.leftDependency], caller=self)



    def _getRelatedUIDs (self, uid):
        return self.data[uid][self.relation]

            
    def _check(self, leftUID, rightUID):
        leftData  = self.leftSource.getData  (leftUID)
        return rightUID in leftData[self.relation]

    
        
        
    def getDataSourceKey (self):
        return self.leftSource.getDataSourceKey ()
    
    def getData (self, uid):
        return self.data[uid]

    def getUID (self, dataSourceKey, primaryKey):
        if dataSourceKey in self.leftSource.getAccessibleDataSources():
            return self.leftSource.getUID (dataSourceKey, primaryKey)
        elif dataSourceKey in self.rightSource.getAccessibleDataSources():
            return self.rightSource.getUID (dataSourceKey, primaryKey)
        raise Exception ('RelationNode', 'getData did not find dataSourceKey')

   
    def getName (self):
        return self.name

    def getAccessibleDataSources (self):
        ret = []
        if self.leftSource is not None:
            ret.extend(self.leftSource.getAccessibleDataSources())
        if self.rightSource is not None:
            ret.extend(self.rightSource.getAccessibleDataSources())
        return ret


    def nodeInfo (self):
        lenLeft = -1
        lenRight = -1
        lenMat = -1
        try:
            lenLeft = len(self.relationMatrix.getLeft())
            lenRight = len(self.relationMatrix.getRight())
            lenMat = len(self.relationMatrix.getMatrix())
        except:
            pass
        return { 'name' : self.getName(), 'condition/relation' : [self.condition, self.relation], 'dataSourceKeys' : [self.leftDependency, self.rightDependency],  'sources' : [self.leftSource, self.rightSource], 'data' : self.relationMatrix, 'length' : 'l:%s,r:%s,m:%s'%(str(lenLeft),str(lenRight),str(lenMat))}




class OrNode:

    def __init__(self, data, sources, dataSourceKeyMapping, name): # dataSourceKeyMapping = { newSrc0 : [oldSrc0,oldSrc1], newSrc1 : [oldSrc3,oldSrc4,oldSrc5], newSrc2 : [oldSrc1] ...}
        self.data = data

        self.sources = sources
        self.accessibleDataSources = list(set(itertools.chain(*dataSourceKeyMapping.values())))
        self.createdDataSources    = dataSourceKeyMapping.keys()
        self.dataSourceKeyMapping = dataSourceKeyMapping

        self.name = name

        self.uidList = []

        self.dataSourceKeyToSourceMapping = {}
        for key in self.accessibleDataSources:
            for src in self.sources:
                if key in src.getAccessibleDataSources():
                    self.dataSourceKeyToSourceMapping[key] = src
                    break

            

    def clear (self):
        self.uidList = []
        for source in self.sources:
            source.clear ()

    @logger
    def executeRequest (self, request, primaryKeyList = None, **prms):
        neededSourcesAnyOf = [self.dataSourceKeyMapping[rq] if self.dataSourceKeyMapping.has_key(rq) else None for rq in request]
        
        # execute requests with primary key lists on all sources
        uidSet = set()
        for source in self.sources:
            accessibleSources = source.getAccessibleDataSources ()
            if not set(self.dataSourceKeyMapping.keys ()).issubset (accessibleSources):
                raise Exception ('OrNode','requested data sources %s are not a subset of the accessible sources %s of the current source %s'%(request,accessibleSources,str(source)))
            neededSources = [src[0] if src else None for src in [filter(lambda s: s in accessibleSources, anyOf) for anyOf in neededSourcesAnyOf]]
            neededRequest = [rq for rq in neededSources if rq is not None] # get rid of the 'Nones'
            neededPrimaryKeyLists = None
            try:
                neededPrimaryKeyLists = dict((k,v) for (k,v) in primaryKeyList.iteritems() if k in neededRequest)
            except:
                pass
            
            result = source.executeRequest (neededRequest, neededPrimaryKeyLists, caller=self)
            uids = []
            for line in result:
                uidLine = []
                uids.append (uidLine)
                for idx,entry in enumerate(line):
                    uidLine.append (source.getUID(neededRequest[idx],entry))

            for u in uids:
                for idx,ns in enumerate(neededSources):
                    if ns is None:
                        r.insert (idx,None)

                unique = tuple(u) # make a hashable type
                uidSet.add (unique)

        self.uidList = list(uidSet)
        return [[x] for x in range(0, len(self.uidList))]
                                


    def getAccessibleDataSources (self):
        return set(self.accessibleDataSources + self.createdDataSources)
    
    def getData (self, uid):
        return self.data[uid]

    def getUID (self, dataSourceIndex, primaryKey):
        if dataSourceIndex not in self.createdDataSources:
            raise Exception ('NodeOr', "tried to fetch data source %d which has not been created (%s)"%(dataSourceIndex, self.createdDataSources))

        return self.uidList[primaryKey][self.createdDataSources.index(dataSourceIndex)]


    def getName (self):
        return self.name

    def nodeInfo (self):
        return { 'name' : self.getName(), 'condition/relation' : None, 'sources' : self.sources, 'dataSourceKeys' : self.createdDataSources, 'data' : self.uidList, 'length' : str(len(self.uidList))}





class NodeFactory:

    @classmethod
    def createGenerator (self, data, source, concept, condition, dataSourceKey, nonConstants, name):
        return Node (data, source, concept, condition, dataSourceKey, nonConstants, name)

    @classmethod
    def createFilter (self, data, source, dataSourceKeys, condition, name):
        # check if condition is between two variables or between a variable and a constant
        if self._checkIfComparesWithConstant (dataSourceKeys):
            return LinearFilteredNode (data, source, dataSourceKeys, condition, name)
        else:
            return FilteredNode (data, source, dataSourceKeys, condition, name)


    @classmethod
    def _checkIfComparesWithConstant (self, dataSourceKeys):
        return None in dataSourceKeys


    @classmethod
    def createRelation (self, data, leftSource, rightSource, leftDependency, rightDependency, condition, relation, name):
        return RelationNode (data, leftSource, rightSource, leftDependency, rightDependency, condition, relation, name)

    @classmethod
    def createOr (self, data, sources, dataSourceKeyMapping, name):
        return OrNode (data, sources, dataSourceKeyMapping, name) # dataSourceKeyMapping = { newSrc0 : [oldSrc0,oldSrc1], newSrc1 : [oldSrc3,oldSrc4,oldSrc5], newS





    
lastCreatedNodeNumber = -1


def makeInstanceName (dataSourceKey):
    global lastCreatedNodeNumber
    lastCreatedNodeNumber += 1
    return "n%d_%d"%(dataSourceKey,lastCreatedNodeNumber)





concepts = [ 'Person', 'Book' ]
relations = {
     'isAuthorOf' : { 'Person' : 'Book'    },
     'isEditedBy' : { 'Book'   : 'Person'  }
    }

ontology = { 'Concepts' : concepts, 'Relations' : relations }


persons = [
    { 'uid' : uuid4().hex, 'type' : 'Person', 'name' : 'Alex', 'age' : 30, 'hairColor' : 'brown', 'isAuthorOf' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Person', 'name' : 'Peter', 'age' : 36, 'hairColor' : 'brown', 'isAuthorOf' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Person', 'name' : 'Norbert', 'age' : 39, 'hairColor' : 'black', 'isAuthorOf' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Person', 'name' : 'Andreas', 'age' : 52, 'hairColor' : 'blonde', 'isAuthorOf' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Person', 'name' : 'Michael', 'age' : 66, 'hairColor' : 'red', 'isAuthorOf' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Person', 'name' : 'Max', 'age' : 55, 'hairColor' : 'green', 'isAuthorOf' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Person', 'name' : 'Franz', 'age' : 43, 'hairColor' : 'black', 'isAuthorOf' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Person', 'name' : 'Gustav', 'age' : 29, 'hairColor' : 'brown', 'isAuthorOf' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Person', 'name' : 'Joe', 'age' : 77, 'hairColor' : 'blonde', 'isAuthorOf' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Person', 'name' : 'Jorge', 'age' : 23, 'hairColor' : 'brown', 'isAuthorOf' : [] }
    ]


books = [
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'A journey', 'pages' : 500, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Long Book', 'pages' : 2000, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Short Book', 'pages' : 20, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Novel', 'pages' : 433, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Thoughts', 'pages' : 333, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Medium Book', 'pages' : 511, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Thick', 'pages' : 1500, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Thin', 'pages' : 200, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Fiction', 'pages' : 123, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Planet of the grapes', 'pages' : 50, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Emergency Exit', 'pages' : 999, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'programming python', 'pages' : 120, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'a physics engine', 'pages' : 1731, 'isEditedBy' : [] },
    { 'uid' : uuid4().hex, 'type' : 'Book',  'title' : 'Another Long Book', 'pages' : 2000, 'isEditedBy' : [] },
    ]

# add relations
persons[0]['isAuthorOf'].append (books[0]['uid'])
persons[1]['isAuthorOf'].append (books[1]['uid'])
persons[2]['isAuthorOf'].append (books[2]['uid'])
persons[3]['isAuthorOf'].append (books[3]['uid'])
persons[4]['isAuthorOf'].append (books[4]['uid'])
persons[5]['isAuthorOf'].append (books[5]['uid'])
persons[6]['isAuthorOf'].append (books[6]['uid'])
persons[7]['isAuthorOf'].append (books[7]['uid'])
persons[8]['isAuthorOf'].append (books[8]['uid'])
persons[9]['isAuthorOf'].append (books[9]['uid'])
persons[4]['isAuthorOf'].append (books[10]['uid'])
persons[3]['isAuthorOf'].append (books[11]['uid'])
persons[0]['isAuthorOf'].append (books[12]['uid'])

persons[1]['isAuthorOf'].append (books[13]['uid'])
persons[4]['isAuthorOf'].append (books[3]['uid'])
persons[5]['isAuthorOf'].append (books[3]['uid'])
persons[5]['isAuthorOf'].append (books[7]['uid'])
persons[5]['isAuthorOf'].append (books[8]['uid'])
persons[1]['isAuthorOf'].append (books[11]['uid'])



books[0]['isEditedBy'].append (persons[1]['uid'])
books[1]['isEditedBy'].append (persons[4]['uid'])
books[2]['isEditedBy'].append (persons[5]['uid'])
books[3]['isEditedBy'].append (persons[1]['uid'])
books[4]['isEditedBy'].append (persons[3]['uid'])
books[5]['isEditedBy'].append (persons[2]['uid'])
books[6]['isEditedBy'].append (persons[8]['uid'])
books[7]['isEditedBy'].append (persons[1]['uid'])
books[8]['isEditedBy'].append (persons[8]['uid'])
books[9]['isEditedBy'].append (persons[2]['uid'])
books[10]['isEditedBy'].append (persons[5]['uid'])
books[11]['isEditedBy'].append (persons[1]['uid'])
books[12]['isEditedBy'].append (persons[3]['uid'])
books[13]['isEditedBy'].append (persons[4]['uid'])





dataBase = {}
for entry in persons+books:
    dataBase[entry['uid']] = entry






TestTable_0 = [
    { 'treelevel' : 0, 'command' : '&', 'additionalID' : None, 'arg0' : None, 'arg1': None, 'path0' : [], 'path1' : []},
    { 'treelevel' : 1, 'command' : '>', 'additionalID' : None, 'arg0' : 'age', 'arg1': 25, 'path0' : ['Person'], 'path1' : []},
    { 'treelevel' : 1, 'command' : '<', 'additionalID' : None, 'arg0' : 'age', 'arg1': 66, 'path0' : ['Person'], 'path1' : []},
    { 'treelevel' : 1, 'command' : '<', 'additionalID' : None, 'arg0' : 'pages', 'arg1': 500, 'path0' : ['Person','isAuthorOf'], 'path1' : [] },
    { 'treelevel' : 1, 'command' : '!=', 'additionalID' : None, 'arg0' : 'hairColor', 'arg1': 'hairColor', 'path0' : ['Person'], 'path1' : ['Person','isAuthorOf','isEditedBy'] },
    { 'treelevel' : 1, 'command' : '>', 'additionalID' : None, 'arg0' : 'age', 'arg1': 50, 'path0' : ['Person','isAuthorOf','isEditedBy'], 'path1' : [] }

####     { 'treelevel' : 1, 'command' : 'notExists', 'additionalID' : None, 'arg0' : None, 'arg1': None, 'path0' : 'Person/isAuthorOf/isEditedBy', 'path1' : None }
####     { 'treelevel' : 1, 'command' : 'exists', 'additionalID' : None, 'arg0' : None, 'arg1': None, 'path0' : 'Person/isAuthorOf/isEditedBy', 'path1' : None }

    ]


TestTable_1 = [
    { 'treelevel' : 0, 'command' : '>', 'additionalID' : None, 'arg0' : 'age', 'arg1': 25, 'path0' : ['Person'], 'path1' : []},
    ]

TestTable_2 = [
    { 'treelevel' : 0, 'command' : '&', 'additionalID' : None, 'arg0' : None, 'arg1': None, 'path0' : [], 'path1' : []},
    { 'treelevel' : 1, 'command' : '>', 'additionalID' : None, 'arg0' : 'age', 'arg1': 50, 'path0' : ['Person'], 'path1' : []},
    ## { 'treelevel' : 1, 'command' : '<', 'additionalID' : None, 'arg0' : 'pages', 'arg1': 500, 'path0' : ['Person','isAuthorOf'], 'path1' : [] },
    { 'treelevel' : 1, 'command' : '!=', 'additionalID' : None, 'arg0' : 'hairColor', 'arg1': 'hairColor', 'path0' : ['Person'], 'path1' : ['Person','isAuthorOf','isEditedBy'] },
    ]

TestTable_3 = [
    { 'treelevel' : 0, 'command' : '&', 'additionalID' : None, 'arg0' : None, 'arg1': None, 'path0' : [], 'path1' : []},
    { 'treelevel' : 1, 'command' : '|', 'additionalID' : None, 'arg0' : None, 'arg1': None, 'path0' : [], 'path1' : []},
    { 'treelevel' : 2, 'command' : '<', 'additionalID' : None, 'arg0' : 'age', 'arg1': 30, 'path0' : ['Person'], 'path1' : []},
    { 'treelevel' : 2, 'command' : '>', 'additionalID' : None, 'arg0' : 'age', 'arg1': 60, 'path0' : ['Person'], 'path1' : []},
    { 'treelevel' : 1, 'command' : '!=', 'additionalID' : None, 'arg0' : 'hairColor', 'arg1': 'hairColor', 'path0' : ['Person'], 'path1' : ['Person','isAuthorOf','isEditedBy'] },
    ]

TestTable = [
    { 'treelevel' : 0, 'command' : '&', 'additionalID' : None, 'arg0' : None, 'arg1': None, 'path0' : [], 'path1' : []},
    { 'treelevel' : 1, 'command' : '>', 'additionalID' : None, 'arg0' : 'age', 'arg1': 30, 'path0' : ['Person'], 'path1' : []},
    { 'treelevel' : 1, 'command' : '|', 'additionalID' : None, 'arg0' : None, 'arg1': None, 'path0' : [], 'path1' : []},
    { 'treelevel' : 2, 'command' : '==', 'additionalID' : None, 'arg0' : 'hairColor', 'arg1': 'brown', 'path0' : ['Person'], 'path1' : []},
    { 'treelevel' : 2, 'command' : '==', 'additionalID' : None, 'arg0' : 'hairColor', 'arg1': 'blonde', 'path0' : ['Person'], 'path1' : []},
    { 'treelevel' : 2, 'command' : '==', 'additionalID' : None, 'arg0' : 'hairColor', 'arg1': 'green', 'path0' : ['Person'], 'path1' : []},
    { 'treelevel' : 1, 'command' : '!=', 'additionalID' : None, 'arg0' : 'hairColor', 'arg1': 'hairColor', 'path0' : ['Person'], 'path1' : ['Person','isAuthorOf','isEditedBy'] },
    ]



def makeNodeGraphFromTable (ontology, table, data, dataSourceKeyMapping = {}, sourceInstance = None):

    if   table[0]['command'] == '|':
        return makeNodeGraphFromTable_OR  (ontology, table, data, dataSourceKeyMapping, sourceInstance)
    elif table[0]['command'] == '&':
        return makeNodeGraphFromTable_AND (ontology, table, data, dataSourceKeyMapping, sourceInstance)
    elif table[0]['command'] == '!':
        pass
    elif table[0]['command'] == 'RULE':
        pass
    elif table[0]['command'] == '>' or table[0]['command'] == '<' or table[0]['command'] == '==' or table[0]['command'] == '!=':
        return makeNodeGraphFromTable_CONDITION (ontology, table, data, dataSourceKeyMapping, sourceInstance)
    

def makeNodeGraphFromTable_AND (ontology, table, data, dataSourceKeyMapping, sourceInstance):
    currentTreeLevel = table[0]['treelevel']

    instance = sourceInstance
    del table[0]
    while table and table[0]['treelevel'] > currentTreeLevel:
        instance = makeNodeGraphFromTable (ontology, table, data, dataSourceKeyMapping, instance)
        
    return instance

def makeNodeGraphFromTable_OR  (ontology, table, data, dataSourceKeyMapping, sourceInstance):
    currentTreeLevel = table[0]['treelevel']

    instances = []
    
    del table[0]
    while table and table[0]['treelevel'] > currentTreeLevel:
        instances.append (makeNodeGraphFromTable (ontology, table, data, dataSourceKeyMapping, sourceInstance))

    orKeyMapping = dict((k,[k]) for k in dataSourceKeyMapping.keys() if type(k)==int)
    return NodeFactory.createOr (data, instances, orKeyMapping, makeInstanceName(orKeyMapping.keys()[0]))


def hasDataSourceKey (dataSourceKeyMapping, path):
    pathString = '/'.join(path)
    return dataSourceKeyMapping.has_key(pathString)
    


def getDataSourceKey (dataSourceKeyMapping, path):
    pathString = '/'.join(path)
    try:
        return dataSourceKeyMapping[pathString]
    except:
        keys = [key for key in dataSourceKeyMapping if type(key)==int]
        keys.sort ()
        if not keys:
            dataSourceKeyMapping[pathString] = 0
            dataSourceKeyMapping[0] = pathString
            return 0
        newKey = keys[-1]+1
        dataSourceKeyMapping[pathString] = newKey
        dataSourceKeyMapping[newKey]     = pathString
        return newKey
        
    
        
    
    
def makeNodeGraphFromTable_CONDITION (ontology, table, data, dataSourceKeyMapping, sourceInstance):
    #    { 'treelevel' : 1, 'command' : '!=', 'additionalID' : None, 'arg0' : 'hairColor', 'arg1': 'hairColor', 'path0' : ['Person'], 'path1' : ['Person','isAuthorOf','isEditedBy'] },
    instance = sourceInstance

    leftDataSourceKey  = None
    rightDataSourceKey = None

    needsLeftRelation  = False
    needsRightRelation = False

    path0 = table[0]['path0']
    path1 = table[0]['path1']

    command   = table[0]['command']
    argument0 = table[0]['arg0']
    argument1 = table[0]['arg1']
    condition = (argument0,command,argument1)


    if sourceInstance is None:
        path0Source = None
        path1Source = None
        leftDataSourceKey  = None
        rightDataSourceKey = None
        nonConstants = [False,False]
        if path0:
            # get the concept of the relation target
            concept = path0[0]
            for relation in path0[1:]:
                concept = ontology['Relations'][relation][concept]
            
#            path0Source = [d['uid'] for d in data.values() if d['type']==concept]
            path0DataSourceKey = getDataSourceKey (dataSourceKeyMapping, path0)
            nonConstants[0] = True

        if path1:
            # get the concept of the relation target
            concept = path1[0]
            for relation in path1[1:]:
                concept = ontology['Relations'][relation][concept]
            
#            path1Source = [d['uid'] for d in data.values() if d['type']==concept]
            path1DataSourceKey = getDataSourceKey (dataSourceKeyMapping, path1)
            nonConstants[1] = True

        if (path0 == path1 or path1Source is None or path0Source is None):
            instance = NodeFactory.createGenerator (data, path0Source, concept, condition, path0DataSourceKey, nonConstants, makeInstanceName(path0DataSourceKey))
            del table[0]
            return instance

        else:
            raise Exception ('makeNodeGraphFromTable_CONDITION', 'generation of data from relation; not implemented yet')

        
    # build all not yet existing relations for the left side
    for idx in xrange(len(path0)):
        if hasDataSourceKey (dataSourceKeyMapping, path0[:idx+1]):
            continue # already existing

        # not yet existing, have to build it
        leftDataSourceKey  = getDataSourceKey (dataSourceKeyMapping, path0[:idx])
        rightDataSourceKey = getDataSourceKey (dataSourceKeyMapping, path0[:idx+1])
        relation = path0[idx]
        instance = NodeFactory.createRelation (data, instance, None, leftDataSourceKey, rightDataSourceKey, 'allRelations', relation, makeInstanceName(leftDataSourceKey))
        
                             
    # build all not yet existing relations for the right side
    for idx in xrange(len(path1)):
        if hasDataSourceKey (dataSourceKeyMapping, path1[:idx+1]):
            continue # already existing

        # not yet existing, have to build it
        leftDataSourceKey  = getDataSourceKey (dataSourceKeyMapping, path1[:idx])
        rightDataSourceKey = getDataSourceKey (dataSourceKeyMapping, path1[:idx+1])
        relation = path1[idx]
        instance = NodeFactory.createRelation (data, instance, None, leftDataSourceKey, rightDataSourceKey, 'allRelations', relation, makeInstanceName(leftDataSourceKey))


           

    # now treat the condition
    path0DataSourceKey = None
    path1DataSourceKey = None
    if hasDataSourceKey (dataSourceKeyMapping, path0):
        path0DataSourceKey = getDataSourceKey (dataSourceKeyMapping, path0)
    if hasDataSourceKey (dataSourceKeyMapping, path1):
        path1DataSourceKey = getDataSourceKey (dataSourceKeyMapping, path1)
    instance = NodeFactory.createFilter (data, instance, [path0DataSourceKey, path1DataSourceKey], condition, makeInstanceName(path0DataSourceKey))

    del table[0]
    return instance






def followGraph (G, lastInstance, whatToPrint):
    info = lastInstance.nodeInfo();

    nodeShape = 'rectangle'
    isDataSource = False
    if not info['sources']:
        nodeShape = 'ellipse'
        isDataSource = True
        

    nodeLabel = ''
    if 'info' in whatToPrint['node']:
        nodeLabel += info['name']+'\\n'+str(info['dataSourceKeys'])
    if 'condition/relation' in whatToPrint['node']:
        nodeLabel += '\\n'+str(info['condition/relation'])
    if 'matrix' in whatToPrint['node']:
        data = info['data']
        if data.__class__ == RelationMatrix:
            nodeLabel += ' R:'+str(data.elements[0])
    if 'length' in whatToPrint['node']:
        nodeLabel = nodeLabel + '\\nL:' + info['length']

    G.add_node (info['name'], shape=nodeShape, color='blue', label=nodeLabel)

    if not info['sources']:
        return;

    idx = 0
    for source in info['sources']:
        sourceInfo = source.nodeInfo()
        followGraph (G, source, whatToPrint)
        edgeLabel = info['name']+' -> '+sourceInfo['name']
        if len(info['sources']) > 1:
            if idx == 0:
                edgeLabel = edgeLabel + ' [l]'
            else:
                edgeLabel = edgeLabel + ' [r]'
        G.add_edge (info['name'], source.getName(), color='black', label = edgeLabel)
        idx += 1

    

def makeGraphFromNodeGraph (lastInstance, name, whatToPrint = {'node' : ['info','condition/relation'], 'edge' : ['info']}):
    G=pgv.AGraph(directed=True)

    followGraph (G, lastInstance, whatToPrint)
    
    G.graph_attr['label'] = name
    G.layout(prog='dot')
#    G.layout(prog='twopi')
    G.draw(name+'.png')


    




lastInstance =  makeNodeGraphFromTable (ontology, TestTable, dataBase)
makeGraphFromNodeGraph (lastInstance, 'instanceGraph')




for requestTest in [ [0], [1], [2], [0,1], [0,2], [1,2],  [0,1,2], [1,0], [2,0], [2,1], [2,0,1], [1,2,0], [0,2,1], [2,1,0], [1,0,2] ]:
#for requestTest in [ [0,1,2] ]:
    print " "
    print "_________________________________ request : "+str(requestTest)
    if not set(requestTest).issubset(lastInstance.getAccessibleDataSources ()):
        continue
    lastInstance.clear ()

    graphSpecifier = ''
    for x in requestTest:
        graphSpecifier += '_'+str(x)


    logGraph = pgv.AGraph(directed=True, strict=False)
    logGraph.graph_attr['label'] = 'logging_%s'%graphSpecifier
    logGraph.add_node ('start', shape='circle', color='red', style='filled', fill='red', fontcolor='white', )
    edgeCount = 0

    resultList = lastInstance.executeRequest (requestTest)

    logGraph.layout(prog='dot')
    #logGraph.layout(prog='circo')
    logGraph.draw(logGraph.graph_attr['label']+'.png')
    # twopi, gvcolor, wc, ccomps, tred, sccmap, fdp, circo, neato, acyclic, nop, gvpr, dot.


    whatToPrint = {'node' : ['length', 'matrix'], 'edge' : ['index']}
    graphName = 'instanceGraphAfterExecute_%s'%(graphSpecifier)
    makeGraphFromNodeGraph (lastInstance, graphName, whatToPrint)

    print "number results: ", len(resultList)
    for entry in resultList:
        for idx, primaryKey in enumerate(entry):
            dataSourceKey = requestTest[idx]
            dataSourceUID  = lastInstance.getUID  (dataSourceKey, primaryKey)
            dataSourceItem = lastInstance.getData (dataSourceUID)
            print "    dataSourceKey: ",dataSourceKey,
            print "uid: ", UUID(hex=dataSourceItem['uid']),
            if dataSourceKey == 0:
                print "  name: ", dataSourceItem['name'], "  age: ", dataSourceItem['age'], "  hairColor: ", dataSourceItem['hairColor']
            elif dataSourceKey == 1:
                print "    title: ", dataSourceItem['title'], "  pages: ", dataSourceItem['pages']
            elif dataSourceKey == 2:
                print "      name: ", dataSourceItem['name'], "  age: ", dataSourceItem['age'], "  hairColor: ", dataSourceItem['hairColor']
        print " "











