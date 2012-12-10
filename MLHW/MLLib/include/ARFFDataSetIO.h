
#ifndef __ARFFDataSetIO_h__
#define __ARFFDataSetIO_h__

#include "Prereqs.h"
#include "DataSetIO.h"

#include <fstream>

namespace MLLib
{

class ARFFDataSetIO : public DataSetIO
{

public:

	ARFFDataSetIO( DataSet* data ) : DataSetIO(data) { }
	~ARFFDataSetIO() { }
	bool read( const std::string& path );
	bool write( const std::string& path );

private:

	bool _readRelation();
	bool _readData();
	bool _readAttribute( DataSet::Attribute& attrib );
	bool _readEnumValues( const std::string& enumStr, DataSet::Attribute& attrib );
	bool _readDataPoint( const std::string dpStr );
	std::string _skipToNextStatement();
	void _skipWS();

	std::ifstream mFileIn;
	std::ofstream mFileOut;

};

}

#endif // __ARFFDataSetIO_h__
