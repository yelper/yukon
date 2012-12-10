
#ifndef __DataSetIO_h__
#define __DataSetIO_h__

#include "Prereqs.h"
#include "DataSet.h"

namespace MLLib
{

/**
* Base class for dataset readers/writers.
*/
class DataSetIO
{

public:

	/**
	* Constructor.
	*/
	DataSetIO( DataSet* data ) : mData(data) { mlAssert(data); }

	/**
	* Destructor.
	*/
	virtual ~DataSetIO() { }

	/**
	* Reads the dataset from a file.
	*
	* @param path Dataset file path.
	* @return true if the dataset was read in successfully, false otherwise.
	*/
	virtual bool read( const std::string& path ) = 0;

	/**
	* Writes the dataset to a file.
	*
	* @param path Dataset file path.
	* @return true if the dataset was written out successfully, false otherwise.
	*/
	virtual bool write( const std::string& path ) = 0;

protected:

	DataSet* mData;

};

}

#endif // __DataSetIO_h__
