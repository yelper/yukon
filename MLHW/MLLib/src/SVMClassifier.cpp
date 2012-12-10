
#include "SVMClassifier.h"

namespace MLLib
{

SVMClassifier::SVMClassifier()
{
	mC = 0.1;
	mD = 1;
	mS = 1;

	INIT_RAND();
}

SVMClassifier::~SVMClassifier()
{
}

void SVMClassifier::train( DataSet* data )
{
	mlAssert( data != NULL );

	reset();

	// Initialize the SVM parameters
	int nx = data->getNumDataPoints();
	mA = std::vector<double>( nx, 0 );
	mX = std::vector<DataPoint>(nx);
	mY = std::vector<double>( nx, 0 );
	for( int xi = 0; xi < nx; ++xi )
	{
		mX[xi] = data->getDataPoint(xi);
		int nvi = mX[xi].getNumValues();

		if( mX[xi].getValueType(nvi-1) == AT_Enum )
		{
			mY[xi] = mX[xi].getEnumValue(nvi-1) == 1 ? 1 : -1;
		}
		else if( mX[xi].getValueType(nvi-1) == AT_Int )
		{
			mY[xi] = mX[xi].getIntValue(nvi-1);
		}
		else // if( mX[xi].getValueType(nvi-1) == AT_Real )
		{
			mY[xi] = mX[xi].getRealValue(nvi-1);
		}
	}
	_cacheKernels();
	mUXIter = mUXCache.end();

	// Train the SVM
	int num_changed = 0;
	bool test_all = true;
	while( num_changed > 0 || test_all )
	{
		num_changed = 0;

		if(test_all)
		{
			// Process all examples
			
			for( int xi = 0; xi < (int)mX.size(); ++xi )
			{
				if( _processExample(xi) )
					++num_changed;
			}
		}
		else
		{
			// Process only unbounded examples
			for( mUXIter = mUXCache.begin(); mUXIter != mUXCache.end(); )
			{
				int uxi = mUXIter->first;
				if( _processExample(uxi) )
					++num_changed;

				if( mUXIter != mUXCache.end() && uxi == mUXIter->first )
					// None of the inner loops tampered with the cache, advance as normal
					++mUXIter;
			}

			mUXIter = mUXCache.end(); // to signify that we're no longer iterating over unbounded examples
		}

		if(test_all)
			test_all = false;
		else if( num_changed == 0 )
			test_all = true;
	}

	// Print out training results
	std::cout << "b = " << mB << std::endl;
	std::cout << "alpha = [ ";
	for( int xi = 0; xi < nx; ++xi )
	{
		std::cout << mA[xi] << ( xi < (nx-1) ? " " : "" );
	}
	std::cout << " ]" << std::endl;
}

void SVMClassifier::reset()
{
	mX.clear();
	mA.clear();
	mY.clear();
	mB = 0;
}

void SVMClassifier::classify( DataSet* data )
{
	mlAssert( data != NULL );

	for( int dpti = 0; dpti < data->getNumDataPoints(); ++dpti )
	{
		DataPoint dpt = data->getDataPoint(dpti);
		double u = _query(dpt);
		int c = u >= 0 ? 1 : 0;

		if( dpt.getValueType( dpt.getNumValues() - 1 ) == AT_Enum )
			// Class attribute is nominal
			dpt.setEnumValue( dpt.getNumValues() - 1, c );
		else if( dpt.getValueType( dpt.getNumValues() - 1 ) == AT_Int )
			// Otherwise it should be integer
			dpt.setIntValue( dpt.getNumValues() - 1, c == 1 ? 1 : -1 );
		else // if( dpt.getValueType( dpt.getNumValues() - 1 ) == AT_Real )
			// This should never happen
			dpt.setRealValue( dpt.getNumValues() - 1, c == 1 ? 1 : -1 );

		data->setDataPoint( dpti, dpt );
	}
}

bool SVMClassifier::_processExample( int xi2 )
{
	double y2 = mY[xi2];
	double a2 = mA[xi2];
	double err2 = _error(xi2);
	double r2 = err2*y2;

	if( r2 < -KKT_EPSILON && a2 < mC || r2 > KKT_EPSILON && a2 > 0 )
	{
		// Our example 2 violates KKT, now choose example 1

		// Find example that maximizes step size
		double max_de = -DBL_MIN;
		int xi1 = -1;
		std::vector<int> ux_list( (int)mUXCache.size() );
		int uxli = 0;
		for( std::map<int,double>::iterator uxi = mUXCache.begin();
			uxi != mUXCache.end(); ++uxi )
		{
			double de = fabs( uxi->second - err2 );
			if( de > KKT_EPSILON && de > max_de )
			{
				xi1 = uxi->first;
				max_de = de;
			}

			ux_list[uxli++] = uxi->first;
		}

		// Try to improve the SVM
		if( xi1 > -1 && _advanceSMO(xi1,xi2,err2) )
		{
			return true;
		}

		// Failed to make any progress, try any unbounded example 1
		int nux = (int)ux_list.size();
		int xioff = nux > 1 ? RAND(0,(nux-1)) : 0;
		for( int uxi = 0; uxi < nux; ++uxi )
		{
			xi1 = ux_list[ (uxi+xioff)%nux ];
			
			// Try to improve the SVM
			if( _advanceSMO(xi1,xi2,err2) )
			{
				return true;
			}
		}

		// Failed to make any progress, try any example 1
		int nx = (int)mX.size();
		for( int xi = 0; xi < nx; ++xi )
		{
			xi1 = (xi+xioff)%nx;
			
			// Try to improve the SVM
			if( _advanceSMO(xi1,xi2,err2) )
			{
				return true;
			}
		}
	}

	return false;
}

bool SVMClassifier::_advanceSMO( int xi1, int xi2, double err2 )
{
	if( xi1 == xi2 )
		return false;

	double y2 = mY[xi2];
	double a2 = mA[xi2];
	double a1 = mA[xi1];
	double y1 = mY[xi1];
	double err1 = _error(xi1);
	double s = y1*y2;

	double L = 0, H = 0;
	// Compute bounds for clipping a2
	if( y1 > 0 && y2 > 0 ||
		y1 < 0 && y2 < 0 ) // are y1 and y2 equal?
	{
		L = std::max<double>( 0, a2 + a1 - mC );
		H = std::min<double>( mC, a2 + a1 );
	}
	else
	{
		L = std::max<double>( 0, a2 - a1 );
		H = std::min<double>( mC, mC + a2 - a1 );
	}

	if( IS_ZEROE( (L-H), KKT_EPSILON ) )
	{
		// There is no room to make progress
		return false;
	}

	// Precompute kernel values
	double k11 = _kernel(xi1,xi1);
	double k12 = _kernel(xi1,xi2);
	double k22 = _kernel(xi2,xi2);

	double eta = k11 + k22 - 2.*k12;
	double a2n = 0;
	if( eta > KKT_EPSILON )
	{
		// Compute and clip new a2
		a2n = a2 + y2*(err1-err2)/eta;
		if( a2n > H )
			a2n = H;
		else if( a2n < L )
			a2n = L;
	}
	else
	{
		// ex1 and ex2 have same input vector, or invalid kernel, can't make progress
		return false;
	}

	if( fabs(a2n-a2) < KKT_EPSILON*( a2n + a2 + KKT_EPSILON ) )
	{
		// No progress made
		return false;
	}

	// Compute a1
	double a1n = a1 + s*(a2-a2n);

	// Update b-param. (threshold)
	double b1 = err1 + y1*(a1n-a1)*k11 + y2*(a2n-a2)*k12 + mB;
	double b2 = err2 + y1*(a1n-a1)*k12 + y2*(a2n-a2)*k22 + mB;
	mB = (b1+b2)/2.;

	// Update multipliers
	mA[xi1] = a1n;
	mA[xi2] = a2n;

	_updateErrorCache();

	return true;
}

double SVMClassifier::_query( int xi ) const
{
	double u = 0;

	for( int xj = 0; xj < (int)mA.size(); ++xj )
		u += ( mA[xj] * mY[xj] * _kernel( xj, xi ) );
	u -= mB;

	return u;
}

double SVMClassifier::_query( const DataPoint& dpt ) const
{
	double u = 0;

	for( int xi = 0; xi < (int)mA.size(); ++xi )
		u += ( mA[xi] * mY[xi] * _kernel( mX[xi], dpt ) );
	u -= mB;

	return u;
}

double SVMClassifier::_error( int xi )
{
	double err = 0;
	
	if( mUXCache.count(xi) > 0 )
	{
		err = mUXCache[xi];
	}
	else
	{
		err = _query(xi) - mY[xi];

		if( _isBound(xi) )
			mUXCache[xi] = err;
	}

	return err;
}

void SVMClassifier::_updateErrorCache()
{
	for( std::map<int,double>::iterator uxi = mUXCache.begin();
		uxi != mUXCache.end(); )
	{
		std::map<int,double>::iterator cur_uxi = uxi;
		++uxi;

		if( _isBound(cur_uxi->first) )
		{
			// Example has become bound, uncache it
			if( mUXIter == cur_uxi )
				// Update outer loop iterator if necessary
				++mUXIter;
			mUXCache.erase(cur_uxi);
		}
		else
		{
			mUXCache[cur_uxi->first] = _query(cur_uxi->first) - mY[cur_uxi->first];
		}
	}
}

double SVMClassifier::_dot( const DataPoint& dpt1, const DataPoint& dpt2 ) const
{
	mlAssert( dpt1.getNumValues() == dpt2.getNumValues() );

	double v = 0;

	for( int vi = 0; vi < dpt1.getNumValues() - 1; ++vi )
	{
		double v1 = 0, v2 = 0;

		if( dpt1.getValueType(vi) == AT_Enum )
		{
			v1 = dpt1.getEnumValue(vi) == 1 ? 1 : -1;
			v2 = dpt2.getEnumValue(vi) == 1 ? 1 : -1;
		}
		else if( dpt1.getValueType(vi) == AT_Int )
		{
			v1 = dpt1.getIntValue(vi);
			v2 = dpt2.getIntValue(vi);
		}
		else // if( dpt1.getValueType(vi) == AT_Real )
		{
			v1 = dpt1.getRealValue(vi);
			v2 = dpt2.getRealValue(vi);
		}

		v += v1*v2;
	}

	return v;
}

double SVMClassifier::_subNormSq( const DataPoint& dpt1, const DataPoint& dpt2 ) const
{
	mlAssert( dpt1.getNumValues() == dpt2.getNumValues() );

	double v = 0;

	for( int vi = 0; vi < dpt1.getNumValues() - 1; ++vi )
	{
		double v1 = 0, v2 = 0;

		if( dpt1.getValueType(vi) == AT_Enum )
		{
			v1 = dpt1.getEnumValue(vi) == 1 ? 1 : -1;
			v2 = dpt2.getEnumValue(vi) == 1 ? 1 : -1;
		}
		else if( dpt1.getValueType(vi) == AT_Int )
		{
			v1 = dpt1.getIntValue(vi);
			v2 = dpt2.getIntValue(vi);
		}
		else // if( dpt1.getValueType(vi) == AT_Real )
		{
			v1 = dpt1.getRealValue(vi);
			v2 = dpt2.getRealValue(vi);
		}

		double diff = v1 - v2;
		v += diff*diff;
	}

	return v;
}

double SVMClassifier::_kernel( int xi1, int xi2 ) const
{
	int nx = (int)mX.size();

	mlAssert( (int)mKernelCache.size() == nx*nx );

	return mKernelCache[ xi1*(int)mX.size() + xi2 ];
}

double SVMClassifier::_kernel( const DataPoint& dpt1, const DataPoint& dpt2 ) const
{
	switch(mKernelType)
	{
	case KT_Linear:

		return _dot(dpt1,dpt2);

	case KT_Poly:

		return _kPoly( dpt1, dpt2, mD );
		
	case KT_Gauss:

		return _kGauss( dpt1, dpt2, mS );
	}

	return 0;
}

double SVMClassifier::_kPoly( const DataPoint& dpt1, const DataPoint& dpt2, int d ) const
{
	double v = _dot(dpt1,dpt2) + 1;

	return pow( v, d );
}

double SVMClassifier::_kGauss( const DataPoint& dpt1, const DataPoint& dpt2, double s ) const
{
	double v = _subNormSq(dpt1,dpt2);

	return exp(-mS*v);
}

bool SVMClassifier::_isBound( int xi ) const
{
	return mA[xi] > KKT_EPSILON && mA[xi] < mC - KKT_EPSILON;
}

void SVMClassifier::_cacheKernels()
{
	int nx = (int)mX.size();
	mKernelCache = std::vector<double>( nx*nx, 0 );

	for( int xi1 = 0; xi1 < nx; ++xi1 )
	{
		for( int xi2 = xi1; xi2 < nx; ++xi2 )
		{
			mKernelCache[xi1*nx+xi2] = _kernel( mX[xi1], mX[xi2] );
			mKernelCache[xi2*nx+xi1] = mKernelCache[xi1*nx+xi2];
		}
	}
}

}
