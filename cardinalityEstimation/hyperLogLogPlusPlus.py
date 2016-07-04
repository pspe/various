#import hashlib

import hllppparameters


class FORMAT:
    SPARSE, NORMAL = range(2)





    
class HLLpp:
    def __init__ (self, **kwargs):
        self._precision_sparse = int (kwargs.get ('precision_sparse', 32)) # <= 64
        self._precision = int (kwargs.get ('precision', 4)) # [4 .. precision_sparse]

        self._m = pow (2, self._precision)
        self._m_sparse = pow (2, self._precision_sparse)

        self._alpha_16 = 0.673
        self._alpha_32 = 0.697
        self._alpha_64 = 0.709

        self._alpha_m = 0.7213/(1.0+1.079/self._m) # for m >= 128
        self._format = FORMAT.SPARSE

        self._tmp_set = set ()
        self._sparse_list = []

        self._mask_index = int (pow (2, self._precision)-1)
        self._mask_w = int (pow (2, self._precision_sparse)-1) - self._mask_index

        self._M = {}

        self._max_len_set = 10


        self._threshold = { 4 : 10, 4 : 20, 6 : 40, 7 : 80, 8 : 220, 9 : 400, 10 : 900, 11: 1800, 12 : 3100, 13 : 6500, 14 : 11500, 15 : 20000, 16 : 50000, 17 : 120000, 18 : 350000}
        

    def hash (self, x):
        return hash (x)

    def estimator (self, value):
        for b in xrange (64):
            m = 1 << (63-b)
            if value&m != 0:
                return b
            
    def aggregate (self, data):
        for element in data:
            self.add (element)

    def add (self, element):
        x = self.hash (element)
        
        if self._format == FORMAT.NORMAL:
            idx = element & self._mask_index
            w = element & self._mask_w
            _M[idx] = max (_M.get (idx,0), estimator (w))
            
        elif self._format == FORMAT.SPARSE:
            k = encodeHash (x, self._precision, self._precision_sparse)
            self._tmp_set.add (k)
            if len (self._tmp_set) > self.max_len_set:
                self._sparse_list = self.merge (self._sparse_list, self._tmp_set)
                self._tmp_set = set ()
                if len (self._sparse_list) > self._m * 6:
                    self._format = FORMAT.NORMAL
                    _M = self.toNormal (self._sparse_list)

    def cardinality (self):
        if self._format == FORMAT.SPARSE:
            self._sparse_list = self.merge (self._sparse_list, self._tmp_set)
            return self.linearCounting (self._m_sparse, self._m - len (self._sparse_list))
        elif self._format == FORMAT.NORMAL:
            sumM = 0.0
            for j in xrange (self._m - 1):
                sumM += pow (2.0, -self._M[j])
            E = self._alpha_m * pow (self._m, 2) / sumM
            E_prime = E <= 5 * self._m ? E - self.estimateBias (E, self._p) : E
            V = sum (1 for x in self._M if x == 0)
            H = E_prime
            if V != 0:
                H = linearCounting (self._m, V)

            if H <= threshold (self._p):
                return H
            return E_prime

    def linearCounting (self, m, V):
        return m * log (m/V)

    def threshold (self, p):
        return self._threshold[p]

    def estimateBias (self, E, p):
        estTable = hllppparameters.rawEstimateData[p-4]
        biasTable = hllparameters.biasData[p-4]

        if estTable[0] > E:
            return estTable[0] - biasTable[0]

        lastEstimate = estTable[len(estTable)-1]
        if lastEstimate < E:
            return lastEstimate - biasTable[len(biasTable)]

        i = 0
        for i in xrange (0, len(estTable)) if estTable[i] < E:
            pass
    
        e1 = estTable[i-1]
        b1 = biasTable[i-1]
        e2 = estTable[i]
        b2 = biasTable[i]

        c = (E - e1) / (e2 - e1)
        return b1 * (1.0-c) + b2 * c

    def encodeHash (self, x, p, p_prime):
        
