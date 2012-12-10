
#ifndef __Classifier_h__
#define __Classifier_h__

#include "Prereqs.h"
#include "DataSet.h"

namespace MLLib
{

/**
* Classifier type.
*/
enum ClassifierType
{
	CT_Bayes, ///< Bayesian classifier.
	CT_SVM ///< SVM classifier.
};

/**
* Base class for data classifiers.
*/
class Classifier
{

public:

	/**
	* Destructor.
	*/
	virtual ~Classifier(){ }

	/**
	* Gets the classifier type.
	*/
	virtual ClassifierType getType() const = 0;

	/**
	* Trains the classifier using a training dataset.
	*
	* @param data Training data.
	*/
	virtual void train( DataSet* data ) = 0;

	/**
	* Resets the classifier, undoing any learned structures.
	*/
	virtual void reset() = 0;

	/**
	* Classifies the instances from the specified dataset.
	*
	* @param data Dataset to classify.
	*/
	virtual void classify( DataSet* data ) = 0;

	/**
	* Tests the classifier on a given dataset,
	* assuming that value 1 for the class attribute is positive.
	* Prints out the test results to std::cout.
	*
	* @param data Test dataset.
	*/
	virtual void test( DataSet* data );

};

}

#endif // __Classifier_h__
