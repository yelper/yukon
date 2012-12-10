
#ifndef __SVMClassifier_h__
#define __SVMClassifier_h__

#include "Prereqs.h"
#include "Classifier.h"

#define KKT_EPSILON 0.001

namespace MLLib
{

enum KernelType
{
	KT_Linear,
	KT_Poly,
	KT_Gauss
};

/**
* Class representing a support vector machine classifier.
*/
class SVMClassifier : public Classifier
{

public:

	/**
	* Constructor.
	*/
	SVMClassifier();

	/**
	* Destructor.
	*/
	~SVMClassifier();

	/**
	* Gets the classifier type.
	*/
	ClassifierType getType() const { return CT_SVM; }

	/**
	* Gets the regularization constant.
	*/
	virtual double getC() const { return mC; }

	/**
	* Sets the regularization constant.
	*/
	virtual void setC( double c ) { mC = c; }

	/**
	* Gets the kernel type.
	*/
	virtual KernelType getKernelType() const { return mKernelType; }

	/**
	* Sets the kernel type.
	*/
	virtual void setKernelType( KernelType kt ) { mKernelType = kt; }

	/**
	* Gets the degree for the polynomial kernel.
	*/
	virtual double getD() const { return mD; }

	/**
	* Sets the degree for the polynomial kernel.
	*/
	virtual void setD( int d ) { mD = d; }

	/**
	* Gets the sigma for the Gaussian kernel.
	*/
	virtual double getS() const { return mS; }

	/**
	* Sets the sigma for the Gaussian kernel.
	*/
	virtual void setS( double s ) { mS = s; }

	/**
	* Trains the SVM classifier using the SMO method.
	*
	* @param Training data.
	*/
	void train( DataSet* data );

	/**
	* Resets the classifier.
	*/
	void reset();

	/**
	* Classifies the instances from the specified dataset.
	*
	* @param data Dataset to classify.
	*/
	void classify( DataSet* data );

protected:

	virtual bool _processExample( int xi2 );
	virtual bool _advanceSMO( int xi1, int xi2, double err2 );
	inline double _query( int xi ) const;
	inline double _query( const DataPoint& dpt ) const;
	inline double _error( int xi );
	inline void _updateErrorCache();
	inline double _dot( const DataPoint& dpt1, const DataPoint& dpt2 ) const;
	inline double _subNormSq( const DataPoint& dpt1, const DataPoint& dpt2 ) const;
	inline double _kernel( int xi1, int xi2 ) const;
	inline double _kernel( const DataPoint& dpt1, const DataPoint& dpt2 ) const;
	inline double _kPoly( const DataPoint& dpt1, const DataPoint& dpt2, int d ) const;
	inline double _kGauss( const DataPoint& dpt1, const DataPoint& dpt2, double s ) const;
	inline bool _isBound( int xi ) const;
	inline void _cacheKernels();

	double mC; // regularization constant
	KernelType mKernelType;
	int mD; // polynomial degree
	double mS; // sigma (in the Gaussian kernel)

	std::vector<DataPoint> mX; // training examples
	std::map<int,double> mUXCache; // unbounded examples and their cached errors
	std::map<int,double>::iterator mUXIter; // currently processed unbounded example
	std::vector<double> mA; // Lagrangian multipliers for each example
	std::vector<double> mY; // class values (1 or -1) for each example
	double mB; // b param. of the SVM (threshold)
	std::vector<double> mKernelCache; // cached kernel values

};

}

#endif // __SVMClassifier_h__
