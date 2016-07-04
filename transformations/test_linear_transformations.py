#!/usr/bin/python


from math import *
from random import *
from copy import *
from polynomial import *
import sys
import operator
import copy
import cmath
from pprint import pprint
#from numpy import dot


epsilon = 1e-10
softEpsilon = 1e-5

def zeros(*shape):
    if len(shape) == 0:
        return 0
    car = shape[0]
    cdr = shape[1:]
    return [zeros(*cdr) for i in range(car)]


    


def createMatrix(angle, scaleA, scaleB, offset):
    matrix = [[0.0,0.0,0.0],[0.0,0.0,0.0],[0.0,0.0,0.0]]
    matrix[0][0] = cos(angle)*scaleA[0]*scaleB[0]
    matrix[0][1] = -sin(angle)*scaleA[0]*scaleB[1]
    matrix[0][2] = offset[0]*scaleA[0]
    matrix[1][0] = sin(angle)*scaleA[1]*scaleB[0]
    matrix[1][1] = cos(angle)*scaleA[1]*scaleB[1]
    matrix[1][2] = offset[1]*scaleA[1]
    matrix[2][0] = 0
    matrix[2][1] = 0
    matrix[2][2] = 1
    return matrix

def getSpecificEWEquationCoefficients(m):
    return [
        -m[0][0]*m[1][1]+m[1][0]*m[0][1],
        +m[0][0]*m[1][1]+m[0][0]+m[1][1]-m[0][1]*m[1][0],
        -m[0][0]-m[1][1]-1
         ]
         



def polyfy(m):
    for i in xrange(len(m)):
        for j in xrange(len(m[i])):
            m[i][j] = Polynomial(m[i][j])


def trace (m):
    tr = 0
    for i in xrange(len(m)):
        tr += m[i][i]
    return tr

def determinant(m):
#    print "len=",len(m)
#    printMatrix(m)
    if len(m)==2:
        det = m[0][0]*m[1][1]-m[1][0]*m[0][1]
#        print "DET=",det
        return det
    det = 0.0
    i = 0
    for j in xrange(len(m)):
        sub = deepcopy(m)
        for l in xrange(len(m)):
            del sub[l][j:j+1]
        del sub[i:i+1]
        sgn = ((-1.0)**(i+j))
        factor = m[i][j]
        detSub = determinant(sub)
#        print "sgn=",sgn,"   factor=",m[i][j],"  ",detSub
        det += sgn * factor * detSub
#    print "det=",det
    return det


def eigenwertEquation(m):
    polyfy(m)
    for i in xrange(len(m)):
        m[i][i] -= 'x'
    return determinant(m)




def toReducedRowEchelonForm( M):
    print "toreduced"
#    printMatrix(M)
    if not M: return
    lead = 0
    rowCount = len(M)
    columnCount = len(M[0])
    for r in range(rowCount):
        if lead >= columnCount:
            return
        i = r
        while abs(M[i][lead])<epsilon:
            i += 1
            if i == rowCount:
                i = r
                lead += 1
                if columnCount == lead:
                    return
        M[i],M[r] = M[r],M[i]
        lv = M[r][lead]
        M[r] = [ mrx / lv for mrx in M[r]]
        for i in range(rowCount):
            if i != r:
                lv = M[i][lead]
                M[i] = [ iv - lv*rv for rv,iv in zip(M[r],M[i])]
#            printMatrix(M)
        lead += 1
    return M
    

def einheitsmatrix(cols, rows):
    m = []
    for i in xrange(rows):
        m.append([])
        for j in xrange(cols):
            if i == j:
                m[i].append (1)
            else:
                m[i].append (0)
    return m

def inverseMatrix (m):
    IM = copy.deepcopy(m)
    cols = len(IM[0])
    rows = len(IM)

    E = einheitsmatrix(cols,rows)
#    printMatrix(E)
#    printMatrix (IM)
    
    for r in xrange(rows):
#        print "E[r] ",E[r]
#        print "m[r] ",m[r]
        IM[r].extend(E[r])
    
#    printMatrix (IM)

    toReducedRowEchelonForm (IM)
    for idx, row in enumerate(IM):
        IM[idx] = row[-cols:]
    

    return IM



def polynomial(coefficientsIn, x):
    coefficients = coefficientsIn
    if type(coefficientsIn) == type(Polynomial):
        coefficients = coefficientsIn.coefficients()
#    print coefficients
        
    value = complex(0,0)
    for order in xrange(len(coefficients)):
        value += coefficients[order]* x**order
    return value

def printRoots(coefficients,roots):
    idx = 0
    for r in roots:
        polyValue = polynomial(coefficients,roots[idx])
        print "(%.2f + %.2fj) = %.2f + %.2fj"%(r.real,r.imag,polyValue.real,polyValue.imag), "    ",
        idx = idx + 1

    print " "


def _solvePoly(coefficients, roots, index):
    x = roots[index]
    denominator = complex(1.0,0.0)
    for i in xrange(len(roots)):
        if i==index:
            continue
        denominator *= (x-roots[i])

    if abs(denominator-0)< 0.00001:
        print "WARNING, denominator==0"

    poly = polynomial(coefficients,x)
    return x-(poly/denominator)


eps = 1e-5 # max error allowed
epsMax = 1e5 # max deviation allowed
def solvePoly(coefficients):
    # Durand-Kerner method
    n = len(coefficients)
    print n
    roots = [complex(0.4,0.9) **i for i in xrange(n)]
    printRoots (coefficients,roots)
#    sys.exit()

    keepRunning = True
    while keepRunning:
        keepRunning = False
        roots_1 = [_solvePoly(coefficients,roots,i) for i in xrange(len(roots))]
        for i in xrange(len(roots)):
            absDiff = abs(roots[i]-roots_1[i])
            if  absDiff > eps and absDiff<epsMax:
                print "deviation,",i,"=",abs(roots[i]-roots_1[i]),"  with ",roots[i],"  and ", roots_1[i]
                keepRunning = True
        roots = roots_1
        printRoots (coefficients,roots)

    return roots
    

def subMat (mat, colFrom, colTo, rowFrom, rowTo):
    m = zeros(colTo-colFrom+1, rowTo-rowFrom+1)
    for r in xrange(rowFrom,rowTo):
        row = mat[r]
        for c in xrange(colFrom,colTo):
            m[r-rowFrom][c-colFrom] = row[c]
    return m


def mul_scalar_vec(s, v):
     #multiplication of scalar with a vector'
     return [x*s for x in v]

def mul_scalar_mat(s, m):
     #multiplication of scalar with a matrix'
     for index, item in enumerate (m):
         if isinstance (item, list):
             mul_scalar_mat (s, item)
         else:
             m[index] *= s
     return m


def inner_prod(v1, v2):
     #inner production of two vectors.'
     total = 0
#     print "v1 : ",v1
#     print "v2 : ",v2
     for i in xrange(len(v1)):
            total += v1[i] * v2[i]
     return total

def mulMatVec(m,v):
    #matrix multiply vector by inner production.'
#    print "mul m:",m
#    print "mul v:",v
    return [inner_prod(r, v) for r in m]


#def mulMatVec(m,v):
#    vec = [0.0]*len(m)
#    for i in xrange(len(m)):
#        for j in xrange(len(m[i])):
#            vec[i] += m[i][j]*v[j]
#    return vec

# def mulMatMat(m0,m1):
#     ret = zeros(len(m0),len(m1[0]))
#     for i in xrange(len(m0)):
#         for j in xrange(len(m1[0])):
#             ret[i][j] = sum([m0[i][k]*m1[k][j] for k in xrange(len(m0[i]))])
# #            print "i=",i," j=",j," ret=",ret[i][j]
#     return ret


#     vec = [0]*len(v)
#     for i in xrange(len(m)):
#         for j in xrange(len(m[i])):
#             vec[i] = m[i][j]*v[j]
#     return vec



 
def mulMatMat(A, B):
    TB = zip(*B)
    return [[sum(ea*eb for ea,eb in zip(a,b)) for b in TB] for a in A]
 
def pivotize(m):
    """Creates the pivoting matrix for m."""
    n = len(m)
    ID = [[float(i == j) for i in xrange(n)] for j in xrange(n)]
    for j in xrange(n):
        row = max(xrange(j, n), key=lambda i: abs(m[i][j]))
        if j != row:
            ID[j], ID[row] = ID[row], ID[j]
    return ID
 
def LU_decomposition(A):
    """Decomposes a nxn matrix A by PA=LU and returns L, U and P."""
    n = len(A)
    L = [[0.0] * n for i in xrange(n)]
    U = [[0.0] * n for i in xrange(n)]
    P = pivotize(A)
    A2 = mulMatMat(P, A)
    for j in xrange(n):
        L[j][j] = 1.0
        for i in xrange(j+1):
            s1 = sum(U[k][j] * L[i][k] for k in xrange(i))
            U[i][j] = A2[i][j] - s1
        for i in xrange(j, n):
            s2 = sum(U[k][j] * L[i][k] for k in xrange(j))
            L[i][j] = (A2[i][j] - s2) / U[j][j]
    return (L, U, P)
 
 
def LUsolve((L,U,P),b):
    n = len(b)
    v = zeros(n)
    while True:
        return false
        



def subVecVec(v0,v1):
    return map(lambda a,b:a-b,v0,v1)



def subMatMat(m0, m1):
     #subtraction of matrix from matrix'
     for index, item in enumerate (m0):
         if isinstance (item, list):
             subMatMat (item, m1[index])
         else:
             m0[index] -= m1[index]
     return m0

def addMatMat(m0, m1):
     #subtraction of matrix from matrix'
     for index, item in enumerate (m0):
         if isinstance (item, list):
             addMatMat (item, m1[index])
         else:
             m0[index] += m1[index]
     return m0


def absVec(v):
    val = 0
    for i in xrange(len(v)):
        val += (v[i])**2
    return sqrt(val)

def dotProduct(v0,v1):
    product = sum([v0[i]*v1[i] for i in xrange(len(v0))])
#    print "<",["%.2f"%x for x in v0],"|",["%.2f"%x for x in v1],"> = %.2f"%product 
    return product



def norm(vector):
    return sqrt(dotProduct(vector,vector))

def unit(vector):
    n = norm(vector)
    return [x/n for x in vector]

def transpose(m):
#    print "transpose: "
#    printMatrix (m)
    try:
        checklen = len(m[0])
    except:
        return [[x] for x in m]
    ret = zip(*m)
    for idx, line in enumerate(ret):
        ret[idx] = list(line)
#    print "transposed "
#    printMatrix (ret)
    return ret


def row(m,r):
    return m[r]

def col(m,c):
    ret = []
    for line in m:
        ret.append(line[c])
    return ret

def setElement(m, col, row, value):
    m[row][col] = value

def getElement(m, col, row):
    return m[row][col]


def setCol(m,col,vect):
    for idxLine in xrange(len(vect)):
        setElement(m, col, idxLine, vect[idxLine])

def setRow(m,row,vect):
    m[row] = vect

def removeRow(m, row):
    ret = copy.deepcopy(m)
    del ret[row]
    return ret

def removeCol(m, col):
    ret = copy.deepcopy(m)
    for line in ret:
        del line[col]
    return ret



## def arnoldi2(M, uInput, numK):
##     print "arnoldi"
##     print "M: ",M
##     print "uInput: ",uInput
##     print "numK: ",numK
##     assert (len(uInput)==len(M))
##     u = zeros(len(uInput),numK+1)
##     h = zeros(numK+1,len(uInput))

##     setCol (u,0, unit(uInput))

##     for j in xrange(numK):
##         w = mulMatVec(M,col(u,j))

##         u_ = removeCol(u,len(u[0])-1)

##         hj = mulMatVec(transpose(u_),w)
##         setCol(h,j,    hj)
##         _uj1 = subVecVec (w, mulMatVec(u_, hj) )

##         norm_uj1 = norm(_uj1)
##         setElement (h,j,j+1,  norm_uj1 ) 
##         setCol (u,j+1,   [_uj1[i]/norm_uj1 for i in xrange(len(_uj1))] )
    
##     print "h"
##     printMatrix (h)
##     print "u"
##     printMatrix (u)
    
##     print "v-norm"
##     printMatrix(mulMatMat(u,h))
##     printMatrix(mulMatMat(M,u))

##     return u, h


def isUpperHessenberg (mat):
    for i in xrange(len(mat)):
        line = mat[i]
        for j in xrange(i-1):
            if abs(mat[i][j]) > epsilon:
                return False
    return True
        
def arnoldi(A, b, k):
    print "arnoldi3"
    print "A: "
    printMatrix (A)
    if isUpperHessenberg (A):
        return einheitsmatrix(len(A),len(A[0])), A
    print "b: ",b
    print "k: ",k
    assert (len(b)==len(A))
    H = zeros(k+1,k)
    Q = zeros(len(b), k+1)

    setCol (Q,0, unit(b))
    for n in xrange(k):
#        print "---- n: ",n
        v = mulMatVec(A,col(Q,n))
        for j in xrange(n+1):
            hjn = inner_prod(col(Q,j), v)
            H[j][n] = hjn
            _v = mul_scalar_vec (hjn, col(Q,j))
            v = subVecVec (v, _v)
        hn1n = norm(v)
        H[n+1][n] = hn1n
        
#        if n is not k-1:
        setCol(Q,n+1, mul_scalar_vec(1.0/hn1n, v))
        printMatrix (H, "H")
        printMatrix (Q, "Q")

    print "H"
    printMatrix (H)
    print "Q"
    printMatrix (Q)
    
    print "v-norm"
    printMatrix(mulMatMat(Q,H))
    printMatrix(mulMatMat(A,Q))

    printMatrix (H)
    hessenberg = removeRow(H,len(H)-1)
    return Q, hessenberg
    





## from scipy import zeros, dot, random, mat, linalg, diag, sqrt, sum, hstack, ones
## from scipy.linalg import norm, eig

## def multMv(A,v):
##     return A*v


## def randomvector(N):
##     v = random.random(N)
##     n = sqrt( sum(v*v) )
##     return mat(v/n).T * 1L


## def arnoldi(A, v0, k):
##     """
## Arnoldi algorithm (Krylov approximation of a matrix)
## input:
## A: matrix to approximate
## v0: initial vector (should be in matrix form)
## k: number of Krylov steps
## output:
## V: matrix (large, N*k) containing the orthogonal vectors
## H: matrix (small, k*k) containing the Krylov approximation of A

## Author: Vasile Gradinaru, 14.12.2007 (Rennes)
## """
##     #print 'ARNOLDI METHOD'
##     A = mat (A)
##     v0 = randomvector(k)
##     inputtype = A.dtype.type
##     V = mat( v0.copy() / norm(v0), dtype=inputtype)
##     H = mat( zeros((k+1,k), dtype=inputtype) )
##     print ("A: ")
##     print A
##     print ("V: ")
##     print V
##     print ("H: ")
##     print H
##     for m in xrange(k):
##         vt = A*V[ :, m]
##         for j in xrange( m+1):
##             H[ j, m] = (V[ :, j].H * vt )[0,0]
##             vt -= H[ j, m] * V[:, j]
##         H[ m+1, m] = norm(vt);
##         if m is not k-1:
##             V = hstack( (V, vt.copy() / H[ m+1, m] ) )

##     print V
##     print "H"
##     print H
##     print H[:-1,:].tolist()
##     return V.tolist(), H.tolist()


        







## def hypot(x, y):
##     return abs(x)*sqrt(1+(y/x)**2)

def hypot(x, y):
    x = abs(x)
    y = abs(y)
    s = 0
    r = 0
    if x>= y:
        if x+y == x:
            s = x
        else:
            r = y/x
            s = x*sqrt(1.0+r**2)
    else:
        assert(y!=0.0)
        r = x/y
        s = y*sqrt (1.0+r**2)
    return s



def sgn(x):
    if x >= 0:
        return 1.0
    if x < 0:
        return -1.0


def givensRotation(matrix, i, j):
    if (i<0):
        tmp = i
        i = j
        j = tmp
    r = hypot (matrix[j][j], matrix[i][j])
    c = matrix[j][j]/r
    s = matrix[i][j]/r
    g = zeros(len(matrix),len(matrix[0]))
    for k in xrange(len(matrix)):
        if not k==i and not k==j and k < len(matrix[0]):
            setElement(g,k,k,1)

    setElement(g,i,i,c)
    setElement(g,j,j,c)
    setElement(g,j,i,-s)
    setElement(g,i,j,s)
    return g



def givensRotation2(matrix, i, j):
    if (i<0):
        tmp = i
        i = j
        j = tmp

    g = zeros(len(matrix),len(matrix))
    for k in xrange(len(g)):
#        if not k==i and not k==j and k < len(g[0]):
            setElement(g,k,k,1.0)

    a = matrix[i-1][j]
    b = matrix[i][j]

    c = 0.0
    s = 0.0
    r = 0.0
    if b==0:
        c = 1.0
        s = 0.0
    else:
        if abs(b) > abs(a):
            r = a/b
            s = 1.0/sqrt(1.0+r**2)
            c = s*r
        else:
            r = b/a
            c = 1.0/sqrt(1.0+r**2)
            s = c*r
#    print "r ",r, " c ",c, " s ",s
    setElement(g,i,i,c)
    setElement(g,j,j,c)
    setElement(g,j,i,-s)
    setElement(g,i,j,s)
    return g



    




def QRDecomposition(mat): 
    # using givens rotation
    R = copy.deepcopy(mat)
    m = len(R)
    n = len(R[0])
    Q = einheitsmatrix(m,n)
#    print "n ",n, " m",m
#    printMatrix (R,"R in qrdeco")
#    printMatrix (Q,"Q in qrdeco")
    emptyQ = True
    for j in xrange(n):
        print "j: ",j, " range ",range(m-1,j,-1)
        for i in xrange(m-1,j,-1):
            print "i: ",i
            if (abs(getElement(R, j, i))<=softEpsilon):
                continue
            g = givensRotation2(R,i,j)
            printMatrix (g, "g")
            if emptyQ:
                Q = transpose(g)
                emptyQ = False
            else:
                Q = mulMatMat (Q,transpose(g))
            R = mulMatMat(g,R)
    return Q, R


## def eigenValues2(M):
##     print "eigenvalues"
##     initVector = [1.0]
##     initVector.extend ([2.0]*(len(M)-1))


## #    initVector = list(xrange(len(M)))
## #    initVector = [0.1+x for x in initVector]
##     initVector = unit(initVector)
##     print ("len(initVector) : ",len(initVector),"  len(M) ",len(M))
##     u, hessenberg = arnoldi(M,initVector,len(M))
    
##     hessenberg = removeRow(hessenberg,len(hessenberg)-1)
##     A = hessenberg

##     printMatrix (A,"A")

##     count = 50
##     while count>0:
##         # should implement "shifts" (wiklinson shift) for stability and convergence speed   TODO: _ps_
##         # without shifts QR decomposition does only converge if the eigenvalues are like that: |L1|>|L2|>|L3|
##         # it does not converge for eigenvalues |L1|==|L2| etc.

##         # no shifts
##         #Q,R = QRDecomposition(A)
##         #A = mulMatMat(R,Q)

##         # rayleigh shift
##         shift = A[len(A)-1][len(A)-1]
##         I = einheitsmatrix(len(A),len(A))
##         Ishift = mul_scalar_mat (shift, I)
##         AminI = copy.deepcopy (A)
##         AminI = subMatMat (AminI, Ishift)
## #        printMatrix (AminI, "AminI")
##         Q,R = QRDecomposition(AminI)
##         A = addMatMat (mulMatMat(R,Q), Ishift)

##         printMatrix(A,"hopefully converging matrix A")
##         count = count -1
    
##     eigenvalues = []
##     complexEigenvalues = []

## #    return

##     # check if there are complex eigenvalues 
##     # if there are complex eigenvalues
##     # then they are the eigenvalues of diagonal 2x2 
##     # sub matrices
##     for i in xrange(len(A)):
##         if i+1 < len(A) and abs(getElement(A,i,i+1))>softEpsilon: # is part of submatrix for complex eigenvalues
##             B = M[i:i+2]
##             for j, line in enumerate(B):
##                 B[j] = line[i:i+2]
##             complexEigenvalues.append(eigenValues(B)[0])
##             i = i+1
##         else:
##             ev = A[i][i]
##             if abs(ev) < epsilon:
##                 ev = 0
##             eigenvalues.append (ev)
            
##     return eigenvalues, complexEigenvalues



def eigenValues2x2(M):
    tr = trace (M)
    det = determinant (M)
    lmbd = []
    toSqrt = tr**2 - 4*det
    arg = cmath.sqrt (toSqrt)
    lmbd = []
    lmbdCmplx = []
    if toSqrt < 0:
        c0 = (tr+arg)/2
        c1 = (tr-arg)/2
        lmbdCmplx.append ([c0.real, c0.imag])
        lmbdCmplx.append ([c1.real, c1.imag])
    else:
        lmbd.append ((tr+arg.real)/2)
        lmbd.append ((tr-arg.real)/2)
    return lmbd, lmbdCmplx


def eigenValues(M):
    if len(M) == 2 and len(M[0]) == 2:
        return eigenValues2x2 (M)
    
    print "eigenvalues"
    initVector = [1.0]
    initVector.extend ([2.0]*(len(M)-1))


#    initVector = list(xrange(len(M)))
#    initVector = [0.1+x for x in initVector]
    initVector = unit(initVector)
    print ("len(initVector) : ",len(initVector),"  len(M) ",len(M))
    u, hessenberg = arnoldi(M,initVector,len(M))
    printMatrix (u,"Q")
    printMatrix (hessenberg,"H")

    A = hessenberg

    printMatrix (A,"A")

    eigenvalues = []
    complexEigenvalues = []

    maxM = len(A)-1
    for m in range(maxM, 0, -1):
        print "m: ",m
        count = 20
        while True:
            # rayleigh shift 
            shift = A[m][m]
            # | check wilkinson shift --> does not yet work right | check "implicit QR"
            ## delta = (A[m-1][m-1] - A[m][m])/2.0
            ## shift = A[m][m] - ((sgn(delta)* (A[m-1][m])**2) / (abs(delta)+sqrt(delta**2 + (A[m-1][m])**2)))
            print "shift : ",shift

            I = einheitsmatrix(len(A),len(A))
            Ishift = mul_scalar_mat (shift, I)
            AsubI = subMatMat (copy.deepcopy (A), Ishift)
            printMatrix (AsubI, "AsubI")
            Q,R = QRDecomposition(AsubI)
            
            printMatrix (Q,"Q")
            printMatrix (R,"R")
            A = addMatMat (mulMatMat(R,Q), Ishift)
            printMatrix (A, "Ak")

            count = count -1

            amm1 = A[m][m-1]
            print "amm1: ",amm1

            if abs(amm1) <= softEpsilon:
                # eigenwert and deflation
                amm = A[m][m]
                if abs(amm) <= epsilon:
                    amm = 0
                eigenvalues.insert (0, amm)
                print "eigenvalues ",eigenvalues
                break
               
            elif count <= 0:
                # eigenwert is complex
                sub = subMat (A, m-1, m, m-1,m)
                subEigen = eigenValues(sub)
                eigenvalues.extend (subEigen[0])
                complexEigenvalues.extend (subEigen[1])
                
                break
                
        printMatrix(A,"A")


    amm = A[0][0]
    if abs(amm) <= epsilon:
        amm = 0
    eigenvalues.insert (0, amm)
    return eigenvalues, complexEigenvalues

    # check if there are complex eigenvalues 
    # if there are complex eigenvalues
    # then they are the eigenvalues of diagonal 2x2 
    # sub matrices
    for i in xrange(len(A)):
        if i+1 < len(A) and abs(getElement(A,i,i+1))>softEpsilon: # is part of submatrix for complex eigenvalues
            B = M[i:i+2]
            for j, line in enumerate(B):
                B[j] = line[i:i+2]
            complexEigenvalues.append(eigenValues(B)[0])
            i = i+1
        else:
            ev = A[i][i]
            if abs(ev) < epsilon:
                ev = 0
            eigenvalues.append (ev)
            
    return eigenvalues, complexEigenvalues



def SVD (A): # https://en.wikibooks.org/wiki/Data_mining_Algorithms_In_R/Dimensionality_Reduction/Singular_Value_Decomposition
    AAT = mulMatMat (A, transpose (A))
    ev, cEv = eigenValues (AAT)
    singularValues = [sqrt(x) for x in ev]
    singularValues.sort (reverse=True)
    return singularValues


def powerMethod (M, tolerance = 1e-6):
    v = zeros(len(M))
    v = [1.0 for x in v]
    v = unit (v)
    print v
    while True:
        vn = unit (mulMatVec (M, v))
        idx = vn.index (max (vn, key=abs)) #avoid oscillations between + and - version of the eigenvector
        factor = cmp (vn[idx],0)
        print "vn w/o: ",vn," factor: ",factor,"   idx ",idx
        vn = [factor*x for x in vn]
#        vn[idx] *= factor
        print "vn w/ : ",vn
        diff2 = 0.0
        for x, xn in zip (v,vn):
            diff2 += (x-xn)**2
        diff = sqrt (diff2)
        v = vn
        if diff < tolerance:
            break
    return v


def projection (vj, vi):
    return mul_scalar_vec (inner_prod (vi, vj)/norm (vi), vi)


def gramSchmidt (vectorList):
    result = []
    for i, iVec in enumerate(vectorList):
        #iv = unit (iVec)
        iv = iVec
        for j, jVec in enumerate(vectorList[i+1:]):
            projVec = projection(jVec,iVec)
            iv = map(lambda x,y : x-y, iv, projVec)
        result.append (iv)
    return result


def powerMethodSeveral (M, vectList, tolerance = 1e-6):
    vectList = gramSchmidt (vectList)
    Mvect = zip(*vectList)
    Mvect_m1 = None
    while True:
        Mvect_m1 = deepcopy(Mvect)
        Mvect = mulMatMat (M, Mvect)
        vectList = gramSchmidt (zip(*Mvect))
        Mvect = zip(*vectList)
        print "Mvec ",Mvect
        diff = 0.0
        for i in xrange (len(Mvect)):
            for j in xrange (len(Mvect[0])):
                diff += (Mvect_m1[i][j] - Mvect[i][j])**2
        print "diff: ",diff
        if diff < tolerance:
            break
    
    return Mvect



def inversePowerMethod (M, tolerance = 1e-6):
    L_U = LUdecomp (M)
    v = zeros(len(M))
    v = [1.0 for x in v]
    v = unit (v)
    while True:
        v_old = deepcopy (v)
        print "inv pow method: ",v
        v = LUsolve (L_U, v)
        vn = unit (v)
        sign = 1.0
        if inner_prod (v_old,vn) < 0.0: # change of sign
            sign = -1.0
            vn = mul_scalar_vec (-1.0, vn)
        v = vn
        diff2 = 0.0
        for x, xn in zip (v,v_old):
            diff2 += (x-xn)**2
        diff = sqrt (diff2)
        v_old = v
        if diff < tolerance:
            break
    return v


def eigenVectors (M, eigenvals):
    # I = einheitsmatrix(len(M),len(M))
    # for idx in xrange (len(I)):
    #     if eigenvals[idx] == 0:
    #         I[idx][idx] = 1.0
    #     else:
    #         I[idx][idx] = eigenvals[idx]
    # Evec = powerMethodSeveral (M, I)
    # print "Evec ",Evec
    # print "M*Evec ",mulMatMat (M, Evec)
    # print "Evec*I ",mulMatMat (Evec,I)
    # return Evec
    
    eigenvecs = []
    for ev in eigenvals:
        print "current EV ",ev
        I = einheitsmatrix(len(M),len(M))
        Ishift = mul_scalar_mat (ev**2, I)
        MsubI = subMatMat (copy.deepcopy (M), Ishift)
        print "M : ",M
        print "Ishift : ",Ishift
        print "MsubI : ",MsubI
#        eigenvecs.append (powerMethod (MsubI))
        eigenvecs.append (inversePowerMethod (MsubI))
    return eigenvecs



def printVector(name,vector):
    print name,": ",
    for entry in xrange(len(vector)):
      print "%.2f   "%(vector[entry]),
    print " "


def printMatrix(matrix, label = ""):
    print label," "
    for line in matrix:
        print " ",
        for entry in xrange(len(line)):
            print "%.7f   "%(line[entry]),
        print " "
    print ""
    return matrix


def printConstituents(angle,scaleA,scaleB,offset):
    print "angle=",angle,"  scaleA=",scaleA,"   scaleB=",scaleB,"  offset=",offset
        
def compareConstituents(angle,scaleA,scaleB,offset, angle2,scaleA2,scaleB2,offset2):
    print "angle=%.2f|%.2f"%(angle,angle2),"   scaleA=(%.2f,%.2f)|(%.2f,%.2f)"%(scaleA[0],scaleA[1],scaleA2[0],scaleA2[1]),"   scaleB=(%.2f,%.2f)|(%.2f,%.2f)"%(scaleB[0],scaleB[1],scaleB2[0],scaleB2[1]),"    offset=(%.2f,%.2f)|(%.2f,%.2f)"%(offset[0],offset[1],offset2[0],offset2[1])

def getConstituents(m):
    tanAngle2 = -m[0][1]*m[1][0]/(m[0][0]*m[1][1])
    tanAngle = sqrt(tanAngle2)
    angle = atan(tanAngle)
    scaleA = (-m[1][1]/sin(angle),
               m[1][1]*m[1][1]/(m[0][1]*cos(angle)))
    scaleB = (m[1][0]*m[0][1]/(m[1][1]*m[1][1]*tanAngle),
              -tanAngle2*m[0][0]*m[1][1]/(m[1][0]*m[0][1]))
    offset = (m[0][2]/(-m[1][1]*sin(angle)),
              m[1][2]*m[0][1]/(m[1][1]*m[1][1]*cos(angle)))
    
    return (angle, scaleA, scaleB, offset)


def test_linear_transformations(count):
    factor = 1.0
    maxOffset = 5.0

    while count > 0:
        angle = pi*random()/4
        scaleA = (factor*random(),factor*random())
        scaleB = (factor*random(),factor*random())
        offset = (2*maxOffset*random()-maxOffset, 2*maxOffset*random()-maxOffset)

        matrix = createMatrix(angle,scaleA,scaleB,offset)
#        matrix = [[2.0,-3.0,1.0],
#                  [3.0,1.0,3.0],
#                  [-5.0,2.0,-4.0]]
        matrix = [[0.83867,-0.5446,2.94031],[0.54463,0.83867,13.8330],[0.0,0.0,1.0]]
        cpMatrix = deepcopy(matrix)
#        angle2,scaleA2,scaleB2,offset2 = getConstituents(matrix)

        printMatrix(cpMatrix)
        sCoeff = getSpecificEWEquationCoefficients(cpMatrix)
        print "specific: ",sCoeff
        eweq = eigenwertEquation(cpMatrix)
        print eweq
#        sys.exit()
        print eweq.coefficients()

#        roots = solvePoly(eweq.coefficients())
        roots = solvePoly(sCoeff)
        print "Eigenvalues=",
        printRoots (eweq.coefficients(),roots)
#        subtractEigenValueFromMatrix(cpMatrix,l)
#        printMatrix(cpMatrix)
#        toReducedRowEchelonForm(cpMatrix)
#        printMatrix(cpMatrix)
#        compareConstituents(angle,scaleA,scaleB,offset,angle2,scaleA2,scaleB2,offset2)

        count = count -1

#test_linear_transformations(1)



mt = [[complex(1.0),2.0,3.0],
      [4.0,5.0,6.0],
      [7.0,8.0,9.0]]
#mt = [[1.0,2.0,3.0],
#      [4.0,5.0,6.0],
#      [7.0,8.0,9.0]]
#mt = [[complex(1.0,3.0),2.0,3.0,4.0],
#      [4.0,5.0,6.0,7.0],
#      [7.0,8.0,9.0,10.0],
#      [11.0,12.0,13.0,14.0]]

#m0 = [[1,2],[3,4]]
#m1 = [[5,6],[7,8]]
#printMatrix(m0)
#printMatrix(m1)
#printMatrix(mulMatMat(m0,m1))


mtest = [[3.5488, 15.593, 8.5775, -4.0123],
         [2.3595, 24.524, 14.596, -5.8157],
         [0.0899, 27.599, 21.438, -5.8415],
         [1.9227, 55.667, 39.717, -10.558]]

printMatrix (mtest)
#printMatrix ( transpose(mtest))

#inverseMatrix (mtest)
#print arnoldi3(mtest,[1.0]*len(mtest),len(mtest))
#print "arnoldi"
#arn = arnoldi(mtest,[1.0,0.0,0.0,0.0],len(mtest))
#print "QR decomposition of the original matrix"
## v0 = [4.0,1.0,8.0,7.0]
## V,H = arnoldi (mtest, v0, 4)
## hessenberg = removeRow(H,len(H)-1)
#Q,R = QRDecomposition(H)
#printMatrix(Q,"Q: ")
#printMatrix(R,"R: ")
#printMatrix(mulMatMat(Q,R),"Q.R")

#eigenValues (mtest)



#print "QR decomposition of the hessenberg form"
#hessenbergForm = arn[1]
#hessenbergForm = removeRow(hessenbergForm,len(hessenbergForm)-1)
#Q,R = QRDecomposition(hessenbergForm)
#printMatrix(Q,"Q:")
#printMatrix(R,"R: ")

mtest2 = [[1.0,5.0,4.0],
          [-2.0, 1.0, 3.0],
          [2.0,0.0,-2.0]]

## mtest3 = [[10.0, -1.0, 2.0, 0.0],
##           [-1.0, 11.0, -1.0, 3.0],
##           [2.0, -1.0, 10.0, -1.0],
##           [0.0,3.0,-1.0,8.0]]

## v0 = [4.0,1.0,8.0,7.0]

## arnoldi (mtest3, v0, 4)

#print "givens rotation"
#giv = givensRotation(mtest2,1,0)
#printMatrix(giv)

#print "arnoldi"
#marn = arnoldi(mtest2,[1.0,0.0,0.0],len(mtest2))
#printMatrix(marn[1])
#print "qr decomposition"
#Q,R = QRDecomposition(marn[1][:-1])
#printMatrix(Q,"Q:")
#printMatrix(R,"R: ")
#Q,R = QRDecomposition(mtest2)
#printMatrix (R)
#printMatrix (mulMatMat(Q,R))


## m = [[1.0,0.0,5.0],[0.0,1.0,0.0],[0.0,0.0,1.0]]
## m = [[3.0,-2.0],[4.0,-1.0]]
## alpha = 0.3
## m = [[cos(alpha),-sin(alpha)],[sin(alpha),cos(alpha)]]
## m = [[0.0,0.0,1.0],[0.0,1.0,0.0],[1.0,0.0,0.0]]
## m = [[cos(alpha),-sin(alpha)],[sin(alpha),cos(alpha)],[1.0,2.0],[3.0,4.0], [5.0,6.0],[7.0,9.0]]
## #m = [[0.5,0.5],[0.5,0.5]]
## #m = [[1.0,0.0,0.0,0.0],[0.0,1.0,0.0,0.0],[0.0,0.0,1.0,0.0],[0.0,1.0,0.0,1.0]]
## #print "isupperhessenberg : ",isUpperHessenberg (m)
## #exit(0)
    
## ev, complev = eigenValues (m)
## print "eigenvalues : ",ev, complev

Q = [[-0.5774, 0.7071, 0.4082],[-0.5774, 0.0, -0.8165],[-0.5774, -0.7071, 0.4082]]
R = [[-1.7321, -25.9808], [0.0, -7.0711], [0.0, 0.0]]

A = mulMatMat (Q, R)

#A = [[1.0, 10.0],[1.0, 15.0], [1.0, 20.0], [1.0,3.0], [7.0, -4.0]]

print "==================="
A = [[1.0, 10.0],[1.0, 15.0], [1.0, 20.0]]
printMatrix (A, "A")
Q, R = QRDecomposition (A)
printMatrix (Q,"Q")
printMatrix (R,"R")
printMatrix (mulMatMat(Q,R), "Q*R")
#ev, complev = eigenValues (A)
#print "eigenvalues : ",ev, complev
       
#coeff = [complex(1.0),complex(-3.0),complex(+3.0),complex(-5.0)]       
#coeff = [complex(-5, 0), complex(3, 0),complex(-3, 0),complex(1, 0)]     
#printRoots( coeff, solvePoly(coeff))
#print polynomial(coeff,complex(2.587,0))
#print " --- " 
#printRoots( coeff, DurandKerner(coeff))
      
#matrix = [[ 0.0333333, 0.2, 0.0,1.0,0.0,0.0 ],  [ -0.1, 0.25, 300.0,0.0,1.0,0.0 ],  [ -0.000305556, 0.000166667, 1.0, 0.0,0.0,1.0 ]]

#matrix = [[ 0.133333, 0.1, 0.0, 1.0, 0.0, 0.0 ],  [ -0.133333, -0.1, 1200.0, 0.0, 1.0, 0.0 ],  [ -0.000111111, -0.000166667, 1.0, 0.0, 0.0, 1.0 ]]
#print matrix
#printMatrix( toReducedRowEchelonForm(matrix))

# [ -810, -810, 9000 ],  [ 497, 497, -12000 ],  [ 0.000833333, 0.000833333, -1.21972e-18 ], 
#ma = [[1,2,3,4],[5,6,7,8]]
#mb = [[1,2],[3,4],[5,6],[7,8]]

#print "ma: ", ma
#print "mb: ", mb
#print mulMatMat(ma,mb)






## v = [[1.0],[2.0],[3.0], [4.0]]
## g = givensRotation2 (v, 2, 0)
## printMatrix (g, "givens rot")


## print "================"
## v = [[3.0, 2.0, 1.0],[2.0, -3.0, 4.0],[5.0, 1.0, -1.0], [7.0, 4.0, 2.0]]
## printMatrix (v, "v")
## g = givensRotation2 (v, 3, 0)
## printMatrix (g, "givens rot")
## printMatrix (mulMatMat (g, v), "g*v")

## print "================"
## v = [[4.0],[3.0]]
## printMatrix (v, "v")
## g = givensRotation2 (v, 1, 0)
## printMatrix (g, "givens rot")
## printMatrix (mulMatMat (g, v), "g*v")

## print "================"
## v = [[6.0, 5.0, 0.0],[5.0, 1.0, 4.0],[0.0, 4.0, 3.0]]
## printMatrix (v, "v")
## g = givensRotation2 (v, 1, 0)
## printMatrix (g, "givens rot")
## v = mulMatMat (g, v)
## printMatrix (v, "g*v")
## g = givensRotation2 (v, 2, 1)
## printMatrix (g, "givens rot")
## v = mulMatMat (g, v)
## printMatrix (v, "g*v")

print "------------------------ SVD -------------------------------------------"
m = [[1.0, 1.0, 0.0, 2.0], [1.0, 1.0, 2.0, 0.0], [2.0, 0.0, 1.0, 1.0]]
m = [[1.0, 10.0],[1.0, 15.0], [1.0, 20.0]]
# m = [[1.0, 0.0, 0.0, 0.0, 2.0],
#      [0.0, 0.0, 3.0, 0.0, 0.0],
#      [0.0, 0.0, 0.0, 0.0, 0.0],
#      [0.0, 4.0, 0.0, 0.0, 0.0]]
    
sv = SVD (m)
#sv = filter (lambda x: x != 0, sv)
print "SVD : ",sv

print "M MT"
mmT = mulMatMat (m, transpose (m))
for l in mmT:
    print l

print "eigenvectors of mmT"
ev_mmT = eigenVectors (mmT, sv)
for row in ev_mmT:
    print row
vecs = [[1.0,0.0,0.0]]
vecs.extend (ev_mmT[:2])
print "gram schmidt of ev ",vecs
print gramSchmidt (vecs)

print "MT M"
mTm = mulMatMat (transpose (m),m)
for l in mTm:
    print l


vecs = [[1.0,0.0,0.0],[0.0,1.0,0.0],[0.0,0.0,1.0]]
print gramSchmidt (vecs)




