
#include "DataSet.h"

namespace MLLib
{

DataSet::DataSet()
{
}

DataSet::~DataSet()
{
}

void DataSet::initAttributes( const std::vector<Attribute>& attribs )
{
	mAttribs = attribs;
}

const DataSet::Attribute& DataSet::getAttribute( int index ) const
{
	mlAssert( index >= 0 && index < getNumAttributes() );

	return mAttribs[index];
}

void DataSet::setAttribute( int index, const Attribute& attrib )
{
	mlAssert( index >= 0 && index < getNumAttributes() );

	mAttribs[index] = attrib;
}

int DataSet::getNumAttributes() const
{
	return (int)mAttribs.size();
}

int DataSet::findAttributeIndex( const std::string& attrib ) const
{
	for( int ai = 0; ai < getNumAttributes(); ++ai )
	{
		if( mAttribs[ai].attribName == attrib )
			return ai;
	}

	return -1;
}

void DataSet::addDataPoint( const DataPoint& dpt, double prob )
{
	// TODO: verify that attributes are correct in number and type

	mData.push_back(dpt);
	mProbs.push_back(prob);
}

void DataSet::removeDataPoint( int index )
{
	mlAssert( index >= 0 && index < getNumDataPoints() );

	mData.erase( mData.begin() + index );
	mProbs.erase( mProbs.begin() + index );
}

void DataSet::removeAllDataPoints()
{
	mData.clear();
	mProbs.clear();
}

const DataPoint& DataSet::getDataPoint( int index ) const
{
	mlAssert( index >= 0 && index < getNumDataPoints() );

	return mData[index];
}

void DataSet::setDataPoint( int index, const DataPoint& dpt, double prob )
{
	mlAssert( index >= 0 && index < getNumDataPoints() );

	mData[index] = dpt;
	mProbs[index] = prob;
}

int DataSet::getNumDataPoints() const
{
	return (int)mData.size();
}

double DataSet::getDPProbability( int index ) const
{
	mlAssert( index >= 0 && index < getNumDataPoints() );

	return mProbs[index];
}

void DataSet::setDPProbability( int index, double prob )
{
	mlAssert( index >= 0 && index < getNumDataPoints() );

	mProbs[index] = prob;
}

DataSet* DataSet::_clone() const
{
	DataSet* ds = new DataSet();
	ds->mAttribs = mAttribs;
	ds->mData = mData;
	ds->mProbs = mProbs;

	return ds;
}

}
