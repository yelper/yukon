
#ifndef __MLLib_h__
#define __MLLib_h__

#include "Prereqs.h"
#include "Singleton.h"
#include "DataSet.h"
#include "Classifier.h"
#include "BayesClassifier.h"
#include "SVMClassifier.h"

#define MLLIB MLLib::MLLibCore::Instance()

namespace MLLib
{

/**
* Core class of the machine learning library.
*
* Provides support for creating, loading, and saving datasets
* and classifiers.
*
* @remark This class is a singleton. To access its instance,
* use the MLLIB macro.
*/
class MLLibCore : public Singleton<MLLibCore>
{

public:

	/**
	* Constructor.
	*/
	MLLibCore();

	/**
	* Destructor.
	*/
	virtual ~MLLibCore();

	/**
	* Creates an empty dataset.
	*
	* @return Data set.
	*/
	virtual DataSet* createDataSet();

	/**
	* Deletes the specified dataset.
	*
	* @param index Dataset index.
	*/
	virtual void deleteDataSet( int index );

	/**
	* Deletes all datasets.
	*/
	virtual void deleteAllDataSets();

	/**
	* Gets the specified data set.
	*
	* @param index Dataset index.
	* @return Dataset.
	*/
	virtual DataSet* getDataSet( int index ) const;

	/**
	* Gets the number of datasets.
	*
	* @param Number of datasets.
	*/
	virtual int getNumDataSets() const;

	/**
	* Finds the index of the specified dataset.
	*
	* @param Dataset.
	* @return Dataset index.
	*/
	virtual int findDataSetIndex( DataSet* data ) const;

	/**
	* Creates a copy of the specified dataset.
	*
	* @param data Input dataset.
	* @return Copy of the dataset.
	*/
	virtual DataSet* cloneDataSet( DataSet* data );
	
	/**
	* Loads data from a file and fills the specified dataset.
	*
	* @param path Dataset file path.
	* @param data Dataset to fill.
	* @return true if loading was successful, false otherwise.
	*/
	virtual bool loadDataSet( const std::string& path, DataSet* data );

	/**
	* Saves the dataset to a file.
	*
	* @param path Dataset file path.
	* @param data Dataset to save.
	* @return true if saving was successful, false otherwise.
	*/
	virtual bool saveDataSet( const std::string& path, DataSet* data );

	/**
	* Creates a new data classifier.
	*
	* @param ct Classifier type.
	* @return Classifier.
	*/
	virtual Classifier* createClassifier( ClassifierType ct );

	/**
	* Deletes the specified classifier.
	*
	* @param index Classifier index.
	*/
	virtual void deleteClassifier( int index );

	/**
	* Deletes all classifiers.
	*/
	virtual void deleteAllClassifiers();

	/**
	* Gets the specified classifier.
	*
	* @param index Classifier index.
	*/
	virtual Classifier* getClassifier( int index ) const;

	/**
	* Gets the number of classifiers.
	*
	* @return Number of classifiers.
	*/
	virtual int getNumClassifiers() const;

	/**
	* Finds the index of the specified classifier.
	*
	* @param Classifier.
	* @return Classifier index.
	*/
	virtual int findClassifierIndex( Classifier* cl ) const;

protected:

	std::vector<DataSet*> mData; ///< Datasets.
	std::vector<Classifier*> mClassifiers; ///< Classifiers.
	
};

}

#endif // __MLLib_h__
