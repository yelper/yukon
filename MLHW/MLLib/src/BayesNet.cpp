
#include "BayesNet.h"

namespace MLLib
{

BayesNet::Node::Node( int attribIndex, int numValues )
{
	mlAssert( numValues >= 2 );

	mAttribIndex = attribIndex;
	mNumValues = numValues;

	// Initialize CPT
	mParams.push_back( std::vector<double>(mNumValues) );
	mHyperParams.push_back( std::vector<double>( mNumValues, 1 ) );
}

BayesNet::Node::~Node()
{
}

int BayesNet::Node::getAttribIndex() const
{
	return mAttribIndex;
}

bool BayesNet::Node::addInNode( Node* node )
{
	// Check if node already present
	for( int ini = 0; ini < (int)mInNodes.size(); ++ini )
	{
		if( mInNodes[ini] == node )
			return true;
	}

	// Add node
	mInNodes.push_back(node);
	node->mOutNodes.push_back(this);

	std::set<Node*> vnodes;
	if( _detectCycle( this, vnodes ) )
	{
		// Cycle introduced, undo operation

		mInNodes.pop_back();
		node->mOutNodes.pop_back();
		return false;
	}

	// If this is the first node, clear CPT
	if( mInNodes.size() <= 0 )
	{
		mParams.clear();
		mHyperParams.clear();
	}

	// Update CPT
	mParams.clear();
	mHyperParams.clear();
	int num_rows = 1;
	for( int ini = 0; ini < (int)mInNodes.size(); ++ini )
		num_rows *= mInNodes[ini]->getNumValues();	
	for( int ri = 0; ri < num_rows; ++ri )
	{
		mParams.push_back( std::vector<double>(mNumValues) );
		mHyperParams.push_back( std::vector<double>( mNumValues, 1 ) );
	}

	return true;
}

void BayesNet::Node::removeInNode( int index )
{
	mlAssert( index >= 0 && index < (int)mInNodes.size() );

	Node* node = mInNodes[index];

	// Remove self from preceding node
	for( int oni0 = 0; oni0 < node->getNumOutNodes(); ++oni0 )
	{
		if( this == node->getOutNode(oni0) )
		{
			node->mOutNodes.erase( node->mOutNodes.begin() + oni0 );
			break;
		}
	}

	mInNodes.erase( mInNodes.begin() + index );

	// Update CPT
	mParams.clear();
	mHyperParams.clear();
	int num_rows = 1;
	for( int ini = 0; ini < (int)mInNodes.size(); ++ini )
		num_rows *= mInNodes[ini]->getNumValues();	
	for( int ri = 0; ri < num_rows; ++ri )
	{
		mParams.push_back( std::vector<double>(mNumValues) );
		mHyperParams.push_back( std::vector<double>( mNumValues, 1 ) );
	}
}

void BayesNet::Node::removeInNode( Node* node )
{
	mlAssert( node != NULL );

	for( int ni = 0; ni < getNumInNodes(); ++ni )
	{
		if( getInNode(ni) == node )
		{
			removeInNode(ni);
			break;
		}
	}

	return;
}

void BayesNet::Node::removeAllInNodes()
{
	while( getNumInNodes() > 0 )
		removeInNode(0);
}

BayesNet::Node* BayesNet::Node::getInNode( int index ) const
{
	mlAssert( index >= 0 && index < (int)mInNodes.size() );

	return mInNodes[index];
}

int BayesNet::Node::getNumInNodes() const
{
	return (int)mInNodes.size();
}

bool BayesNet::Node::hasInNode( Node* node ) const
{
	for( int ni = 0; ni < getNumInNodes(); ++ni )
		if( getInNode(ni) == node )
			return true;

	return false;
}

BayesNet::Node* BayesNet::Node::getOutNode( int index ) const
{
	mlAssert( index >= 0 && index < (int)mOutNodes.size() );

	return mOutNodes[index];
}

int BayesNet::Node::getNumOutNodes() const
{
	return (int)mOutNodes.size();
}

int BayesNet::Node::getNumValues() const
{
	return mNumValues;
}

double BayesNet::Node::getParam( int rowIndex, int valueIndex ) const
{
	mlAssert( rowIndex >= 0 && rowIndex < (int)mParams.size() );
	mlAssert( valueIndex >= 0 && valueIndex < mNumValues );

	return mParams[rowIndex][valueIndex];
}

int BayesNet::Node::getNumParams() const
{
	return (int)mParams.size();
}

void BayesNet::Node::incCount( int rowIndex, int valueIndex, double count )
{
	mlAssert( rowIndex >= 0 && rowIndex < (int)mParams.size() );
	mlAssert( valueIndex >= 0 && valueIndex < mNumValues );

	mHyperParams[rowIndex][valueIndex] += count;
}

double BayesNet::Node::getCount( int rowIndex, int valueIndex ) const
{
	mlAssert( rowIndex >= 0 && rowIndex < (int)mParams.size() );
	mlAssert( valueIndex >= 0 && valueIndex < mNumValues );

	return mHyperParams[rowIndex][valueIndex];
}

void BayesNet::Node::setCount( int rowIndex, int valueIndex, double count )
{
	mlAssert( rowIndex >= 0 && rowIndex < (int)mParams.size() );
	mlAssert( valueIndex >= 0 && valueIndex < mNumValues );
	mlAssert( count >= 0 );

	mHyperParams[rowIndex][valueIndex] = count;
}

void BayesNet::Node::initParams()
{
	// Compute CPT values
	for( int ri = 0; ri < (int)mParams.size(); ++ri )
	{
		std::vector<double>& params = mParams[ri];
		std::vector<double>& hparams = mHyperParams[ri];
		double sum = 0;

		// Compute sum of counts in this row
		for( int vi = 0; vi < mNumValues; ++vi )
		{
			sum += hparams[vi];
		}

		// Compute probabilities for each value in this row
		for( int vi = 0; vi < mNumValues; ++vi )
		{
			params[vi] = hparams[vi]/sum;
		}
	}
}

void BayesNet::Node::resetParams()
{
	// Reset all CPT entries
	for( int ri = 0; ri < (int)mParams.size(); ++ri )
	{
		std::vector<double>& params = mParams[ri];
		std::vector<double>& hparams = mHyperParams[ri];

		for( int vi = 0; vi < mNumValues; ++vi )
		{
			hparams[vi] = 1;
			params[vi] = 0;
		}
	}
}

int BayesNet::Node::getCPTRowIndex( const DataPoint& dpt ) const
{
	int ri = 0;
	int c = 1;

	// Compute row index in CPT given values of relevant attributes
	// (i.e. those in input nodes)
	for( int ni = 0; ni < getNumInNodes(); ++ni )
	{
		Node* node = getInNode(ni);
		ri += ( dpt.getEnumValue( node->getAttribIndex() ) * c );
		c *= node->getNumValues();
	}

	return ri;
}

bool BayesNet::Node::_detectCycle( Node* node, std::set<Node*> visitedNodes ) const
{
	if( visitedNodes.count(node) > 0 )
		return true;

	visitedNodes.insert(node);

	for( int oni = 0; oni < node->getNumOutNodes(); ++oni )
		if( _detectCycle( node->getOutNode(oni), visitedNodes ) )
			return true;

	return false;
}

BayesNet::BayesNet()
{
}

BayesNet::~BayesNet()
{
	deleteAllNodes();
}

BayesNet::Node* BayesNet::createNode( int attribIndex, int numValues )
{
	mlAssert( attribIndex >= 0 );
	mlAssert( numValues >= 2 );
	mlAssert( !hasNode(attribIndex) );

	Node* node = new Node( attribIndex, numValues );
	mNodes.insert( std::make_pair( attribIndex, node ) );

	return node;
}

void BayesNet::deleteAllNodes()
{
	for( int ni = 0; ni < getNumNodes(); ++ni )
		delete mNodes[ni];
}

bool BayesNet::hasNode( int attribIndex ) const
{
	return mNodes.count(attribIndex) > 0;
}

BayesNet::Node* BayesNet::getNode( int attribIndex ) const
{
	mlAssert( hasNode(attribIndex) );

	return mNodes.find(attribIndex)->second;
}

int BayesNet::getNumNodes() const
{
	return (int)mNodes.size();
}

double BayesNet::query( const DataPoint& dpt ) const
{
	for( int dvi = 0; dvi < dpt.getNumValues(); ++dvi )
		// Can classify only nominal values
		mlAssert( dpt.getValueType(dvi) == AT_Enum );

	int attrc = dpt.getNumValues() - 1;
	Node* qnode = getNode(attrc);
	DataPoint qdpt = dpt;
	std::vector<double> vlogp = std::vector<double>( qnode->getNumValues(), 0 );

	// Compute probability for every possible value
	double sump = 0;
	double maxlogp = -DBL_MAX;
	int maxv = 0;
	for( int vi = 0; vi < qnode->getNumValues(); ++vi )
	{
		qdpt.setEnumValue( attrc, vi );
		vlogp[vi] = queryLog(qdpt);
		sump += exp( vlogp[vi] );

		if( vlogp[vi] > maxlogp )
		{
			maxlogp = vlogp[vi];
			maxv = vi;
		}
	}

	// Compute scaled probability of this instance
	sump -= exp( vlogp[maxv] );
	double logp = vlogp[ dpt.getEnumValue(attrc) ] - maxlogp - log( 1 + sump/exp( vlogp[maxv] ) );
	double prob = exp(logp);

	return prob;
}

double BayesNet::queryLog( const DataPoint& dpt ) const
{	
	for( int dvi = 0; dvi < dpt.getNumValues(); ++dvi )
		// Can classify only nominal values
		mlAssert( dpt.getValueType(dvi) == AT_Enum );

	double logp = 0;

	for( std::map<int, Node*>::const_iterator ni = mNodes.begin();
			ni != mNodes.end(); ++ni )
	{
		int cai = ni->first;
		Node* cnode = ni->second;

		int row_i = cnode->getCPTRowIndex(dpt);
		double prob = cnode->getParam( row_i, dpt.getEnumValue(cai) );
		prob = IS_ZERO(prob) ? 0 : log(prob);
			
		logp += prob;
	}

	return logp;
}

double BayesNet::classify( DataPoint& dpt ) const
{
	int attrc = dpt.getNumValues() - 1;
	Node* qnode = getNode(attrc);

	// What is the most probable class?
	double max_prob = -DBL_MAX;
	int max_v = -1;
	for( int vi = 0; vi < qnode->getNumValues(); ++vi )
	{
		dpt.setEnumValue( attrc, vi );
		double prob = query(dpt);

		if( prob > max_prob )
		{
			max_prob = prob;
			max_v = vi;
		}
	}
	
	// Assign it
	dpt.setEnumValue( attrc, max_v );

	return max_prob;
}

}
