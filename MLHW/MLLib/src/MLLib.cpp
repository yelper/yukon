
#include "MLLib.h"
#include "ARFFDataSetIO.h"
#include "BayesClassifier.h"

namespace MLLib
{

MLLibCore::MLLibCore()
{
}

MLLibCore::~MLLibCore()
{
	// Delete all data sets
	for( int dsi = 0; dsi < getNumDataSets(); ++dsi )
		delete mData[dsi];
}

DataSet* MLLibCore::createDataSet()
{
	DataSet* data = new DataSet();
	mData.push_back(data);

	return data;
}

void MLLibCore::deleteDataSet( int index )
{
	mlAssert( index >= 0 && index < getNumDataSets() );

	delete mData[index];
	mData.erase( mData.begin() + index );
}

void MLLibCore::deleteAllDataSets()
{
	while( getNumDataSets() > 0 )
		deleteDataSet(0);
}

DataSet* MLLibCore::getDataSet( int index ) const
{
	mlAssert( index >= 0 && index < getNumDataSets() );

	return mData[index];
}

int MLLibCore::getNumDataSets() const
{
	return (int)mData.size();
}

int MLLibCore::findDataSetIndex( DataSet* data ) const
{
	for( int dsi = 0; dsi < getNumDataSets(); ++dsi )
	{
		if( mData[dsi] == data )
			return dsi;
	}

	return -1;
}

DataSet* MLLibCore::cloneDataSet( DataSet* data )
{
	mlAssert( data != NULL );

	DataSet* cdata = data->_clone();
	mData.push_back(cdata);

	return cdata;
}

bool MLLibCore::loadDataSet( const std::string& path, DataSet* data )
{
	mlAssert( data != NULL );

	bool success = false;

	// Using file extension, determine which loader to use
	std::string lpath = path;
	std::transform( lpath.begin(), lpath.end(), lpath.begin(), ::tolower );
	size_t exti = -1;
	if( ( exti = path.rfind(".arff") ) != std::string::npos &&
		exti == lpath.length() - 5 )
	{
		// Load the data set

		ARFFDataSetIO dsio(data);
		success = dsio.read(path);
	}

	if(!success)
	{
		std::cout << "ERROR: Unable to load dataset file " << path << std::endl;
	}

	return success;
}

bool MLLibCore::saveDataSet( const std::string& path, DataSet* data )
{
	mlAssert( data != NULL );

	bool success = false;

	// Using file extension, determine which loader to use
	std::string lpath = path;
	std::transform( lpath.begin(), lpath.end(), lpath.begin(), ::tolower );
	size_t exti = -1;
	if( ( exti = path.rfind(".arff") ) != std::string::npos &&
		exti == lpath.length() - 5 )
	{
		// Load the data set

		ARFFDataSetIO dsio(data);
		success = dsio.write(path);
	}

	if(!success)
	{
		std::cout << "ERROR: Unable to save dataset file " << path << std::endl;
	}

	return success;
}

Classifier* MLLibCore::createClassifier( ClassifierType ct )
{
	Classifier* cl = NULL;
	if( ct == CT_Bayes )
	{
		cl = new BayesClassifier();
	}
	else // if( ct == CT_SVM )
	{
		cl = new SVMClassifier();
	}
	
	mClassifiers.push_back(cl);

	return cl;
}

void MLLibCore::deleteClassifier( int index )
{
	mlAssert( index >= 0 && index < getNumClassifiers() );

	delete mClassifiers[index];
	mClassifiers.erase( mClassifiers.begin() + index );
}

void MLLibCore::deleteAllClassifiers()
{
	while( getNumClassifiers() > 0 )
		deleteClassifier(0);
}

Classifier* MLLibCore::getClassifier( int index ) const
{
	mlAssert( index >= 0 && index < getNumClassifiers() );

	return mClassifiers[index];
}

int MLLibCore::getNumClassifiers() const
{
	return (int)mClassifiers.size();
}

int MLLibCore::findClassifierIndex( Classifier* cl ) const
{
	for( int cli = 0; cli < getNumClassifiers(); ++cli )
	{
		if( mClassifiers[cli] == cl )
			return cli;
	}

	return -1;
}

}
