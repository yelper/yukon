
#ifndef __DataPoint_h__
#define __DataPoint_h__

#include "Prereqs.h"

namespace MLLib
{

/**
* Data attribute (feature) type.
*/
enum AttribType
{
	AT_Int, ///< Integer-value attribute.
	AT_Real, ///< Real-value attribute.
	AT_Enum ///< Nominal attribute.
};

/**
* Class representing a data point/feature vector.
*/
class DataPoint
{

public:

	/**
	* Constructor.
	*/
	DataPoint();

	/**
	* Destructor.
	*/
	~DataPoint();

	/**
	* Adds an integer value to the data point.
	*
	* @param val Integer value.
	*/
	void addIntValue( int val );

	/**
	* Adds a real value to the data point.
	*
	* @param val Real value.
	*/
	void addRealValue( double val );

	/**
	* Adds a nominal value to the data point.
	*
	* @param val Nominal value.
	*/
	void addEnumValue( int val );

	/**
	* Adds an unknown value to the data point.
	*
	* @param valType Attribute type.
	*/
	void addUnknownValue( AttribType valType );

	/**
	* Removes a value from the data point.
	*
	* @param index Attribute index.
	*/
	void removeValue( int index );

	/**
	* Removes all values from the data point.
	*/
	void removeAllValues();

	/**
	* Gets the type of the specified attribute
	* (this is just a shorthand for DataSet::getAttribute(index).attribType).
	*
	* @param index Attribute index.
	* @return Attribute type.
	*/
	AttribType getValueType( int index ) const;

	/**
	* Checks if the specified attribute has an unknown value in
	* this data point.
	*
	* @param index Attribute index.
	* @return true if the value is unknown, false otherwise.
	*/
	bool isValueUnknown( int index ) const;

	/**
	* Gets the integer value at the specified attribute
	* in this data point.
	*
	* @param index Attribute index.
	* @return Integer value.
	*/
	int getIntValue( int index ) const;

	/**
	* Sets the integer value at the specified attribute
	* in this data point.
	*
	* @param index Attribute index.
	* @param val Integer value.
	*/
	void setIntValue( int index, int val );

	/**
	* Gets the real value at the specified attribute
	* in this data point.
	*
	* @param index Attribute index.
	* @return Real value.
	*/
	double getRealValue( int index ) const;

	/**
	* Sets the real value at the specified attribute
	* in this data point.
	*
	* @param index Attribute index.
	* @param val Real value.
	*/
	void setRealValue( int index, double val );

	/**
	* Gets the nominal value at the specified attribute
	* in this data point.
	*
	* @param index Attribute index.
	* @return Nominal value.
	*/
	int getEnumValue( int index ) const;

	/**
	* Sets the nominal value at the specified attribute
	* in this data point.
	*
	* @param index Attribute index.
	* @param val Nominal value.
	*/
	void setEnumValue( int index, int val );

	/**
	* Gets the number of attribute values in this data point.

	* @return Number of values.
	*/
	int getNumValues() const;

private:

	struct ValueMapping
	{

		AttribType valueType;
		int valueIndex;

		ValueMapping( AttribType vt, int vi )
		{
			valueType = vt;
			valueIndex = vi;
		}

	};

	std::vector<ValueMapping> mVals;
	std::vector<int> mIntVals;
	std::vector<double> mRealVals;
	std::vector<int> mEnumVals;

};

}

#endif // __DataPoint_h__
