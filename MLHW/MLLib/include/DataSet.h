
#ifndef __DataSet_h__
#define __DataSet_h__

#include "Prereqs.h"
#include "DataPoint.h"

namespace MLLib
{

/**
* Class representing a dataset.
*/
class DataSet
{

public:

	/**
	* Data attribute (feature) descriptor.
	*/
	struct Attribute
	{

		std::string attribName; ///< Attribute name.
		AttribType attribType; ///< Attribute type.
		std::vector<std::string> enumValues; // Attribute value names (nominal attributes only).

		Attribute()
		{
		}

		Attribute( const std::string& attribName, AttribType attribType )
		{
			this->attribName = attribName;
			this->attribType = attribType;
		}

		Attribute( const std::string& attribName, AttribType attribType, const std::vector<std::string>& enumValues )
		{
			this->attribName = attribName;
			this->attribType = attribType;
			this->enumValues = enumValues;
		}

		int getEnumAsInt( const std::string& enumStr ) const
		{
			for( size_t vi = 0; vi < enumValues.size(); ++vi )
			{
				if( enumValues[vi] == enumStr )
					return vi;
			}

			return -1;
		}

	};

	/**
	* Constructor.
	*/
	DataSet();

	/**
	* Destructor.
	*/
	~DataSet();

	/**
	* Initializes the data attributes.
	*
	* param attribs Attributes specification.
	*/
	void initAttributes( const std::vector<Attribute>& attribs );

	/**
	* Gets the description of the specified attribute.
	*
	* @param index Attribute index.
	* @return Attribute description.
	*/
	const Attribute& getAttribute( int index ) const;

	/**
	* Sets the description of the specified attribute.
	*
	* @param index Attribute index.
	* @return Attribute description.
	*/
	void setAttribute( int index, const Attribute& attrib );

	/**
	* Gets the number of attributes in the dataset.
	*
	* @return Number of attributes.
	*/
	int getNumAttributes() const;

	/**
	* Finds the index of the specified attribute.
	*
	* @param attrib Attribute name.
	* @return Attribute index.
	*/
	int findAttributeIndex( const std::string& attrib ) const;

	/**
	* Adds a new data point to the dataset.
	*
	* @param dpt Data point.
	* @param prob Probability of said data point.
	*/
	void addDataPoint( const DataPoint& dpt, double prob = 0 );

	/**
	* Removes a data point from the dataset.
	*
	* @param index Data point index.
	*/
	void removeDataPoint( int index );

	/**
	* Removes all data points from the dataset.
	*/
	void removeAllDataPoints();

	/**
	* Gets a data point from the dataset.
	*
	* @param index Data point index.
	* @return Data point.
	*/
	const DataPoint& getDataPoint( int index ) const;

	/**
	* Sets a data point from the dataset.
	*
	* @param index Data point index.
	* @param dpt Data point.
	* @param prob Data point probability.
	*/
	void setDataPoint( int index, const DataPoint& dpt, double prob = 0 );

	/**
	* Gets the number of data point in the dataset.
	*
	* @return Number of data points.
	*/
	int getNumDataPoints() const;

	/**
	* Gets the probability of the specified data point.
	*
	* @param index Data point index.
	* @return Probability.
	*/
	double getDPProbability( int index ) const;

	/**
	* Sets the probability of the specified data point.
	*
	* @param index Data point index.
	* @param prob Probability.
	*/
	void setDPProbability( int index, double prob );

	/**
	* Creates a copy of this dataset.
	*
	* @return Dataset copy.
	*/
	DataSet* _clone() const;

private:

	std::vector<Attribute> mAttribs;
	std::vector<DataPoint> mData;
	std::vector<double> mProbs;

};

}

#endif // __DataSet_h__
