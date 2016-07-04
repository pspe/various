

m = [
[0,0,0,0,0,0],
]

def makeMatrix(sizeX, sizeY, value = 0):
#    print "sizeX: ",sizeX
#    print "sizeY: ",sizeY
    mat = []
    for i in xrange(sizeY):
#        print "i:",i
        mat.extend([[]]);
        for j in xrange(sizeX):
#            print "j:",j
            mat[i].extend([value])
    return mat

def addPattern(matrix):
    # add pattern
    matrix[3][3]=2
    matrix[3][4]=2
    matrix[4][4]=2
    matrix[4][5]=2
    matrix[4][6]=2
    matrix[4][7]=2
    matrix[3][8]=2
    matrix[2][8]=2
    matrix[1][7]=2
    matrix[1][6]=2
    matrix[1][5]=2
    matrix[1][4]=2
    matrix[2][4]=2

    matrix[3][7]=2
    matrix[1][8]=2

    return matrix



directions = [
[1,0],
[-1,0],
[0,1],
[0,-1],
[-1,-1],
[-1,1],
[1,-1],
[1,1]
]

def printMatrix(matrix):
    for line in matrix:
        for element in line:
            if element == 0:
                print " "," ",
            else:
                print element," ",
        print " "
    print " " 

def printQueues(next,queue,last,done, sizeX, sizeY):
    matrix = makeMatrix(sizeX,sizeY,0)

    for e in next:
       matrix[e[1]][e[0]] = 1
    for e in queue:
       matrix[e[1]][e[0]] = 2
    for e in last:
       matrix[e[1]][e[0]] = 3
    for e in done:
       if matrix[e[1]][e[0]] >=4:
           matrix[e[1]][e[0]] = matrix[e[1]][e[0]]+1
       else:
           matrix[e[1]][e[0]] = 4
    # for e in next:
    #     matrix[e[1]][e[0]] = matrix[e[1]][e[0]]+1
    # for e in queue:
    #     matrix[e[1]][e[0]] = matrix[e[1]][e[0]]+1
    # for e in last:
    #     matrix[e[1]][e[0]] = matrix[e[1]][e[0]]+1
    # for e in done:
    #     matrix[e[1]][e[0]] = matrix[e[1]][e[0]]+1

    printMatrix(matrix)

def floodFill(matrix, start):
    printMatrix (matrix)

    queue = [start]
    done  = []
    last  = []
    
    next = []

    while len(queue)>0:
        for entry in queue:
            for d in directions:
               newEntry = [entry[0]+d[0],entry[1]+d[1]]
#               print newEntry
               if newEntry in queue:
                   continue
               if newEntry in last:
                   continue
#               if newEntry in done:
#                   continue
               if newEntry in next:
                   continue
               if newEntry[0]<0 or newEntry[0]>=len(matrix):
                   continue
               if newEntry[1]<0 or newEntry[1]>=len(matrix[0]):
                   continue
               
               if (matrix[newEntry[1]][newEntry[0]] == 0):
                   next.extend([newEntry])
        printQueues(next, queue, last, done, len(matrix[0]), len(matrix))

        for e in last:
            done.extend([e])
        last = []
        for e in queue:
            last.extend([e])
        queue = []
        for entry in next:
            queue.extend([entry])
        next = []
        



def run():
    floodFill(addPattern(makeMatrix(15,15)),[13,7])
