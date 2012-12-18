
#include "DataPoint.h"

namespace MLLib
{

DataPoint::DataPoint()
{
}

DataPoint::~DataPoint()
{
}

void DataPoint::addIntValue( int val )
{
	mVals.push_back( ValueMapping( AT_Int, (int)mIntVals.size() ) );
	mIntVals.push_back(val);
}

void DataPoint::addRealValue( double val )
{
	mVals.push_back( ValueMapping( AT_Real, (int)mRealVals.size() ) );
	mRealVals.push_back(val);
}

void DataPoint::addEnumValue( int val )
{
	mVals.push_back( ValueMapping( AT_Enum, (int)mEnumVals.size() ) );
	mEnumVals.push_back(val);
}

void DataPoint::addUnknownValue( AttribType valType )
{
	mVals.push_back( ValueMapping( valType, -1 ) );
}

void DataPoint::removeValue( int index )
{
	mlAssert( index >= 0 && index < getNumValues() );

	ValueMapping val = mVals[index];
	mVals.erase( mVals.begin() + index );
	
	if( val.valueIndex < 0 )
		return;

	if( val.valueType == AT_Int )
	{
		mIntVals.erase( mIntVals.begin() + val.valueIndex );
	}
	else if( val.valueType == AT_Real )
	{
		mRealVals.erase( mRealVals.begin() + val.valueIndex );
	}
	else // if( val.valueType == AT_Enum )
	{
		mEnumVals.erase( mEnumVals.begin() + val.valueIndex );
	}
}

void DataPoint::removeAllValues()
{
	mVals.clear();
	mIntVals.clear();
	mEnumVals.clear();
}

AttribType DataPoint::getValueType( int index ) const
{
	mlAssert( index >= 0 && index < getNumValues() );

	return mVals[index].valueType;
}

bool DataPoint::isValueUnknown( int index ) const
{
	mlAssert( index >= 0 && index < getNumValues() );

	return mVals[index].valueIndex < 0;
}

int DataPoint::getIntValue( int index ) const
{
	mlAssert( index >= 0 && index < getNumValues() );
	mlAssert( mVals[index].valueType == AT_Int );

	return mIntVals[ mVals[index].valueIndex ];
}

void DataPoint::setIntValue( int index, int val )
{
	mlAssert( index >= 0 && index < getNumValues() );
	mlAssert( mVals[index].valueType == AT_Int );

	mIntVals[ mVals[index].valueIndex ] = val;
}

double DataPoint::getRealValue( int index ) const
{
	mlAssert( index >= 0 && index < getNumValues() );
	mlAssert( mVals[index].valueType == AT_Real );

	return mRealVals[ mVals[index].valueIndex ];
}

void DataPoint::setRealValue( int index, double val )
{
	mlAssert( index >= 0 && index < getNumValues() );
	mlAssert( mVals[index].valueType == AT_Real );

	mRealVals[ mVals[index].valueIndex ] = val;
}

int DataPoint::getEnumValue( int index ) const
{
	//modifying
	mlAssert( index >= 0 && index < getNumValues() );
	mlAssert( mVals[index].valueType == AT_Enum );

	return mEnumVals[ mVals[index].valueIndex ];
}

void DataPoint::setEnumValue( int index, int val )
{
	mlAssert( index >= 0 && index < getNumValues() );
	mlAssert( mVals[index].valueType == AT_Enum );

	mEnumVals[ mVals[index].valueIndex ] = val;
}

int DataPoint::getNumValues() const
{
	return mVals.size();
}

}
