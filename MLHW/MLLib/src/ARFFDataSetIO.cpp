
#include "ARFFDataSetIO.h"

#include <sstream>

namespace MLLib
{

bool ARFFDataSetIO::read( const std::string& path )
{
	// Open file for reading
	mFileIn = std::ifstream( path.c_str() );
	if( mFileIn.bad() )
	{
		std::cout << "ERROR: Unable to open file " + path << std::endl;

		return false;
	}

	// Read the dataset
	if( !_readRelation() ||
		!_readData() )
	{
		mFileIn.close();

		return false;
	}

	// Make sure the dataset is valid
	if( mData->getNumAttributes() <= 0 )
	{
		std::cout << "ERROR: Data set in file " + path +
			" has no attributes and is invalid." << std::endl;

		return false;
	}
	if( mData->getAttribute( mData->getNumAttributes() - 1 ).attribType != AT_Enum )
	{
		std::cout << "ERROR: Data set in file " + path +
			" is invalid, because the last attribute (class) isn't nominal." << std::endl;

		return false;
	}

	mFileIn.close();

	return true;
}

bool ARFFDataSetIO::write( const std::string& path )
{
	// Open file for writing
	mFileOut = std::ofstream( path.c_str() );
	if( mFileOut.bad() )
	{
		std::cout << "ERROR: Unable to open file " + path << std::endl;

		return false;
	}

	// Write the dataset

	// Write relation
	mFileOut << "@RELATION MLLibData" << std::endl << std::endl;

	// Write attributes
	for( int attri = 0; attri < mData->getNumAttributes(); ++attri )
	{
		const DataSet::Attribute& attrib = mData->getAttribute(attri);

		if( attrib.attribType == AT_Int )
		{
			mFileOut << "@ATTRIBUTE " << attrib.attribName << " INTEGER" << std::endl;
		}
		else if( attrib.attribType == AT_Real )
		{
			mFileOut << "@ATTRIBUTE " << attrib.attribName << " REAL" << std::endl;
		}
		else // if( attrib.attribType == AT_Enum )
		{
			mFileOut << "@ATTRIBUTE " << attrib.attribName << " {";
			for( int evi = 0; evi < (int)attrib.enumValues.size(); ++evi )
			{
				if( evi < (int)attrib.enumValues.size() - 1 )
					mFileOut << attrib.enumValues[evi] << ",";
				else
					mFileOut << attrib.enumValues[evi] << "}" << std::endl;
			}
		}
	}
	mFileOut << std::endl;

	// Write data
	mFileOut << "@DATA" << std::endl;
	for( int dpti = 0; dpti < mData->getNumDataPoints(); ++dpti )
	{
		const DataPoint& dpt = mData->getDataPoint(dpti);
		
		for( int vi = 0; vi < dpt.getNumValues(); ++vi )
		{
			DataSet::Attribute attr = mData->getAttribute(vi);

			if( dpt.isValueUnknown(vi) )
			{
				mFileOut << "?";
			}
			else if( attr.attribType == AT_Int )
			{
				mFileOut << dpt.getIntValue(vi);
			}
			else if( attr.attribType == AT_Real )
			{
				mFileOut << dpt.getRealValue(vi);
			}
			else // if( attr.attribType == AT_Enum )
			{
				mFileOut << attr.enumValues[ dpt.getEnumValue(vi) ];
			}

			if( vi < dpt.getNumValues() - 1 )
				mFileOut << ",";
		}

		mFileOut << std::endl;
	}

	mFileOut.close();

	return true;
}

bool ARFFDataSetIO::_readRelation()
{
	if( _skipToNextStatement() != "@relation" )
	{
		return false;
	}

	std::string relname;
	mFileIn >> relname;
	
	return true;
}

bool ARFFDataSetIO::_readData()
{
	std::string kw;

	// Read attributes first
	std::vector<DataSet::Attribute> attribs;
	while( ( kw = _skipToNextStatement() ) == "@attribute" )
	{
		DataSet::Attribute attrib;

		if( !_readAttribute(attrib) )
		{
			return false;
		}

		attribs.push_back(attrib);
	}
	mData->initAttributes(attribs);

	if( kw != "@data" )
	{
		// No data defined!
		return false;
	}

	// Read data
	char line[4097];
	do
	{
		mFileIn.getline( &line[0], 4096 );

		if( line[0] == '\n' || line[0] == NULL )
		{
			continue;
		}
		else if( !_readDataPoint(line) )
		{
			return false;
		}
	}
	while( !mFileIn.eof() );

	return true;
}

bool ARFFDataSetIO::_readAttribute( DataSet::Attribute& attrib )
{
	std::string type_str, lctype_str;

	// Read attribute name
	mFileIn >> attrib.attribName;
	_skipWS();
	if( mFileIn.eof() )
	{
		return false;
	}

	// Read attribute type
	mFileIn >> type_str;
	_skipWS();
	lctype_str = type_str;
	std::transform( lctype_str.begin(), lctype_str.end(), lctype_str.begin(), ::tolower );
	if( lctype_str == "integer" )
	{
		attrib.attribType = AT_Int;
	}
	else if( lctype_str == "real" || lctype_str == "numeric" )
	{
		attrib.attribType = AT_Real;
	}
	else if( lctype_str.length() > 0 && lctype_str[0] == '{' )
	{
		// This is a nominal type, read all the possible values

		while( type_str[ type_str.length() - 1 ] != '}' &&
			!mFileIn.eof() && 
			mFileIn.peek() != '\n' )
			type_str += (char)mFileIn.get();

		if( type_str[ type_str.length() - 1 ] != '}' ||
			!_readEnumValues( type_str, attrib ) )
		{
			return false;
		}

		attrib.attribType = AT_Enum;
	}
	else
	{
		// Syntax error

		return false;
	}

	return true;
}

bool ARFFDataSetIO::_readEnumValues( const std::string& enumStr, DataSet::Attribute& attrib )
{
	std::string str = enumStr;

	// Replace special characters with whitespaces for easier parsing
	for( int ci = 0; ci < (int)str.length(); ++ci )
	{
		if( str[ci] == '{' || str[ci] == '}' || str[ci] == ',' )
			str[ci] = ' ';
	}

	std::istringstream iss(str);

	// Read nominal values
	std::string val;
	_skipWS();
	while( !iss.eof() )
	{
		iss >> val;
		if( val == "" )
			break;

		attrib.enumValues.push_back(val);

		val = "";
		_skipWS();
	}

	if( attrib.enumValues.size() <= 0 )
	{
		// No nominal values defined

		return false;
	}

	return true;
}

bool ARFFDataSetIO::_readDataPoint( const std::string dpStr )
{
	std::string str = dpStr;

	// Replace special characters with whitespaces for easier parsing
	// Replace special characters with whitespaces for easier parsing
	for( int ci = 0; ci < (int)str.length(); ++ci )
	{
		if( str[ci] == ',' )
			str[ci] = ' ';
	}

	std::istringstream iss(str);
	DataPoint dpt;

	// Read data values
	std::string val;
	int attrib_i = 0;
	_skipWS();
	while( !iss.eof() && attrib_i < mData->getNumAttributes() )
	{
		const DataSet::Attribute& attrib = mData->getAttribute(attrib_i);
		iss >> val;

		if( val == "?" )
		{
			// Unknown value
			dpt.addUnknownValue( attrib.attribType );
		}
		else if( attrib.attribType == AT_Int )
		{
			dpt.addIntValue( atoi(val.c_str()) );
		}
		else if( attrib.attribType == AT_Real )
		{
			dpt.addRealValue( atof(val.c_str()) );
		}
		else // if( attrib.attribType == AT_Enum )
		{
			int eval = attrib.getEnumAsInt(val);
			if( eval >= 0 )
				dpt.addEnumValue(eval);
			else
				dpt.addUnknownValue(AT_Enum);
		}

		++attrib_i;
	}

	// Add the new data point
	mData->addDataPoint(dpt);

	return true;
}

std::string ARFFDataSetIO::_skipToNextStatement()
{
	std::string str;

	while( !mFileIn.eof() )
	{
		_skipWS();

		if( (char)mFileIn.peek() == '\n' )
		{
			// Skip to next line
			(char)mFileIn.get();
		}
		else if( (char)mFileIn.peek() == '%' )
		{
			// Skip past comment
			while( (char)mFileIn.get() != '\n' )
			{
			}
		}
		else if( (char)mFileIn.peek() == '@' )
		{
			// Found next statement
			mFileIn >> str;
			_skipWS();
			std::transform( str.begin(), str.end(), str.begin(), ::tolower );

			if( mFileIn.eof() )
				return "";
			else
				return str;
		}
		else
		{
			// Invalid ARFF file
			break;
		}
	}

	return "";
}

void ARFFDataSetIO::_skipWS()
{
	char nc;
	while( ( nc = (char)mFileIn.peek() ) == ' ' ||
		nc == '\t' || nc == '\r' )
	{
		mFileIn.get();
	}
}

}
