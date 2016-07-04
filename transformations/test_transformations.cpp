

#include <iostream>
#include <iomanip>
#include <math.h>
#include <vector>
#include <algorithm>


//#define KEEP_SEPARATED 1
#define EPSILON 1e-10

typedef std::vector<double> MatrixLine;
typedef std::vector<MatrixLine> Matrix;

std::ostream& operator<<(std::ostream& out, const Matrix& matrix)
{
    int count = 0;
    for (Matrix::const_iterator itLine = matrix.begin(), itLineEnd = matrix.end(); itLine != itLineEnd; ++itLine)
    {
	out << count << ": ";
	for (MatrixLine::const_iterator it = itLine->begin(), itEnd = itLine->end(); it != itEnd; ++it)
	{  
	    out << std::right << std::setw(14) << (*it);
	}
	out << std::endl;
	++count;
    }
    out << std::endl;
    return out;
}


namespace IKS
{

    
    class POSITION
    {
    protected:
	double m_value[2];
    public:
	POSITION () 
	{
	    m_value[0] = 0;
	    m_value[1] = 0;
	}
	POSITION (double x, double y) 
	{
	    m_value[0] = x;
	    m_value[1] = y;
	}

	double x() const { return m_value[0]; }
	double y() const { return m_value[1]; }
	double& x()  { return m_value[0]; }
	double& y()  { return m_value[1]; }

	POSITION& operator+= (const POSITION& pos) 
	{
	    m_value[0] += pos.m_value[0];
	    m_value[1] += pos.m_value[1];
	    return *this;
	}

	POSITION& operator*= (double factor) 
	{
	    m_value[0] *= factor;
	    m_value[1] *= factor;
	    return *this;
	}

	double operator* (const POSITION& RHS) 
	{
	    double ret(0);
	    ret += m_value[0] * RHS.m_value[0];
	    ret += m_value[1] * RHS.m_value[1];
	    return ret;
	}


	POSITION operator- (const POSITION& RHS) 
	{
	    IKS::POSITION ret(*this);
	    ret.m_value[0] -= RHS.m_value[0];
	    ret.m_value[1] -= RHS.m_value[1];
	    return ret;
	}


	double norm ()
	{
	    return (*this)*(*this);
	}
    };



    class POINT
    {
	long m_value[2];
    public:
	POINT (long x, long y) 
	{
	    m_value[0] = x;
	    m_value[1] = y;
	}

	long x() const { return m_value[0]; }
	long y() const { return m_value[1]; }

	double distance (const POINT& other)
	{
	    IKS::POSITION posA ((*this).x(), (*this).y());
	    IKS::POSITION posB (other.x(), other.y());
	    return (posA-posB).norm();
	}

    };

    class RESOLUTION;
    
    class PIXELSIZE : public POSITION
    {
    public:
	PIXELSIZE (double x, double y) : POSITION (x, y) {}
	PIXELSIZE (const RESOLUTION& resolution);
    };


    class RESOLUTION : public POSITION
    {
    public:
	RESOLUTION (double x, double y) : POSITION (x, y) {}
	RESOLUTION (const PIXELSIZE& pixelSize) : POSITION (pixelSize.x()/25.4, pixelSize.y()/25.4) {}

    private:
	double m_resolutionX;
	double m_resolutionY;
    };


    PIXELSIZE::PIXELSIZE (const RESOLUTION& resolution)
    {
	m_value[0] = 25.4/resolution.x();
	m_value[1] = 25.4/resolution.y();
    }


    class HomogeneousCoordinates;
    std::ostream& operator<<(std::ostream& out, const HomogeneousCoordinates& pos);

    class HomogeneousCoordinates // homogeneous coordinates
    {
	double m_value[3];
    public:
	HomogeneousCoordinates(const POSITION& p) 
	{
	    m_value[0] = p.x();
	    m_value[1] = p.y();
	    m_value[2] = 1;
	}
	HomogeneousCoordinates(const POINT& p) 
	{
	    m_value[0] = (double)p.x();
	    m_value[1] = (double)p.y();
	    m_value[2] = (double)1;
	}
	HomogeneousCoordinates(double x, double y) 
	{
	    m_value[0] = x;
	    m_value[1] = y;
	    m_value[2] = 1;
	}
	HomogeneousCoordinates(double x, double y, double scale) 
	{
	    m_value[0] = x;
	    m_value[1] = y;
	    m_value[2] = scale;
	}
	HomogeneousCoordinates() 
	{
	    m_value[0] = 0;
	    m_value[1] = 0;
	    m_value[2] = 1; //scale
	}
	HomogeneousCoordinates(const HomogeneousCoordinates& RHS) 
	{
	    m_value[0] = RHS[0];
	    m_value[1] = RHS[1];
	    m_value[2] = RHS[2];
	}

	operator POSITION() { return POSITION(x(), y()); }
	operator POINT   () { return POINT   ((long)(x()+0.5), (long)(y()+0.5)); }

	HomogeneousCoordinates& operator=(const HomogeneousCoordinates& RHS) 
	{
	    m_value[0] = RHS[0];
	    m_value[1] = RHS[1];
 	    m_value[2] = RHS[2];
	    return *this;
	}
	double& operator[](short index) { return m_value[index]; }
	double  operator[](short index) const { return m_value[index]; }

	double x() const { return m_value[0]/m_value[2]; }
	double y() const { return m_value[1]/m_value[2]; }

	bool isDirection () const { return fabs(m_value[2])<EPSILON; }
	bool isPosition  () const { return fabs(m_value[2])>EPSILON; }

	HomogeneousCoordinates operator+(const HomogeneousCoordinates& RHS) const 
	{
	    HomogeneousCoordinates ret;
	    for (int i=0; i<3; ++i)
	    {
		ret[i] = (*this)[i]+RHS[i];
	    }
	    return ret;
	}
	HomogeneousCoordinates operator-(const HomogeneousCoordinates& RHS) const 
	{
	    HomogeneousCoordinates ret;
	    for (int i=0; i<3; ++i)
	    {
		ret[i] = (*this)[i]-RHS[i];
	    }
	    return ret;
	}

	double operator*(const HomogeneousCoordinates& RHS) const 
	{
	    double ret(0);
	    for (int i=0; i<3; ++i)
	    {
		ret += (*this)[i]*RHS[i];
	    }
	    return ret;
	}
	double cross2D(const HomogeneousCoordinates& RHS) const
	{
	    return (*this)[0]*RHS[1] - (*this)[1]*RHS[0];
	}
	HomogeneousCoordinates cross(const HomogeneousCoordinates& RHS) const
	{
	    return HomogeneousCoordinates( (*this)[1]*RHS[2] - (*this)[2]*RHS[1], 
					   (*this)[2]*RHS[0] - (*this)[0]*RHS[2], 
					   (*this)[0]*RHS[1] - (*this)[1]*RHS[0] );
	}
	double norm() const 
	{
	    return (*this)*(*this);
	}
    };



    class RECT
    {
	POSITION m_leftTop;
	POSITION m_rightBottom;
    public:
	RECT (POSITION LeftTop, POSITION RightBottom) : m_leftTop(LeftTop), m_rightBottom(RightBottom) {}

	POSITION LeftTop() const { return m_leftTop; }
	POSITION RightBottom() const { return m_rightBottom; }
	POSITION LeftBottom() const { return POSITION(m_leftTop.x(), m_rightBottom.y()); }
	POSITION RightTop() const { return POSITION(m_rightBottom.x(), m_leftTop.y()); }

    };



    std::ostream& operator<<(std::ostream& out, const IKS::POSITION& pos)
    {
	out << pos.x() << "   " << pos.y();
	return out;
    }


    std::ostream& operator<<(std::ostream& out, const IKS::POINT& pos)
    {
	out << pos.x() << "   " << pos.y();
	return out;
    }


    std::ostream& operator<<(std::ostream& out, const HomogeneousCoordinates& pos)
    {
	for (int i=0; i<3; ++i)
	{
	    out << pos[i] << "   ";
	}
//	out << std::endl;
	return out;
    }

    std::ostream& operator<<(std::ostream& out, const RECT& rect)
    {
	out << "leftTop    = "<< rect.LeftTop() << "   ";
	out << "rightTop   = "<< rect.RightTop() << "   ";
	out << "rightBottom= "<< rect.RightBottom() << "   ";
	out << "leftBottom = "<< rect.LeftBottom() << "   ";
	out << std::endl;
	return out;
    }


    typedef std::vector<IKS::HomogeneousCoordinates> VctHomogeneousCoordinates;
}



enum EnumTransformationType
{
    enumContainer = 0,
    enumMatrix = 1
};


class ITransformation
{
public:
    virtual IKS::POSITION  transform(const IKS::POSITION& Position) const = 0;
    virtual IKS::POINT     transform(const IKS::POINT&   Point) { return Point; };
};



// forward declarations
class cTransformationHandler;
class cTransformationMatrix;
class cMatrixTransformation;
class cTransformation;

std::ostream& operator<<(std::ostream& out, const cTransformationHandler& trf);
std::ostream& operator<<(std::ostream& out, const cTransformationMatrix&  trf);
std::ostream& operator<<(std::ostream& out, const cTransformation&        trf);




class cTransformationHandler 
{
    cMatrixTransformation* m_matrixTransformation;
    cTransformation*       m_transformationContainer;
    EnumTransformationType m_transformationType;

public:

    cTransformationHandler(const cTransformationHandler& trf);
    cTransformationHandler(const cMatrixTransformation* trf);
    cTransformationHandler(const cTransformation*       trf);
    ~cTransformationHandler();


    IKS::HomogeneousCoordinates  transform(const IKS::HomogeneousCoordinates& Position) const;

    const cTransformationHandler& operator= (const cTransformationHandler& trf);
    const cTransformationHandler& operator+=(const cTransformationHandler& trf);
    cTransformationHandler        operator- () const;
    bool AlmostEqual (const cTransformationHandler& trf, double dTolerance) const;

    bool IsIdentity() const;

    EnumTransformationType TransformationType() const { return m_transformationType; } 

    // exceptions
    class IncompatibleTransformationType {};
    class PointerToTransformationIsNull {};
    class UnknownTransformationType {};

    // friends
    friend class cTransformation;
    friend std::ostream& operator<<(std::ostream& out, const cTransformationHandler& trf);
};





typedef std::vector<cTransformationHandler> TrfVct;

// master transformation class (has a container holding the individual transformations; combines subsequent transformations which are of the same type)
class cTransformation : public ITransformation
{
    TrfVct m_constituents;

    IKS::HomogeneousCoordinates  transform(const IKS::HomogeneousCoordinates& Position, size_t index, size_t size) const 
    { 
//	std::cout << "index: " <<index << "  pos: " << Position << std::endl;
	if (index>=size)
	    return Position;
	return transform(m_constituents.at(index).transform(Position), index+1, size);
    }


    IKS::HomogeneousCoordinates  transform(const IKS::HomogeneousCoordinates& Position) const 
    { 
	if (m_constituents.empty()) // if empty: transformation is identity
	    return Position;
	return transform(Position, 0, m_constituents.size());
    }

public:
    cTransformation() {}
    cTransformation(const cTransformation& trf) 
    { 
	m_constituents.clear();
	for (TrfVct::const_iterator it = trf.m_constituents.begin(), itEnd = trf.m_constituents.end(); it != itEnd; ++it)
	{
	    m_constituents.push_back ( (*it) );
	}
    }
    cTransformation(const cTransformationHandler& trf) 
    { 
	m_constituents.clear();
	switch (trf.TransformationType())
	{
	case enumMatrix:
	    m_constituents.push_back(trf); 
	    break;
	case enumContainer:
	    for (TrfVct::const_iterator it = trf.m_transformationContainer->m_constituents.begin(), itEnd = trf.m_transformationContainer->m_constituents.end(); it != itEnd; ++it)
	    {
		m_constituents.push_back ( (*it) );
	    }
	    break;
	}
    }

    EnumTransformationType TransformationType() const { return enumContainer; } 


    virtual IKS::POSITION          transform(const IKS::POSITION&         Position) const 
    {
	return (IKS::POSITION)transform(IKS::HomogeneousCoordinates(Position));
    }
    virtual IKS::POINT          transform(const IKS::POINT&         Position) const 
    {
	return (IKS::POINT)transform(IKS::HomogeneousCoordinates(Position));
    }


    operator cTransformationHandler () const
    { 
	return cTransformationHandler(this); 
    }

    // compare operators
    bool operator ==(const cTransformation& RHS) const;
    bool operator !=(const cTransformation& RHS) const;
    bool AlmostEqual(const cTransformation& RHS, double dTolerance) const;
    bool IsIdentity() const;

    // arithmetic operators
    const cTransformation&  operator= (const cTransformationHandler& RHS);

    const cTransformation&  operator-=(const cTransformationHandler& RHS);
    cTransformation         operator- (const cTransformationHandler& RHS) const;
    cTransformation         operator- () const;
    const cTransformation&  operator+=(const cTransformationHandler& RHS);
    cTransformation         operator+ (const cTransformationHandler& RHS) const;

    // friends
    friend std::ostream& operator<<(std::ostream& out, const cTransformation& trf);
    friend class cTransformationHandler;
};

std::ostream& operator<<(std::ostream& out, const cTransformation& trf)
{
    out << "=== Transformation ===" << std::endl;
    if (trf.m_constituents.empty())
    {
	out << "1 (Identity)" << std::endl;
	return out;
    }
    int count(0);
    for (TrfVct::const_iterator it = trf.m_constituents.begin(), itEnd = trf.m_constituents.end(); it != itEnd; ++it)
    {	
	if (count>0)
	    out << " + ";
	switch (it->TransformationType())
	{
	case enumMatrix:
	    out << "M" << count;
	    break;
	case enumContainer:
	    out << "C" << count;
	    break;
	}
	++count;
    }
    out << std::endl;
    out << "===" << std::endl;
    count = 0;
    for (TrfVct::const_iterator it = trf.m_constituents.begin(), itEnd = trf.m_constituents.end(); it != itEnd; ++it)
    {	
	switch (it->TransformationType())
	{
	case enumMatrix:
	    out << "M" << count << "=";
	    out << std::endl;
	    out << (*it);
	    out << std::endl;
	    break;
	case enumContainer:
	    break;
	}
	++count;
    }
    return out;
}


bool cTransformation::AlmostEqual(const cTransformation& RHS, double dTolerance) const
{
    for (TrfVct::const_iterator it = m_constituents.begin(), itOther = RHS.m_constituents.begin(), itEnd = m_constituents.end(), itEndOther = RHS.m_constituents.end();
	 it != itEnd && itOther != itEndOther; ++it, ++itOther)
    {
	if ( !(*it).AlmostEqual((*itOther),dTolerance) )
	    return false;
    }
    return true;
}

bool cTransformation::operator==(const cTransformation& RHS) const
{
    return AlmostEqual(RHS,EPSILON);
}

bool cTransformation::operator!=(const cTransformation& RHS) const
{
    return !((*this)==RHS);
}

bool cTransformation::IsIdentity() const
{
    for (TrfVct::const_iterator it = m_constituents.begin(), itEnd = m_constituents.end(); it != itEnd; ++it)
    {
	if ( !it->IsIdentity() )
	    return false;
    }
    return true;
}



const cTransformation& cTransformation::operator= (const cTransformationHandler& RHS)
{
    m_constituents.clear();
    if (RHS.TransformationType()==enumContainer)
    {
	m_constituents.assign (RHS.m_transformationContainer->m_constituents.begin(), RHS.m_transformationContainer->m_constituents.end());
	return (*this);
    }
    m_constituents.push_back (RHS);
    return (*this);
}


cTransformation cTransformation::operator+(const cTransformationHandler& RHS) const
{
    cTransformation trf(*this);
    trf += (RHS);
    return trf;
}

cTransformation cTransformation::operator-(const cTransformationHandler& RHS) const
{
    cTransformation trf(*this);
    trf += (-RHS);
    return trf;
}

const cTransformation& cTransformation::operator+=(const cTransformationHandler& RHS) 
{
    // if another transformation container is added
    if (RHS.TransformationType() == enumContainer)
    {
	// add all transformations to the list of constituents
	const cTransformation* const pTrf = RHS.m_transformationContainer;
	for (TrfVct::const_iterator it = pTrf->m_constituents.begin(), itEnd = pTrf->m_constituents.end(); it != itEnd; ++it)
	{
#ifdef 	KEEP_SEPARATED
	    if (false)   // ========================================== testing
#else
		if (!m_constituents.empty() && ((*this).m_constituents.back().TransformationType() == it->TransformationType()))
#endif
	    {
		(*this).m_constituents.back() += (*it);
		if ((*this).m_constituents.back().IsIdentity())
		    m_constituents.erase (m_constituents.end()-1);
	    }
	    else
	    {
		(*this).m_constituents.push_back (*it);
	    }
	}
	return (*this);
    }

    if (m_constituents.empty())
	return (*this);

    // added transformation has a different transformation type as the last one in the list of constituents
#ifdef 	KEEP_SEPARATED
    if (true)  // ========================================== testing
#else
	if (m_constituents.back().TransformationType() != RHS.TransformationType())
#endif
	{
	    m_constituents.push_back(RHS);
	    return (*this);
	}
    
    // added transformation has the same transformation type as the last one in the list of constituents
    m_constituents.back() += RHS;
    if ((*this).m_constituents.back().IsIdentity())
	m_constituents.erase (m_constituents.end()-1);
    return (*this);
}


const cTransformation& cTransformation::operator-=(const cTransformationHandler& RHS) 
{
    (*this) += -RHS;
    return (*this);
}


cTransformation cTransformation::operator-() const
{
    cTransformation trf;
    if (m_constituents.empty())
	return trf;

    TrfVct::const_iterator it = m_constituents.end();
    TrfVct::const_iterator itBegin = m_constituents.begin();
    // go through the transformations in inverse direction
    while (it != itBegin)
    {
	--it;
	trf.m_constituents.push_back (-(*it)); // add the inverse transformation
    }
    return trf;
}






class cTransformationMatrix
{
protected:
    double m_matrixMM [3][3]; // [y][x]

public:

    cTransformationMatrix() 
    {
	setUnit (m_matrixMM);
	// unit matrix does not have to be metrically scaled
    }
    cTransformationMatrix(const cTransformationMatrix& other) 
    {
	std::copy (*other.m_matrixMM,  *other.m_matrixMM+9,  *m_matrixMM );
    } 
    cTransformationMatrix(double mMM[3][3]) 
    {
	std::copy (*mMM, *mMM+9, *m_matrixMM );
    }
    ~cTransformationMatrix() {}


    EnumTransformationType TransformationType() const { return enumMatrix; } 

    IKS::HomogeneousCoordinates   transform(const IKS::HomogeneousCoordinates& Position) const; 


    virtual IKS::POSITION   transform(const IKS::POSITION& Position) const;

    // compare operators
    bool operator ==(const cTransformationMatrix& RHS) const;
    bool operator !=(const cTransformationMatrix& RHS) const;
    bool AlmostEqual(const cTransformationMatrix& RHS, double dTolerance) const;
//    bool IsNull() const;
    bool IsIdentity() const;
    // arithmetic operator
    const cTransformationMatrix&  operator=(const cTransformationMatrix& RHS);
    cMatrixTransformation         operator-(const cTransformationMatrix& RHS) const;
    cMatrixTransformation         operator-() const;
    cMatrixTransformation         operator+(const cTransformationMatrix& RHS) const;
    cMatrixTransformation         operator*(const cTransformationMatrix& RHS) const;

    cMatrixTransformation         subElem(const cTransformationMatrix& RHS) const;
    cMatrixTransformation         addElem(const cTransformationMatrix& RHS) const;

    operator cMatrixTransformation () const; 
    operator cTransformationHandler () const; 


public:

    double& operator ()(short line, short column, double value) 
    { 
	m_matrixMM[line][column] = value; 
	return m_matrixMM[line][column];
    }

private:
    void setUnit (double matrix[3][3]) 
    {
	// set matrix to identity
	for (int i=0; i<3; ++i)
	    for (int j=0; j<3; ++j)
	    {
		if (i==j)
		{
		    matrix[i][j] = 1;
		    continue;
		}
		matrix[i][j] = 0;
	    }
    }

    void multiply(double LHS[3][3], double RHS[3][3], double result[3][3]) const;


    // helper
    friend std::ostream& operator<<(std::ostream& out, const cTransformationMatrix& matrix);
};





IKS::HomogeneousCoordinates   cTransformationMatrix::transform(const IKS::HomogeneousCoordinates& Position)  const
{ 
    IKS::HomogeneousCoordinates pos(0,0,0);
    for (int i=0; i<3; ++i)
	for (int j=0; j<3; ++j)
	    pos[i] += m_matrixMM[i][j]*Position[j];
    if (pos[2]==1)
	return pos;
    if (fabs(pos[2])<EPSILON) // error
	return pos;
    for (int i=0; i<3; ++i)
	pos[i] /= pos[2];
    return pos;
}



IKS::POSITION   cTransformationMatrix::transform(const IKS::POSITION& Position) const 
{ 
    return transform(IKS::HomogeneousCoordinates(Position)); 
}







// reducedRowEchelonForm. Gauss-Jordan elemination. for better numerical stability 
// one could implement pivoting or partial pivoting. 
void reducedRowEchelonForm (Matrix& matrix, double dTolerance = EPSILON)
{
//    std::cout << matrix;
    if (matrix.empty())
	return;
    size_t lead = 0;
    size_t rowCount = matrix.size();
    size_t columnCount = matrix[0].size();
    for (size_t r = 0; r < rowCount; ++r)
    {
	if (lead >= columnCount)
	    return;
	size_t i = r;
	while (fabs(matrix[i][lead])<dTolerance)
	{
	    i += 1;
	    if (i == rowCount)
	    {
		i = r;
		lead += 1;
		if (columnCount == lead)
		    return;
	    }
	}
	matrix[i].swap(matrix[r]);
	double lv = matrix[r][lead];
	for (MatrixLine::iterator it = matrix[r].begin(), itEnd = matrix[r].end(); it != itEnd; ++it)
	    (*it) /= lv;
	for (size_t i = 0; i < rowCount; ++i)
	{
	    if (i != r)
	    {
		lv = matrix[i][lead];
		for (MatrixLine::iterator itI = matrix[i].begin(), itR = matrix[r].begin(), itIEnd = matrix[i].end(), itREnd = matrix[r].end(); 
		     itI != itIEnd; ++itI, ++itR)
		{
		    double rv = (*itR);
		    double iv = (*itI);
		    (*itI) = iv - lv*rv;
		}
	    }
	}
	lead += 1;
    }
//    std::cout << matrix;
}



std::ostream& operator<<(std::ostream& out, const cTransformationMatrix& trf)
{
    out << "(matrix 3x3)" << std::endl;
    for (int i=0; i<3; ++i)
    {
	for (int j=0; j<3; ++j)
	    out << std::right << std::setw(14) << trf.m_matrixMM[i][j];
	out << std::endl;
    }

    return out;
}





bool cTransformationMatrix::operator!=(const cTransformationMatrix& RHS) const
{
    return !(*this==RHS);
}

bool cTransformationMatrix::AlmostEqual(const cTransformationMatrix& RHS, double dTolerance) const
{
    for (size_t i = 0; i<3; ++i)
	for (size_t j = 0; j<3; ++j)
	{
	    if (!(fabs(m_matrixMM[i][j]-RHS.m_matrixMM[i][j])<dTolerance)) // check !( A < B) to get "false" for "NaN" as well 
		return false;
	}
    return true;
}

bool cTransformationMatrix::operator==(const cTransformationMatrix& RHS) const
{
    return AlmostEqual(RHS, EPSILON);
}

bool cTransformationMatrix::IsIdentity() const
{
    return (*this)==cTransformationMatrix();
}

const cTransformationMatrix& cTransformationMatrix::operator=(const cTransformationMatrix& RHS) 
{
    std::copy (*RHS.m_matrixMM, *RHS.m_matrixMM+9, *m_matrixMM); 
    return *this;
}





class cScaling : public cTransformationMatrix
{
public:

    cScaling(double scaleX, double scaleY) : cTransformationMatrix()
    {
	this->operator()(0,0,scaleX);
	this->operator()(1,1,scaleY);
    }
};

class cRotation : public cTransformationMatrix
{
public:

    cRotation(double angle) : cTransformationMatrix()
    {
	this->operator()(0,0,cos(angle));
	this->operator()(0,1,-sin(angle));
	this->operator()(1,0,sin(angle));
	this->operator()(1,1,cos(angle));
    }
};

class cTranslation : public cTransformationMatrix
{
public:
    cTranslation(IKS::POSITION trans) : cTransformationMatrix()
    {
	this->operator()(0,2,trans.x());
	this->operator()(1,2,trans.y());
    }
};




class cProjectiveTransformation : public cTransformationMatrix
{
public:
    class InvalidInput {};

    template <typename Iterator >
    cProjectiveTransformation (Iterator srcBegin, Iterator srcEnd, Iterator dstBegin) : cTransformationMatrix() 
    {
	Initialize (srcBegin, srcEnd, dstBegin, m_matrixMM);
    }


    template <typename Iterator>
    void Initialize (Iterator srcBegin, Iterator srcEnd, Iterator dstBegin, double matrix[3][3]) 
    {
	size_t numSrcPoints = std::distance(srcBegin,srcEnd);
	if (numSrcPoints != 4)
	{
	    // throw IKS::Error(E_INVALID_INPUT, "cProjTransformation: 4 source and 4 target points needed but only %d given", numSrcPoints);
	    throw InvalidInput();
	}

	int N = 8 ; //number of parameters to determine (as the 9th parameter is equal to 1 for projective maps)  

	// create homogeneous coordinates from the POSITIONs or POINTs
	IKS::VctHomogeneousCoordinates srcPoints; 
	IKS::VctHomogeneousCoordinates dstPoints; 
	for (Iterator itSrc = srcBegin, itSrcEnd = srcEnd, itDst = dstBegin; itSrc != itSrcEnd; ++itSrc, ++itDst)
	{
	    srcPoints.push_back ( (*itSrc) );
	    dstPoints.push_back ( (*itDst) );
	}

	// test triples of src points if they are on the same line. If yes, reduce the number of Src points by one. 
        // If less than four survive, no projective transformation can be computed
	size_t nonAlignedSourcePoints = numSrcPoints;
	for (IKS::VctHomogeneousCoordinates::const_iterator itA = srcPoints.begin(), itEnd = srcPoints.end(); itA != itEnd; ++itA)
	    for (IKS::VctHomogeneousCoordinates::const_iterator itB = itA+1; itB != itEnd; ++itB)
		for (IKS::VctHomogeneousCoordinates::const_iterator itC = itB+1; itC != itEnd; ++itC)
		{
		    IKS::HomogeneousCoordinates ba(*itA - *itB);
		    IKS::HomogeneousCoordinates bc(*itC - *itB);
		    double prod( bc.cross2D(ba) );


		    // std::cout << "--" << std::endl;
		    // std::cout << "*itA : " << (*itA) << std::endl;
		    // std::cout << "*itB : " << (*itB) << std::endl;
		    // std::cout << "*itC : " << (*itC) << std::endl;
		    // std::cout << "ba : " << ba << std::endl;
		    // std::cout << "bc : " << bc << std::endl;
		    // std::cout << "prod : " << prod << std::endl;

		    if (fabs(prod)<EPSILON)
			--nonAlignedSourcePoints;
		}

	if (nonAlignedSourcePoints < 4)
	    throw InvalidInput();


	// test triples of destination points if they are on the same line. If yes, reduce the number of destination points by one. 
        // If less than four survive, no projective transformation can be computed
	size_t nonAlignedDestPoints = numSrcPoints; // got the same number of destination points as source points
	for (IKS::VctHomogeneousCoordinates::const_iterator itA = dstPoints.begin(), itEnd = dstPoints.end(); itA != itEnd; ++itA)
	    for (IKS::VctHomogeneousCoordinates::const_iterator itB = itA+1; itB != itEnd; ++itB)
		for (IKS::VctHomogeneousCoordinates::const_iterator itC = itB+1; itC != itEnd; ++itC)
		{
		    IKS::HomogeneousCoordinates ba(*itA - *itB);
		    IKS::HomogeneousCoordinates bc(*itC - *itB);
		    double prod( bc.cross2D(ba) );
		    if (fabs(prod)<EPSILON)
			--nonAlignedDestPoints;
		}

	if (nonAlignedDestPoints < 4)
	    throw InvalidInput();

	

	//Initialization of the 8x8 projective matrix plus the nonhomogeneous part pM[i][8]  
	size_t nPoints = 4;
	Matrix eqSystem(2*nPoints);
	for (size_t i = 0; i < nPoints; ++i)
	{
	    eqSystem.at(2*i).push_back(srcPoints.at(i)[0]); 
	    eqSystem.at(2*i).push_back(srcPoints.at(i)[1]); 
	    eqSystem.at(2*i).push_back(1); 
	    eqSystem.at(2*i).push_back(0); 
	    eqSystem.at(2*i).push_back(0); 
	    eqSystem.at(2*i).push_back(0); 
	    eqSystem.at(2*i).push_back(-srcPoints.at(i)[0]*dstPoints.at(i)[0]); 
	    eqSystem.at(2*i).push_back(-dstPoints.at(i)[0]*srcPoints.at(i)[1]); 
//	    eqSystem.at(2*i).push_back(-dstPoints.at(i)[0]); 
	    eqSystem.at(2*i).push_back(dstPoints.at(i)[0]); 

	    eqSystem.at(2*i+1).push_back(0); 
	    eqSystem.at(2*i+1).push_back(0); 
	    eqSystem.at(2*i+1).push_back(0); 
	    eqSystem.at(2*i+1).push_back(srcPoints.at(i)[0]); 
	    eqSystem.at(2*i+1).push_back(srcPoints.at(i)[1]); 
	    eqSystem.at(2*i+1).push_back(1); 
	    eqSystem.at(2*i+1).push_back(-srcPoints.at(i)[0]*dstPoints.at(i)[1]); 
	    eqSystem.at(2*i+1).push_back(-dstPoints.at(i)[1]*srcPoints.at(i)[1]); 
//	    eqSystem.at(2*i+1).push_back(-dstPoints.at(i)[1]); 
	    eqSystem.at(2*i+1).push_back(dstPoints.at(i)[1]); 
	}


	reducedRowEchelonForm(eqSystem);
//	std::cout << "eqsystem: " << eqSystem << std::endl;

	int count = 0;
	for (int k=0; k<3; ++k)
	{
	    for (int j=0; j<3; ++j)
	    {
		if (count == N)
		    matrix[k][j] = 1;
		else
		    matrix[k][j] = eqSystem.at(count).at(N);
		++count;
	    }
	}
    }
};



class cMatrixTransformation : public cTransformationMatrix
{
public:
    cMatrixTransformation () : cTransformationMatrix() {}
    cMatrixTransformation (const cTransformationMatrix& trf) : cTransformationMatrix(trf) {}
    cMatrixTransformation (const cMatrixTransformation& trf) : cTransformationMatrix(trf) {}
    ~cMatrixTransformation () {}

    const cMatrixTransformation& operator=(const cMatrixTransformation& RHS) 
    { 
	cTransformationMatrix::operator=(RHS);
	return (*this);
    }
    operator cTransformationHandler() const
    { 
	return cTransformationHandler(this); 
    }

    bool operator==(const cMatrixTransformation& RHS) { return cTransformationMatrix::operator==(RHS); }
    bool operator!=(const cMatrixTransformation& RHS) { return cTransformationMatrix::operator!=(RHS); }

    cMatrixTransformation        operator+ (const cMatrixTransformation& RHS) const { return cTransformationMatrix::operator+( RHS); }
    cMatrixTransformation        operator- (const cMatrixTransformation& RHS) const { return cTransformationMatrix::operator+(-RHS); }
    cMatrixTransformation        operator- () const { return cTransformationMatrix::operator-(); }
    const cMatrixTransformation& operator+=(const cMatrixTransformation& RHS) { return *this = (*this) + RHS; }
    const cMatrixTransformation& operator-=(const cMatrixTransformation& RHS) { return *this = (*this) - RHS; }
    cMatrixTransformation        operator* (const cMatrixTransformation& RHS) const   { return cTransformationMatrix::operator*(RHS); }
    cMatrixTransformation        operator*=(const cMatrixTransformation& RHS) { return *this = cTransformationMatrix::operator*(RHS); }
};




cMatrixTransformation cTransformationMatrix::operator-() const 
{
    Matrix matrixMM;
    for (int i=0; i<3; ++i)
    {
	matrixMM.push_back  (MatrixLine());
	for (int j=0; j<3; ++j)
	    matrixMM. back().push_back(m_matrixMM [i][j]);
	for (int j=0; j<3; ++j)
	    matrixMM. back().push_back(i==j ? 1 : 0);
    }
    reducedRowEchelonForm(matrixMM );

    cTransformationMatrix trf;
    for (int i=0; i<3; ++i)
	for (int j=0; j<3; ++j)
	    trf.m_matrixMM [i][j] = matrixMM [i][j+3];

    return trf;
}




void cTransformationMatrix::multiply(double LHS[3][3], double RHS[3][3], double result[3][3]) const
{
    for (size_t i = 0; i<3; ++i)
	for (size_t k = 0; k<3; ++k)
	{
	    result [i][k] = 0;
	    for (size_t j = 0; j<3; ++j)
	    {
		result [i][k] += LHS [i][j] * RHS [j][k];
	    }
	}
}



cMatrixTransformation cTransformationMatrix::operator*(const cTransformationMatrix& RHS) const
{
    cMatrixTransformation trf;
    for (size_t i = 0; i<3; ++i)
	for (size_t k = 0; k<3; ++k)
	{
	    trf.m_matrixMM [i][k] = 0;
	    for (size_t j = 0; j<3; ++j)
		trf.m_matrixMM [i][k] += m_matrixMM [i][j] * RHS.m_matrixMM [j][k];
	}
    return trf;
}




cMatrixTransformation cTransformationMatrix::operator+(const cTransformationMatrix& RHS) const
{
    return (RHS*(*this));
}


cMatrixTransformation cTransformationMatrix::subElem(const cTransformationMatrix& RHS) const
{
    cMatrixTransformation trf;
    for (size_t i = 0; i<3; ++i)
	for (size_t j = 0; j<3; ++j)
	    trf.m_matrixMM [i][j] = m_matrixMM [i][j] - RHS.m_matrixMM [i][j];
    return trf;
}

cMatrixTransformation cTransformationMatrix::addElem(const cTransformationMatrix& RHS) const
{
    cMatrixTransformation trf;
    for (size_t i = 0; i<3; ++i)
	for (size_t j = 0; j<3; ++j)
	    trf.m_matrixMM [i][j] = m_matrixMM [i][j] + RHS.m_matrixMM [i][j];
    return trf;
}

cMatrixTransformation cTransformationMatrix::operator-(const cTransformationMatrix& RHS) const
{
    return cMatrixTransformation(*this+(-RHS));
}


cTransformationHandler::~cTransformationHandler()
{
    if (m_matrixTransformation)    delete m_matrixTransformation;
    if (m_transformationContainer) delete m_transformationContainer;
}


cTransformationHandler::cTransformationHandler(const cTransformationHandler& trf) 
    : m_matrixTransformation(trf.m_matrixTransformation ? new cMatrixTransformation(*trf.m_matrixTransformation) : NULL), 
      m_transformationContainer(trf.m_transformationContainer ? new cTransformation(*trf.m_transformationContainer) : NULL), 
      m_transformationType(trf.m_transformationType) 
{}

const cTransformationHandler& cTransformationHandler::operator=(const cTransformationHandler& trf)
{
    m_matrixTransformation    = (trf.m_matrixTransformation ? new cMatrixTransformation(*trf.m_matrixTransformation) : NULL);
    m_transformationContainer = (trf.m_transformationContainer ? new cTransformation(*trf.m_transformationContainer) : NULL);
    m_transformationType = trf.m_transformationType;
    return (*this);
}

cTransformationHandler::cTransformationHandler(const cMatrixTransformation* trf) 
    : m_matrixTransformation(new cMatrixTransformation(*trf)), 
      m_transformationContainer(NULL), 
      m_transformationType(enumMatrix) 
{
}

cTransformationHandler::cTransformationHandler(const cTransformation*       trf) 
    : m_matrixTransformation(NULL), 
      m_transformationContainer(new cTransformation(*trf)), 
      m_transformationType(enumContainer) 
{
}


cTransformationMatrix::operator cMatrixTransformation () const 
{ 
    return cMatrixTransformation(*this); 
}

cTransformationMatrix::operator cTransformationHandler () const 
{ 
    cMatrixTransformation cm(*this);
    return cTransformationHandler(&cm); 
}

const cTransformationHandler& cTransformationHandler::operator+=(const cTransformationHandler& trf)
{
    if (trf.TransformationType() != this->TransformationType())
	throw IncompatibleTransformationType();
    switch (TransformationType())
    {
    case enumMatrix:
	if (!m_matrixTransformation)
	    throw PointerToTransformationIsNull();
	*m_matrixTransformation += (*(trf.m_matrixTransformation));
	break;
    case enumContainer:
	if (!m_transformationContainer)
	    throw PointerToTransformationIsNull();
	(*m_transformationContainer)+=(*(trf.m_transformationContainer));
	break;
    };
    return (*this);
}

bool cTransformationHandler::IsIdentity() const
{
    switch(TransformationType())
    {
    case enumMatrix:
	if (!m_matrixTransformation)
	    throw PointerToTransformationIsNull();
	return m_matrixTransformation->IsIdentity();
    case enumContainer:
	if (!m_transformationContainer)
	    throw PointerToTransformationIsNull();
	return m_transformationContainer->IsIdentity();
    };
    return false;
}

cTransformationHandler cTransformationHandler::operator- () const
{
    switch(TransformationType())
    {
    case enumMatrix:
	if (!m_matrixTransformation)
	    throw PointerToTransformationIsNull();
	return -(*m_matrixTransformation);
    case enumContainer:
	if (!m_transformationContainer)
	    throw PointerToTransformationIsNull();
	return -(*m_transformationContainer);
    };
    throw UnknownTransformationType();
    return cTransformationHandler(cMatrixTransformation());
}


IKS::HomogeneousCoordinates  cTransformationHandler::transform(const IKS::HomogeneousCoordinates& Position) const
{
    switch(TransformationType())
    {
    case enumMatrix:
	if (!m_matrixTransformation)
	    throw PointerToTransformationIsNull();
	return (*m_matrixTransformation).transform(Position);
    case enumContainer:
	if (!m_transformationContainer)
	    throw PointerToTransformationIsNull();
	return (*m_transformationContainer).transform(Position);
    };
    return Position;
}


bool cTransformationHandler::AlmostEqual (const cTransformationHandler& trf, double dTolerance) const
{
    switch(TransformationType())
    {
    case enumMatrix:
	if (!m_matrixTransformation)
	    throw PointerToTransformationIsNull();
	return (*m_matrixTransformation).AlmostEqual(*trf.m_matrixTransformation,dTolerance);
    case enumContainer:
	if (!m_transformationContainer)
	    throw PointerToTransformationIsNull();
	return (*m_transformationContainer).AlmostEqual(*trf.m_transformationContainer,dTolerance);
    };
    return false;
}

std::ostream& operator<<(std::ostream& out, const cTransformationHandler& trf)
{
    switch(trf.TransformationType())
    {
    case enumMatrix:
	out << *trf.m_matrixTransformation;
	break;
    case enumContainer:
	out << *trf.m_transformationContainer;
	break;
    };
    return out;
}




class cPixelTransformation 
{
public:
    cPixelTransformation (const cTransformation& metricTransformation, const IKS::RESOLUTION& inputResolution, const IKS::RESOLUTION& outputResolution, bool isResolutionsAppliedToTrans = false)
	: m_metricTransformation(), m_inputResolution (inputResolution), m_outputResolution (outputResolution)
    {
	if (isResolutionsAppliedToTrans)
	{
	    m_metricTransformation += metricTransformation;
	    return;
	}
	IKS::PIXELSIZE pixSizeInput  (inputResolution);
	IKS::PIXELSIZE pixSizeOutput (outputResolution);
	m_metricTransformation += cScaling (pixSizeInput.x(), pixSizeInput.y());
	m_metricTransformation += metricTransformation;
	m_metricTransformation -= cScaling (pixSizeOutput.x(), pixSizeOutput.y());
    }

    cPixelTransformation operator- ()
    {
	return cPixelTransformation (-m_metricTransformation, m_outputResolution, m_inputResolution, true);
    }

    IKS::POINT transform (const IKS::POINT& point) const
    {
	IKS::POSITION pos = m_metricTransformation.transform (IKS::POSITION(point.x(), point.y()));
	return IKS::POINT ((long)(pos.x()+0.5), (long)(pos.y()+0.5));
    }


private:
    cTransformation m_metricTransformation;
    IKS::RESOLUTION m_inputResolution;
    IKS::RESOLUTION m_outputResolution;
};








template <typename T>
std::ostream& paint(std::ostream& out, const T& transformation)
{
    typedef std::vector<char> MLine;
    std::vector<MLine> image;
    size_t sizeX = 60;
    size_t sizeY = 40;
    
    for (size_t i = 0; i < sizeY; ++i)
    {
	image.push_back (MLine());
	for (size_t j = 0; j < sizeX; ++j)
	{
	    image.back().push_back('0');
	}
    }

    for (size_t i = 0; i < sizeY; ++i)
    {
	for (size_t j = 0; j < sizeX; ++j)
	{
	    if (j==5 || (j==sizeX-5))
		image[i][j] = '1';
	    if ((i==5) || (i==sizeY-5))
		image[i][j] = '2';
	    double j2 = (j-sizeX/3); j2 = j2*j2;
	    double i2 = (i-sizeY/3); i2 = i2*i2;
	    if ((i2+j2)<(10*10))
		image[i][j] = '3';
	}
    }

    for (size_t i = 0; i < sizeY; ++i)
    {
	for (size_t j = 0; j < sizeX; ++j)
	{
	    IKS::POSITION pos(j,i);
	    IKS::POSITION pt = transformation.transform(pos);
	    try
	    {
		out << image.at(pt.y()).at(pt.x());
//		out << image.at(pt[1]).at(pt[0]);
	    }
	    catch(...)
	    {
		out << "_";
	    }
	}
	out << std::endl;
    }
    return out;
}



template <typename Iterator>
std::ostream& paint(std::ostream& out, Iterator srcBegin, Iterator srcEnd, Iterator dstBegin, Iterator dstEnd, double factor = 0.01, IKS::POSITION origin = IKS::POSITION(0,0))
{
    typedef std::vector<char> MLine;
    std::vector<MLine> image;
    size_t sizeX = 40;
    size_t sizeY = 40;
    
    for (size_t i = 0; i < sizeY; ++i)
    {
	image.push_back (MLine());
	for (size_t j = 0; j < sizeX; ++j)
	{
	    image.back().push_back('_');
	}
    }

    char c = 'a';
    for (Iterator it = srcBegin; it != srcEnd; ++it)
    {
	typename Iterator::value_type pos = *it;
	pos += origin;
	pos *= factor;
	try
	{
	    image.at(pos.y()).at(pos.x()) = c;
	}
	catch (...)
	{
	}
	++c;
    }

    c = 'A';
    for (Iterator it = dstBegin; it != dstEnd; ++it)
    {
	typename Iterator::value_type pos = *it;
	pos += origin;
	pos *= factor;
	try
	{
	    char cx = image.at(pos.y()).at(pos.x());
	    if (cx >= 'a' && cx < c)
		image.at(pos.y()).at(pos.x()) = 'X';
	    else
		image.at(pos.y()).at(pos.x()) = c;
	} 
	catch(...)
	{
	}
	++c;
    }

    for (size_t i = 0; i < sizeY; ++i)
    {
	for (size_t j = 0; j < sizeX; ++j)
	{
	    try
	    {
		out << image.at(i).at(j);
	    }
	    catch(...)
	    {
		out << "_";
	    }
	}
	out << std::endl;
    }
    return out;
}






namespace IKS
{
    typedef std::vector<IKS::POSITION> VCT_POINTS;
}


/******************************************************************************************************/
cMatrixTransformation  projective(const IKS::VCT_POINTS& srcInPoints, const IKS::RECT& DestRect) 
{
    cMatrixTransformation result;
    if (srcInPoints.size() != 4)
    {
//         throw IKS::Error(E_INVALID_INPUT, "cProjTransformation: 4 source and 4 target points needed but only %d given", 
//                          srcInPoints.size());
    }
    else
    {
	int N = 8 ; //number of parameters to determine (as the 9th parameter is equal to 1 for projective maps)  

	IKS::VCT_POINTS tarPoints; 

	tarPoints.push_back(DestRect.LeftTop()); 
	tarPoints.push_back(DestRect.RightTop());
	tarPoints.push_back(DestRect.RightBottom());
	tarPoints.push_back(DestRect.LeftBottom()); 

	//Initialization of the 8x8 projective matrix plus the nonhomogeneous part pM[i][8]  

	IKS::VCT_POINTS   srcPoints(srcInPoints.size());
	srcPoints.assign (srcInPoints.begin(), srcInPoints.end());
	//Quick fix of problem that 2x inverted matrix suffers from numerical instabilities
	//Therefore for the moment we skip one Inversion and exchange source and target points
	//x_gs 02052011
//         long tmpx, tmpy; 
	// double tmpx, tmpy; 
	// for(int ii=0; ii<4;ii++) 
	// {
	//     tmpx = tarPoints[ii].x(); 
	//     tmpy = tarPoints[ii].y();
	//     tarPoints[ii].x() = srcInPoints[ii].x(); 
	//     tarPoints[ii].y() = srcInPoints[ii].y(); 
	//     srcPoints[ii].x() = tmpx;
	//     srcPoints[ii].y() = tmpy;
	// }
	// /////////////////////////////////////////////////////////////////////////////////////////
       
//	long pM[8][9]; 
	double pM[8][9]; 
 
	pM[0][0] =  srcPoints[0].x() ;  pM[0][1] = srcPoints[0].y() ; 
	pM[0][2] = 1 ; pM[0][3] = 0;  pM[0][4] = 0;   pM[0][5] = 0;   
	pM[0][6] = -srcPoints[0].x()*tarPoints[0].x();   pM[0][7] = -srcPoints[0].y()*tarPoints[0].x();  
	pM[0][8] = tarPoints[0].x(); 

	pM[1][0] = srcPoints[1].x() ;  pM[1][1] = srcPoints[1].y() ; 
	pM[1][2] = 1 ; pM[1][3] = 0;  pM[1][4] = 0;   pM[1][5] = 0;   
	pM[1][6] = -srcPoints[1].x()*tarPoints[1].x();   pM[1][7] = -srcPoints[1].y()*tarPoints[1].x();  
	pM[1][8] = tarPoints[1].x();

	pM[2][0] = srcPoints[2].x() ;  pM[2][1] = srcPoints[2].y() ; 
	pM[2][2] = 1 ; pM[2][3] = 0;  pM[2][4] = 0;   pM[2][5] = 0;   
	pM[2][6] = -srcPoints[2].x()*tarPoints[2].x();   pM[2][7] = -srcPoints[2].y()*tarPoints[2].x();  
	pM[2][8] = tarPoints[2].x();

	pM[3][0] = srcPoints[3].x() ;  pM[3][1] = srcPoints[3].y() ; 
	pM[3][2] = 1 ; pM[3][3] = 0;  pM[3][4] = 0;   pM[3][5] = 0;   
	pM[3][6] = -srcPoints[3].x()*tarPoints[3].x();   pM[3][7] = -srcPoints[3].y()*tarPoints[3].x();  
	pM[3][8] = tarPoints[3].x();

	pM[4][0] = 0;  pM[4][1] = 0;   pM[4][2] = 0;   
	pM[4][3] = srcPoints[0].x() ;  pM[4][4] = srcPoints[0].y() ; pM[4][5] = 1;
	pM[4][6] = -srcPoints[0].x()*tarPoints[0].y();   pM[4][7] = -srcPoints[0].y()*tarPoints[0].y();  
	pM[4][8] = tarPoints[0].y();

	pM[5][0] = 0;  pM[5][1] = 0;   pM[5][2] = 0;   
	pM[5][3] = srcPoints[1].x() ;  pM[5][4] = srcPoints[1].y() ; pM[5][5] = 1;
	pM[5][6] = -srcPoints[1].x()*tarPoints[1].y();   pM[5][7] = -srcPoints[1].y()*tarPoints[1].y(); 
	pM[5][8] = tarPoints[1].y();

	pM[6][0] = 0;  pM[6][1] = 0;   pM[6][2] = 0;   
	pM[6][3] = srcPoints[2].x() ;  pM[6][4] = srcPoints[2].y() ; pM[6][5] = 1;
	pM[6][6] = -srcPoints[2].x()*tarPoints[2].y();   pM[6][7] = -srcPoints[2].y()*tarPoints[2].y(); 
	pM[6][8] = tarPoints[2].y(); 

	pM[7][0] = 0;  pM[7][1] = 0;   pM[7][2] = 0;   
	pM[7][3] = srcPoints[3].x() ;  pM[7][4] = srcPoints[3].y() ; pM[7][5] = 1;
	pM[7][6] = -srcPoints[3].x()*tarPoints[3].y();   pM[7][7] = -srcPoints[3].y()*tarPoints[3].y(); 
	pM[7][8] = tarPoints[3].y(); 		

	//Simple Gauss algorithm for solving the system of linear equations to determine the 8 transformation parameters
	// a - h  (the 9th parameter is 1 )  

	//forwardSubstitution()
                     
	int i, j, k; 
	double dpM[8][9];
              
	//type casting to double 
	for(i=0;i<N;i++)
	{
            for(j=0;j<=N;j++)
		dpM[i][j] = (double) pM[i][j] ;  
	}

	int indx[20];
	double scale[20];
	double maxRatio;
	int maxIndx;
	int tmpIndx;
	double ratio;
	double sum;
	bool err; 

	for (i = 0; i < N; i++) 
            indx[i] = i;	// index array initialization

	// determine scale factors

	for (int row = 0; row < N; row++)
	{
	    scale[row] = abs(dpM[row][0]);
	    for (int col = 1; col < N; col++)
	    {
		if (abs(dpM[row][col]) > scale[row]) scale[row] = abs(dpM[row][col]);
	    }
	}		

	for ( k = 0; k < N; k++)
	{
	    // determine index of pivot row
	    maxRatio = abs(dpM[indx[k]][k])/scale[indx[k]];
	    maxIndx = k;
	    for ( i = k+1; i < N; i++)
	    {
		if (abs(dpM [indx[i]] [k])/scale[indx[i]] > maxRatio)
		{
		    maxRatio = abs(dpM [indx[i]] [k])/scale[indx[i]];
		    maxIndx = i;
		}
	    }
	    if (maxRatio == 0) // no pivot available
	    {
		err = true;
	    }
	    tmpIndx =indx[k]; indx[k]=indx[maxIndx]; indx[maxIndx] = tmpIndx;

	    // use pivot row to eliminate kth variable in "lower" rows
	    for ( i = k+1; i < N; i++)
	    {
		ratio = -dpM [indx[i]] [k]/dpM [indx[k]] [k];
		for (int col = k; col <= N; col++)
		{
		    dpM [indx[i]] [col] += ratio*dpM [indx[k]] [col];
		}
	    }
	}

	// back substitution
	for ( k = N-1; k >= 0; k--)
	{
	    sum = 0;
	    for (int col = k+1; col < N; col++)
	    {
		sum += dpM [indx[k]] [col] * dpM [indx[col]] [N];
	    }
	    dpM [indx[k]][N] = (dpM [indx[k]] [N] - sum)/dpM [indx[k]] [k];
	}

	// // Resulting 8 Parameters
	// for (i = 0; i < N; ++i)
	//     m_MxCoeff[i] = dpM[indx[i]][N]; 

	// m_MxCoeff[N] = 1; 
	// //Gauss_End 

	int count = 0;
	for (int k=0; k<3; ++k)
	{
	    for (int j=0; j<3; ++j)
	    {
		if (count == N)
		    result(k,j, 1);
		else
		    result(k,j, dpM[indx[count]][N]);
		++count;
	    }
	}
    }
    return result;
}












int main()
{
    IKS::POSITION v(2,4);

    double Ms[3][3] = {{2,0,0},{0,3,0},{0,0,1}};
    cTransformationMatrix S(Ms);
    std::cout  << "S     " << S;


    double phi = 0.1;
    double Mr[3][3] = {{cos(phi),-sin(phi),0},{sin(phi),cos(phi),0},{0,0,1}};
    cTransformationMatrix R(Mr);
    std::cout  << "R     "<< R;

    phi = 0.7;
    double Mr2[3][3] = {{cos(phi),-sin(phi),0},{sin(phi),cos(phi),0},{0,0,1}};
    cTransformationMatrix R2(Mr2);
    std::cout  << "R2     "<< R2;

    double Mt[3][3] = {{1,0,5},{0,1,-3},{0,0,1}};
    cTransformationMatrix T(Mt);
    std::cout << "T      "<< T;

    double Mt2[3][3] = {{1,0,2},{0,1,-8},{0,0,1}};
    cTransformationMatrix T2(Mt2);
    std::cout << "T2      "<< T2;

    cMatrixTransformation Mx(cRotation(0.2)+cScaling(3,5)+cTranslation(IKS::POSITION(7,7)));
#define ALL 1
#ifdef ALL
    std::cout << "S  " << (S);
    std::cout << "S^-1  " << (-S);
    std::cout << "S^-1^-1  " << (-S);
    std::cout << std::endl;
    std::cout << "R  " << (R);
    std::cout << "R^-1  " << (-R);
    std::cout << "R^-1^-1  " << (-(-R));
    std::cout << std::endl;
    std::cout << "T  " << (T);
    std::cout << "T^-1  " << (-T);
    std::cout << "T^-1^-1  " << (-(-T));
    std::cout << std::endl;
    std::cout << "S+T+R  " << (S+T+R);
    std::cout << "(S+T+R)^-1  " << (S+T+(-R));
    std::cout << "(R^-1+T^-1+S^-1)  " << (-R+-T+-S);
    std::cout << "(R^-1+T^-1+S^-1)^-1  " << -(-R+-T+-S);
    std::cout << "(R^-1+T^-1+S^-1)^-1 -(S+T+R) " << -(-R+-T+-S) - (S+T+R);
    std::cout << "(S+R+T)- -(S+T+R) " << -(-R+-T+-S) - (S+T+R);
    
    std::cout << "(S+T+R).v " << (S+T+R).transform(v);
    std::cout << "(S+T2+R2).v " << (S+T2+R2).transform(v);
    std::cout << "(((S+T2+R2)-(T2+R2))+(T+R)).v " << (((S+T2+R2)-(T2+R2))+(T+R)).transform(v);

    std::cout << "Mx  " << (Mx);


    cTransformation trf(S+T);
    std::cout << "S+T" << std::endl << trf << std::endl;
    trf += R;
    std::cout << "S+T+R" << std::endl << trf << std::endl;
    trf += S;
    std::cout << "S+T+R+S" << std::endl << trf << std::endl;
    trf -= S;
    std::cout << "S+T+R+S-S == S+T+R" << std::endl <<  trf << std::endl;
    trf -= R;
    std::cout << "S+T+R+S-S-R == S+T" << std::endl << trf << std::endl;
    trf += -(S+T);
    std::cout << "S+T+R+S-S-R-(S+T) == 1" << std::endl << trf << std::endl;
    

    std::cout << "(S).v   = " << cTransformation(S).transform(v) << std::endl;
    std::cout << "(S+T).v   = " << cTransformation(S+T).transform(v) << std::endl;
    std::cout << "(S+T+R).v   = " << cTransformation(S+T+R).transform(v) << std::endl;
    std::cout << "(S+T+R+S).v   = " << cTransformation(S+T+R+S).transform(v) << std::endl;

    std::cout << "[cTransformation(S)-cTransformation(S)].v   = " << (cTransformation(S)-cTransformation(S)).transform(v) << std::endl;
    std::cout << "[cTransformation(S+T+R)-cTransformation(S+T+R)].v   = " << (cTransformation(S+T+R)-cTransformation(S+T+R)).transform(v) << std::endl;
    std::cout << "[cTransformation(S+T+R)-cTransformation(S+T)].v   = " << (cTransformation(S+T+R)-cTransformation(S+T)).transform(v) << std::endl;
    std::cout << "[cTransformation(S+T+R)-(S+T)].v   = " << (cTransformation(S+T+R)-(S+T)).transform(v) << std::endl;
    std::cout << std::endl<< std::endl;
    std::cout << "[ (S+T)].v   = " << ( (S+T)).transform(v) << std::endl;
    std::cout << "[ (T+S)].v   = " << ( (T+S)).transform(v) << std::endl;
    std::cout << "[ (S*T)].v   = " << ( (S*T)).transform(v) << std::endl;
    std::cout << "[ (T*S)].v   = " << ( (T*S)).transform(v) << std::endl;
    std::cout << "[ ct(S+T)].v   = " << ( cTransformation(S+T)).transform(v) << std::endl;
    std::cout << "[ ct(T+S)].v   = " << ( cTransformation(T+S)).transform(v) << std::endl;
    std::cout << "[ ct(S*T)].v   = " << ( cTransformation(S*T)).transform(v) << std::endl;
    std::cout << "[ ct(T*S)].v   = " << ( cTransformation(T*S)).transform(v) << std::endl;
    std::cout << "[ (ct(S)+ct(T))].v   = " << ( (cTransformation(S)+cTransformation(T))).transform(v) << std::endl;
    std::cout << "[ (ct(T)+ct(S))].v   = " << ( (cTransformation(T)+cTransformation(S))).transform(v) << std::endl;
    std::cout << std::endl<< std::endl;
    std::cout << "[-(S+T)].v   = " << (-(S+T)).transform(v) << std::endl;
    std::cout << "[-(T+S)].v   = " << (-(T+S)).transform(v) << std::endl;
    std::cout << "[-(S*T)].v   = " << (-(S*T)).transform(v) << std::endl;
    std::cout << "[-(T*S)].v   = " << (-(T*S)).transform(v) << std::endl;
    std::cout << "[-ct(S+T)].v   = " << (-cTransformation(S+T)).transform(v) << std::endl;
    std::cout << "[-ct(T+S)].v   = " << (-cTransformation(T+S)).transform(v) << std::endl;
    std::cout << "[-ct(S*T)].v   = " << (-cTransformation(S*T)).transform(v) << std::endl;
    std::cout << "[-ct(T*S)].v   = " << (-cTransformation(T*S)).transform(v) << std::endl;
    std::cout << "[-(ct(S)+ct(T))].v   = " << (-(cTransformation(S)+cTransformation(T))).transform(v) << std::endl;
    std::cout << "[-(ct(T)+ct(S))].v   = " << (-(cTransformation(T)+cTransformation(S))).transform(v) << std::endl;
    std::cout << "[-ct(T)+-ct(S))].v   = " << ((-cTransformation(T)+-cTransformation(S))).transform(v) << std::endl;
    std::cout << "[-ct(T)- ct(S))].v   = " << ((-cTransformation(T) -cTransformation(S))).transform(v) << std::endl;
    std::cout << "[-ct(S)+-ct(T))].v   = " << ((-cTransformation(S)+-cTransformation(T))).transform(v) << std::endl;
    std::cout << "[-ct(S)- ct(T))].v   = " << ((-cTransformation(S) -cTransformation(T))).transform(v) << std::endl;
    std::cout << std::endl<< std::endl;



    std::cout << "[ct(ct(S)+ct(T)+ct(R))-ct(ct(S)+ct(T)+ct(R))].v   = " << (cTransformation(cTransformation(S)+cTransformation(T)+cTransformation(R))-cTransformation(cTransformation(S)+cTransformation(T)+cTransformation(R))).transform(v) << std::endl;

    std::cout << "[cTransformation(S+T+R)-cTransformation(S+T+R)].v   = " << (cTransformation(S+T+R)-cTransformation(S+T+R)).transform(v) << std::endl;
    std::cout << "[cTransformation(S+T+R)-(S+T+R)].v   = " << (cTransformation(S+T+R)-(S+T+R)).transform(v) << std::endl;
    std::cout << "[cTransformation(S+T+R)-cTransformation(S+T)].v   = " << (cTransformation(S+T+R)-cTransformation(S+T)).transform(v) << std::endl;
    std::cout << "[cTransformation(S+T+R)-(S+T)].v   = " << (cTransformation(S+T+R)-(S+T)).transform(v) << std::endl;
    std::cout << "[ct(S+T+R)-ct(S+T+R)].v   = " << (cTransformation(S+T+R)-cTransformation(S+T+R)).transform(v) << std::endl;

    std::cout << "[ct(S+T+R)-ct(S+T)]     = " << (cTransformation(S+T+R)-cTransformation(S+T)) << std::endl;
    std::cout << "[ct(S+T+R)-ct(S+T+R)]   = " << (cTransformation(S+T+R)-cTransformation(S+T+R)) << std::endl;
    std::cout << "[ct(S+T)-ct(S+T)]   = " << (cTransformation(S+T)-cTransformation(S+T)) << std::endl;
    std::cout << "[ct(S)-ct(S)]   = " << (cTransformation(S)-cTransformation(S)) << std::endl;

    std::cout << "[cTransformation(S+T+R)-cTransformation(S+T)].v   = " << (cTransformation(S+T+R)-cTransformation(S+T)).transform(v) << std::endl;
    std::cout << "[cTransformation(S+T+R)-(S+T)].v   = " << (cTransformation(S+T+R)-(S+T)).transform(v) << std::endl;

    std::cout << "[cmt(S+T+R)-cmt(S+T))].v   = " << (cMatrixTransformation(S+T+R)-cMatrixTransformation(S+T)).transform(v) << std::endl;
    std::cout << "[ct(S+T+R)-cmt(S+T)].v   = " << (cTransformation(S+T+R)-cMatrixTransformation(S+T)).transform(v) << std::endl;
    std::cout << "[ct(S+T+R)-ct(cmt(S+T))].v   = " << (cTransformation(S+T+R)-cTransformation(cMatrixTransformation(S+T))).transform(v) << std::endl;
//    std::cout << "[ct(S+T+R)-cmt(S+T)].v   = " << (cTransformation(S+T+R)-cMatrixTransformation(S+T)).transform(v) << std::endl;


//#define INVINVTEST 1
#ifdef INVINVTEST
    {
	double maxDim = 3000;
	double step = 750;
	for (double leftT= 0; leftT < maxDim-step; leftT+=step)
	    for (double leftB= 0; leftB < maxDim-step; leftB+=step)
		for (double topL = 0; topL<maxDim-step; topL+=step)
		    for (double topR = 0; topR<maxDim-step; topR+=step)
			for (double rightT = leftT+step; rightT<maxDim; rightT+=step)
			    for (double rightB = leftB+step; rightB<maxDim; rightB+=step)
				for (double bottomL = topL+step; bottomL<maxDim; bottomL+=step)
				    for (double bottomR = topR+step; bottomR<maxDim; bottomR+=step)
				    {

					std::vector<IKS::POSITION> dstPoints;
					dstPoints.push_back (IKS::POSITION(0,0));
					dstPoints.push_back (IKS::POSITION(3000,0));
					dstPoints.push_back (IKS::POSITION(3000,2000));
					dstPoints.push_back (IKS::POSITION(0,2000));


					std::vector<IKS::POSITION> srcPoints;
					srcPoints.push_back (IKS::POSITION(leftT,topL));
					srcPoints.push_back (IKS::POSITION(rightT,topR));
					srcPoints.push_back (IKS::POSITION(rightB,bottomR));
					srcPoints.push_back (IKS::POSITION(leftB,bottomL));

//					srcPoints.swap (dstPoints);
					try
					{
					    cProjectiveTransformation P(srcPoints, dstPoints);
					    cMatrixTransformation invP = (-(-P));

					    IKS::RECT rect (dstPoints.at(0), dstPoints.at(2));
					    cMatrixTransformation P2 = projective (srcPoints, rect);

					    // std::cout << "P  " << P  << std::endl;
					    // std::cout << "P2 " << P2 << std::endl;

					    // perform the transformation of the points
					    bool transformationOK_P(true);
					    bool invTransformationOK_P(true);
					    for (std::vector<IKS::POSITION>::const_iterator itSrc = srcPoints.begin(), itSrcEnd = srcPoints.end(), itDst = dstPoints.begin(); 
						 itSrc != itSrcEnd; ++itSrc, ++itDst)
					    {
						IKS::POSITION vp = P.transform ( (*itSrc) );
						double dist = (vp - (*itDst)).norm();
//						std::cout << "P  dist " << dist << "  (*itSrc) " << (*itSrc) << "  tr " << vp << "  (*itDst) " << (*itDst) << std::endl;
						if (dist>EPSILON || dist!=dist)
						    transformationOK_P = false;
						IKS::POSITION vpI = (-P).transform (vp);
						dist = (vpI - (*itSrc)).norm();
//						std::cout << "P  dist " << dist << "  (*itSrc) " << (*itSrc) << "  tr " << vp << "  (*itDst) " << (*itDst) << std::endl;
						if (dist>EPSILON || dist!=dist)
						    invTransformationOK_P = false;
					    }

					    bool transformationOK_P2(true);
					    bool invTransformationOK_P2(true);
					    for (std::vector<IKS::POSITION>::const_iterator itSrc = srcPoints.begin(), itSrcEnd = srcPoints.end(), itDst = dstPoints.begin(); 
						 itSrc != itSrcEnd; ++itSrc, ++itDst)
					    {
						IKS::POSITION vp = P2.transform ( (*itSrc) );
						double dist = (vp - (*itDst)).norm();
//						std::cout << "P2 dist " << dist << "  (*itSrc) " << (*itSrc) << "  tr " << vp << "  (*itDst) " << (*itDst) << std::endl;
						if (dist>EPSILON || dist!=dist)
						    transformationOK_P2 = false;
						IKS::POSITION vpI = (-P2).transform (vp);
						dist = (vpI - (*itSrc)).norm();
//						std::cout << "P  dist " << dist << "  (*itSrc) " << (*itSrc) << "  tr " << vp << "  (*itDst) " << (*itDst) << std::endl;
						if (dist>EPSILON || dist!=dist)
						    invTransformationOK_P2 = false;
					    }

					    if (!invP.AlmostEqual(P,1) || !P2.AlmostEqual(P,1) || !transformationOK_P || !transformationOK_P2 || !invTransformationOK_P || !invTransformationOK_P2)
					    {
						std::cout << "src : ";
						for (std::vector<IKS::POSITION>::const_iterator it = srcPoints.begin(), itEnd = srcPoints.end(); it != itEnd; ++it)
						    std::cout << "|" << (*it);

						std::cout << std::endl;
						std::cout << "dst : ";
						for (std::vector<IKS::POSITION>::const_iterator it = dstPoints.begin(), itEnd = dstPoints.end(); it != itEnd; ++it)
						    std::cout << "|" << (*it);

						std::cout << std::endl;
					    
						paint (std::cout, srcPoints.begin(), srcPoints.end(), dstPoints.begin(), dstPoints.end());

						std::cout << "   ==> ";
						std::cout << "-(-P)==P   == " << (invP==P ? "true" : "false") << std::endl;
						std::cout << "P    ==P2  == " << (P==P2 ? "true" : "false") << std::endl;
						std::cout << "P.transform  == " << (transformationOK_P  ? "true" : "false") << std::endl;
						std::cout << "P2.transform == " << (transformationOK_P2 ? "true" : "false") << std::endl;
						std::cout << "-P.transform  == " << (invTransformationOK_P  ? "true" : "false") << std::endl;
						std::cout << "-P2.transform == " << (invTransformationOK_P2 ? "true" : "false") << std::endl;

//					    std::cout << "P" << P << "   det(P)=" << P.determinant() << std::endl;
						std::cout << "   P " << P;
						std::cout << "-(-P)" << invP;
						std::cout << "P2   " << P2;

						std::cout << "difference" << P.subElem(invP) << std::endl;


						if (!transformationOK_P || !transformationOK_P2)
						    for (std::vector<IKS::POSITION>::const_iterator itSrc = srcPoints.begin(), itSrcEnd = srcPoints.end(), itDst = dstPoints.begin(); 
							 itSrc != itSrcEnd; ++itSrc, ++itDst)
						    {
							IKS::POSITION vp = P.transform ( (*itSrc) );
							std::cout << "P.transform(src)  = " << vp << std::endl;
							std::cout << "dst               = " << (*itDst) << std::endl;
						    }

						if (!transformationOK_P2 || !transformationOK_P)
						    for (std::vector<IKS::POSITION>::const_iterator itSrc = srcPoints.begin(), itSrcEnd = srcPoints.end(), itDst = dstPoints.begin(); 
							 itSrc != itSrcEnd; ++itSrc, ++itDst)
						    {
							IKS::POSITION vp = P2.transform ( (*itSrc) );
							std::cout << "P2.transform(src) = " << vp << std::endl;
							std::cout << "dst               = " << (*itDst) << std::endl;
						    }


						// paint(std::cout, P);						
						// std::cout << std::endl;
						// paint(std::cout, P2);						
						

//						return 0;
					    }
					    else if (true)
					    {
						for (std::vector<IKS::POSITION>::const_iterator it = srcPoints.begin(), itEnd = srcPoints.end(); it != itEnd; ++it)
						{
						    std::cout << "|" << (*it);
						}
						std::cout << std::endl;
					    }
					}
					catch(cProjectiveTransformation::InvalidInput& excpt)
					{
//					std::cout << "Invalid Input for projection transformation" << std::endl;
					}
				    }
	std::cout << std::endl;
	std::cout << std::endl;
    }
#endif

    // double vals[3][3] = {{0.0333333, 0.2, 0},{-0.1,0.25,300},{-0.000305556,0.000166667,1}};
    // cTransformationMatrix M(vals);
    // std::cout << "M" << M << std::endl;

    // std::cout << "-(-M)" << (-(-M)) << std::endl;

    // std::cout << std::endl;
    // std::cout << std::endl;

#endif
    double vals2[3][3] = {{ 0.133333, 0.1, 0 },  { -0.133333, -0.1, 1200 },  { -0.000111111, -0.000166667, 1 }};
    cTransformationMatrix M2(vals2);
    std::cout << "M2" << M2 << std::endl;
    std::cout << "(-M2)" << ((-M2)) << std::endl;
    std::cout << "-(-M2)" << (-(-M2)) << std::endl;
    std::cout << M2.subElem(-(-M2)) << std::endl;

    std::vector<IKS::POSITION> srcPoints;
    std::vector<IKS::POSITION> dstPoints;

    srcPoints.push_back (IKS::POSITION(500,0));
    srcPoints.push_back (IKS::POSITION(700,100));
    srcPoints.push_back (IKS::POSITION(3000,2000));
    srcPoints.push_back (IKS::POSITION(0,2000));

    dstPoints.push_back (IKS::POSITION(0,0));
    dstPoints.push_back (IKS::POSITION(3000,0));
    dstPoints.push_back (IKS::POSITION(3000,2000));
    dstPoints.push_back (IKS::POSITION(0,2000));

    cProjectiveTransformation P(srcPoints.begin(), srcPoints.end(), dstPoints.begin());
    std::cout << "projective: " << P << std::endl;
    std::cout << "projective test: " << -(-P) << "   equal: " << (-(-P)==P ? "true" : "false") << std::endl;



    IKS::POSITION w(600,1500);

    std::cout << "-P.P = " << (-P+P) << std::endl;
    std::cout << "(-P.P).w = " << (-P+P).transform(w) << std::endl;
    std::cout << "-P.P.w = " << ((-P).transform(P.transform(w))) << std::endl;
    std::cout << "w  = " << w << std::endl;
    IKS::POSITION wt(P.transform(w));
    std::cout << "P.w = " << (wt) << std::endl;
    std::cout << "(-P).wt = " << ((-P).transform(wt)) << std::endl;
    // std::cout << "(-P).wt[z==1] = " << ((-P).transform(wt)) << std::endl;


    std::vector<IKS::POSITION> src;
    std::vector<IKS::POSITION> dst;

    src.push_back (IKS::POSITION(10,5));
    src.push_back (IKS::POSITION(40,10));
    src.push_back (IKS::POSITION(70,30));
    src.push_back (IKS::POSITION(0,40));

    dst.push_back (IKS::POSITION(0,0));
    dst.push_back (IKS::POSITION(60,0));
    dst.push_back (IKS::POSITION(60,40));
    dst.push_back (IKS::POSITION(0,40));

    cProjectiveTransformation U(src.begin(), src.end(), dst.begin());
    std::cout << U << std::endl;

    IKS::RECT rect (dst.at(0), dst.at(2));
    cMatrixTransformation U2 = projective (src, rect);
    std::cout << "U2 " << U2 << std::endl;


    paint(std::cout, cMatrixTransformation());
    std::cout << std::endl;
    paint(std::cout, U);
//  paint(std::cout, cTranslation(10,10));
    // IKS::HomogeneousCoordinates mypos(0,0,1);
    // std::cout << cTranslation(10,10).transform(mypos) << std::endl;

    
    // Matrix matrix;
    // for (int i=0; i<3; ++i)
    // {
    // 	matrix.push_back (MatrixLine());
    // 	for (int j=0; j<4; ++j)
    // 	{
    // 	    matrix.back().push_back(0);
    // 	}
    // }

    // matrix[0][0] = 1; 
    // matrix[0][1] = 2;
    // matrix[0][2] = -1;
    // matrix[0][3] = -4;
    // matrix[1][0] = 2;
    // matrix[1][1] = 3;
    // matrix[1][2] = -1;
    // matrix[1][3] = -11;
    // matrix[2][0] = -2;
    // matrix[2][1] = 0;
    // matrix[2][2] = -3;
    // matrix[2][3] = 22;

    // std::cout << matrix << std::endl;
    // matrix[0].swap(matrix[2]);

    // std::cout << matrix << std::endl;
    // reducedRowEchelonForm(matrix);
    // std::cout << matrix << std::endl;


    std::cout << "=========================" << std::endl;

    

    phi = 3.1415927/2.;
    double Mr3[3][3] = {{cos(phi),-sin(phi),0},{sin(phi),cos(phi),0},{0,0,1}};
    cTransformationMatrix R3(Mr3);
    std::cout  << "R3     "<< R3;
    std::cout  << "-R3     "<< -R3;

    cTranslation T3(IKS::POSITION(0,667));
    cTransformationMatrix M = R3+T3;
    std::cout << "M  " << M;
    std::cout << "-M " << (-M);


    {
    double Mtest[3][3] = {{-1.78284230129339,-4.07891268413354E-02,281.94},{-0.008201581275015,-1.79756149566903,211.709},{1.13754981522559E-04,-3.63368293822716E-04,1}};
    cTransformation TTest ( (cTransformationMatrix(Mtest)) );
    
    std::cout << "TTest: " << TTest << std::endl;
    std::cout << "-TTest: " << (-TTest) << std::endl;
    std::cout << "-(-TTest): " << (-(-TTest)) << std::endl;
    std::cout << "-(-TTest) == TTest: " << ((-(-TTest))==TTest) << std::endl;


    TTest += cScaling (2.777777, 2.777777);
    std::cout << "TTest+S: " << TTest << std::endl;
    std::cout << "-(TTest+S): " << (-TTest) << std::endl;
    std::cout << "-(-(TTest+S)): " << (-(-TTest)) << std::endl;
    std::cout << "-(-(TTest+S)) == (TTest+S): " << ((-(-TTest))==TTest) << std::endl;

    cTransformationMatrix cmpA(Mtest);
    double cmpBvals[3][3] = {{-0.550639174345929,-1.97321678433632E-02,159.424686337042},
			     {1.03317371458833E-02,-0.580811169282092,120.050021866632},
			     {6.63921748022426E-05,-2.08803731226789E-04,1.02548701937055}};
    cTransformationMatrix cmpB(cmpBvals);

    std::cout << "difference " << (-cmpA).subElem(cmpB) << std::endl;


    IKS::POSITION posTest(335.357360406091, 74.2659898477157);


    std::cout << "posTest = " << posTest << std::endl;
    std::cout << "(-TTest).posTest: " << ((-TTest).transform (posTest)) << std::endl;
    std::cout << "TTest.((-TTest).posTest): " << TTest.transform(((-TTest).transform (posTest))) << std::endl;
    std::cout << std::endl;

    IKS::POSITION posTestBack(92.1949058025536, 105.512230671956);
    std::cout << "posTestBack = " << posTestBack << std::endl;
    std::cout << "TTest.posTestBack: " << (TTest.transform (posTestBack)) << std::endl;
    std::cout << "(-TTest).(TTest.posTestBack): " << (-TTest).transform((TTest.transform (posTestBack))) << std::endl;
    std::cout << std::endl;
    }


    std::cout << "---------------------" << std::endl;
    {
    double Mtest[3][3] = {{-1.78284230129339,-4.07891268413354E-02,281.94},{-0.008201581275015,-1.79756149566903,211.709},{1.13754981522559E-04,-3.63368293822716E-04,1}};
    cTransformation TTest ( (cTransformationMatrix(Mtest)) );
    
    std::cout << "TTest: " << TTest << std::endl;
    std::cout << "-TTest: " << (-TTest) << std::endl;
    std::cout << "-(-TTest): " << (-(-TTest)) << std::endl;
    std::cout << "-(-TTest) == TTest: " << ((-(-TTest))==TTest) << std::endl;


    cTransformation Sc( cScaling (2.777777, 2.777777));

    IKS::POSITION posTest(335.357360406091, 74.2659898477157);

    std::cout << "posTest = " << posTest << std::endl;
    std::cout << "(-TTest).transform((-Sc).transform(posTest)): " << ((-TTest).transform((-Sc).transform (posTest))) << std::endl;
    std::cout << "Sc.transform(TTest.transform(((-TTest).transform((-Sc).transform(posTest))))): " << (Sc.transform(TTest.transform(((-TTest).transform((-Sc).transform(posTest)))))) << std::endl;
    std::cout << std::endl;

    IKS::POSITION posTestBack(92.1949058025536, 105.512230671956);
    std::cout << "posTestBack = " << posTestBack << std::endl;
    std::cout << "Sc.transform(TTest.transform(posTestBack)): " << (Sc.transform(TTest.transform(posTestBack))) << std::endl;
    std::cout << "(-TTest).transform((-Sc).transform((Sc.transform(TTest.transform(posTestBack))))): " << ((-TTest).transform((-Sc).transform((Sc.transform(TTest.transform(posTestBack)))))) << std::endl;
    std::cout << std::endl;



    double cmpBvals[3][3] = {{-0.550639174345929,-1.97321678433632E-02,159.424686337042},
			     {1.03317371458833E-02,-0.580811169282092,120.050021866632},
			     {6.63921748022426E-05,-2.08803731226789E-04,1.02548701937055}};
    cTransformation invTTest ( (cTransformationMatrix (cmpBvals)) );



    std::cout << "posTest = " << posTest << std::endl;
    std::cout << "(invTTest).transform((-Sc).transform(posTest)): " << ((invTTest).transform((-Sc).transform (posTest))) << std::endl;
    std::cout << "Sc.transform(TTest.transform(((invTTest).transform((-Sc).transform(posTest))))): " << (Sc.transform(TTest.transform(((invTTest).transform((-Sc).transform(posTest)))))) << std::endl;
    std::cout << std::endl;


    }

    IKS::RESOLUTION inputResolution  (72, 143);
    IKS::RESOLUTION outputResolution (300, 500);
    cPixelTransformation pixTrans ( (cTransformation(R+S+T)) , inputResolution, outputResolution);

    IKS::POINT myPos (345, 8783);
    IKS::POINT myPosTrans = pixTrans.transform (myPos);
    IKS::POINT myPosTransInv = (-pixTrans).transform (myPosTrans);

    std::cout << "(-pixelTransformation).transform (pixelTransformation.transform (point)) == point  " <<  (myPos.distance (myPosTransInv)< 0.00001) << std::endl;


}


