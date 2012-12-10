
#include "MLLib.h"

using namespace MLLib;

int main( int argc, char** argv )
{
	if( argc != 3 )
	{
		std::cout << "Usage: MLBayesTest <TrainingDataSetFile> <TestDataSetFile>" << std::endl;
		return 0;
	}

	DataSet* data, *test_data;
	data = MLLIB->createDataSet();
	test_data = MLLIB->createDataSet();
	
	// Load the training dataset
	std::string dspath = argv[1];
	if( !MLLIB->loadDataSet( dspath + ".arff", data ) )
	{
		return 0;
	}

	// Load the test dataset
	dspath = argv[2];
	if( !MLLIB->loadDataSet( dspath + ".arff", test_data ) )
	{
		return 0;
	}

	// Create each classifier, train it and test it
	BayesClassifier* bcl = static_cast<BayesClassifier*>( MLLIB->createClassifier(CT_Bayes) );
	bcl->setBayesType(BT_Naive);
	bcl->train(data);
	bcl->test(test_data);
	bcl->reset();
	bcl->setBayesType(BT_TAN);
	bcl->train(data);
	bcl->test(test_data);
	bcl->reset();
	bcl->setBayesType(BT_SparseCandidate);
	bcl->train(data);
	bcl->test(test_data);
	bcl->reset();

	return 0;
}
