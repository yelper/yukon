
#include "MLLib.h"

using namespace MLLib;

int main( int argc, char** argv )
{
	if( argc != 6 )
	{
		std::cout << "Usage: MLSVMTest <1|2> <C> <PolyDegree|GaussSigma> <TrainingDataSetFile> <TestDataSetFile>" << std::endl;
		return 0;
	}

	KernelType kt;
	double C = 0;
	int degree = 0;
	double sigma = 0;

	// Create classifier
	SVMClassifier* svm = static_cast<SVMClassifier*>( MLLIB->createClassifier(CT_SVM) );

	// Get SVM parameters
	kt = atoi(argv[1]) == 1 ? KT_Poly : KT_Gauss;
	svm->setKernelType(kt);
	C = atof(argv[2]);
	svm->setC(C);
	if( kt == KT_Poly )
	{
		degree = atoi(argv[3]);
		svm->setD(degree);
	}
	else // if( kt == KT_Gauss )
	{
		sigma = atof(argv[3]);
		svm->setS(sigma);
	}

	DataSet* data, *test_data;
	data = MLLIB->createDataSet();
	test_data = MLLIB->createDataSet();
	
	// Load the training dataset
	std::string dspath = argv[4];
	if( !MLLIB->loadDataSet( dspath + ".arff", data ) )
	{
		return 0;
	}

	// Load the test dataset
	dspath = argv[5];
	if( !MLLIB->loadDataSet( dspath + ".arff", test_data ) )
	{
		return 0;
	}

	// Train and test the SVM
	svm->train(data);
	svm->test(test_data);
	svm->reset();

	return 0;
}
