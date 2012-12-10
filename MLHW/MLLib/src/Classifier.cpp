
#include "Classifier.h"
#include "MLLib.h"

namespace MLLib
{

void Classifier::test( DataSet* data )
{
	// Save copy of original data
	DataSet* test_data = MLLIB->cloneDataSet(data);
	// Classify the test data
	classify(test_data);
	
	// Compute test results
	int tp = 0, fp = 0, tn = 0, fn = 0;
	for( int dpti = 0; dpti < test_data->getNumDataPoints(); ++dpti )
	{
		const DataPoint& edpt = test_data->getDataPoint(dpti);
		const DataPoint& tdpt = data->getDataPoint(dpti);

		int edv = edpt.getEnumValue( edpt.getNumValues() - 1 ),
			tdv = tdpt.getEnumValue( tdpt.getNumValues() - 1 );
		if( tdv == 1 && edv == 1 )
		{
			++tp;
		}
		else if( tdv == 1 && edv != 1 )
		{
			++fn;
		}
		else if( tdv != 1 && edv == 1 )
		{
			++fp;
		}
		else // if( tdv != 1 && edv != 1 )
		{
			++tn;
		}

		// Print out class and probability for each instance
		for( int attri = 0; attri < tdpt.getNumValues(); ++attri )
		{
			if( tdpt.getValueType(attri) == AT_Enum )
				std::cout << test_data->getAttribute(attri).enumValues[ tdpt.getEnumValue(attri) ] << ",";
			else if( tdpt.getValueType(attri) == AT_Int )
				std::cout << tdpt.getIntValue(attri) << ",";
			else if( tdpt.getValueType(attri) == AT_Real )
				std::cout << tdpt.getRealValue(attri) << ",";
		}
		std::cout << test_data->getDPProbability(dpti) << std::endl;
	}
	double acc = ((double)( tp + tn ))/( tp + tn + fp + fn );

	// Dispose of the test dataset
	MLLIB->deleteDataSet( MLLIB->findDataSetIndex(test_data) );

	// Print out test results
	std::cout << std::endl;
	std::cout << "Classifier accuracy = " << acc << std::endl;
	std::cout << "Contingency table = " << std::endl;
	std::cout << "TRUE+\tTRUE-" << std::endl;
	std::cout << tp << "\t" << fp << "\tEST+" << std::endl;
	std::cout << fn << "\t" << tn << "\tEST-" << std::endl;
	std::cout << std::endl;
}

}
