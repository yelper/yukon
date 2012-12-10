
#include "BayesClassifier.h"
#include "MLLib.h"

#include <queue>

namespace MLLib
{

BayesClassifier::BayesClassifier()
{
	mNet = NULL;
}

BayesClassifier::~BayesClassifier()
{
	reset();
}

BayesNet* BayesClassifier::getBayesNet() const
{
	return mNet;
}

void BayesClassifier::train( DataSet* data )
{
	mlAssert( data != NULL );

	// Reset the network
	reset();
	mNet = new BayesNet();

	// Train the network
	if( mBType == BT_Naive )
		_trainNaiveBayes(data);
	else if( mBType == BT_TAN )
		_trainTAN(data);
	else // if( mBType == BT_SparseCandidate )
		_trainSparseCandidate(data);

	return;
}

void BayesClassifier::reset()
{
	if( mNet != NULL )
	{
		delete mNet;
		mNet = NULL;
	}
}

void BayesClassifier::classify( DataSet* data )
{
	mlAssert( mNet != NULL );
	mlAssert( data != NULL );

	for( int dpti = 0; dpti < data->getNumDataPoints(); ++dpti )
	{
		DataPoint dpt = data->getDataPoint(dpti);
		double p = mNet->classify(dpt);
		data->setDataPoint( dpti, dpt, p );
	}
}

void BayesClassifier::_trainNaiveBayes( DataSet* data )
{
	_createNaiveBayesStructure(data);
	_learnNetParams(data);
}

void BayesClassifier::_trainTAN( DataSet* data )
{
	_createNaiveBayesStructure(data);

	std::map<Arc,double> warcs;
	std::set<Arc> cdarcs;
	double maxw = -DBL_MAX;
	Arc maxw_arc;

	// Create and weigh arcs between all nodes
	BayesNet::Node *node1 = NULL, *node2 = NULL;
	for( int ni1 = 0; ni1 < mNet->getNumNodes(); ++ni1 )
	{
		for( int ni2 = ni1 + 1; ni2 < mNet->getNumNodes(); ++ni2 )
		{
			node1 = mNet->getNode(ni1);
			node2 = mNet->getNode(ni2);

			if( node1->getAttribIndex() == data->getNumAttributes() - 1 ||
				node2->getAttribIndex() == data->getNumAttributes() - 1 )
				// Don't include the class attribute
				continue;

			double weight = _computeMutualInf( data, node1->getAttribIndex(), node2->getAttribIndex() );
			warcs.insert( std::make_pair( Arc( node1, node2 ), weight ) );
			cdarcs.insert( Arc( node1, node2 ) );

			if( weight > maxw )
			{
				maxw = weight;
				maxw_arc = Arc( node1, node2 );
			}
		}
	}

	// Compute maximum weight spanning tree
	std::set<Arc> mwst_arcs;
	std::set<BayesNet::Node*> mwst_nodes;
	// Initialize tree
	mwst_arcs.insert(maxw_arc);
	mwst_nodes.insert( maxw_arc.getNode1() );
	mwst_nodes.insert( maxw_arc.getNode2() );
	cdarcs.erase(maxw_arc);
	// Compute tree
	do
	{
		// Sort eligible arcs (those branching off of the existing tree)
		std::set<WeightedArc> new_twarcs;
		for( std::set<Arc>::iterator arci = cdarcs.begin();
			arci != cdarcs.end(); ++arci )
		{
			Arc arc = *arci;

			if( mwst_nodes.count( arc.getNode1() ) > 0 &&
				mwst_nodes.count( arc.getNode2() ) == 0 ||
				mwst_nodes.count( arc.getNode1() ) == 0 &&
				mwst_nodes.count( arc.getNode2() ) > 0 )
			{
				new_twarcs.insert( WeightedArc( arc, warcs[arc] ) );
			}
		}

		if( new_twarcs.size() <= 0 )
			// No more arcs to add
			break;

		// Add the arc with highest weight into the tree
		Arc new_arc = new_twarcs.begin()->arc;
		mwst_arcs.insert(new_arc);
		mwst_nodes.insert( new_arc.getNode1() );
		mwst_nodes.insert( new_arc.getNode2() );
		cdarcs.erase(new_arc);
	}
	while(true);

	// Connect nodes into the tree
	_buildNodeTree( *mwst_nodes.begin(), mwst_arcs, mwst_nodes );

	_learnNetParams(data);
}

void BayesClassifier::_trainSparseCandidate( DataSet* data )
{
	// Create arcless Bayes net
	BayesNet::Node* node = NULL;
	for( int attr_i = 0; attr_i < data->getNumAttributes(); ++attr_i )
	{
		node = mNet->createNode( attr_i,
			(int)data->getAttribute(attr_i).enumValues.size() );
	}
	_learnNetParams(data);
	
	// Compute initial BIC score
	_BICInit(data);

	std::map<BayesNet::Node*, std::set<BayesNet::Node*>> cands; // candidate parents
	bool bn_changed = false;

	do
	{
		bn_changed = false;

		// Restrict step:

		for( int ni = 0; ni < mNet->getNumNodes(); ++ni )
		{
			BayesNet::Node* node = mNet->getNode(ni);
			cands[node].clear();
			std::set<WeightedNode> pcnodes;

			// Score potential candidates
			for( int nj = 0; nj < mNet->getNumNodes(); ++nj )
			{
				BayesNet::Node* pcnode = mNet->getNode(nj);

				if( node == pcnode || node->hasInNode(pcnode) )
					// We don't consider the node itself or its parents as candidates
					continue;

				double minf = 0;
				if( node->getNumInNodes() > 0 )
					// TODO: Compute MCInf given *all* current parents!
					minf = _computeCondMutualInf( data, node->getAttribIndex(), pcnode->getAttribIndex(),
					node->getInNode(0)->getAttribIndex() );
				else
					minf = _computeMutualInf( data, node->getAttribIndex(), pcnode->getAttribIndex() );
				pcnodes.insert( WeightedNode( pcnode, minf ) );
			}

			// Choose best candidates
			int num_cands = BAYES_NUMCAND - node->getNumInNodes();
			int ci = 0;
			for( std::set<WeightedNode>::iterator pcni = pcnodes.begin();
				pcni != pcnodes.end(); ++pcni )
			{
				if( ci >= num_cands )
					break;

				cands[node].insert(pcni->node);
				++ci;
			}
		}

		// Maximize step:

		do
		{
			// Find and link best possible candidate parent:

			BayesNet::Node* best_node = NULL;
			BayesNet::Node* best_parent = NULL;
			double best_bic = _BIC();

			for( int ni = 0; ni < mNet->getNumNodes(); ++ni )
			{
				BayesNet::Node* node = mNet->getNode(ni);

				for( std::set<BayesNet::Node*>::iterator cpi = cands[node].begin();
					cpi != cands[node].end(); ++cpi )
				{
					BayesNet::Node* parent = *cpi;

					double bic =_BICArcAddTest( data, node, parent );
					if( bic > best_bic + BAYES_HCLIMBTHRESH )
					{
						best_bic = bic;
						best_node = node;
						best_parent = parent;
					}
				}
			}

			if( best_node == NULL )
				// We have converged on the optimal BN
				break;

			// Add best possible node
			_BICArcAdd( data, best_node, best_parent );
			bn_changed = true;
		}
		while(true);
	}
	while(bn_changed);
}

void BayesClassifier::_createNaiveBayesStructure( DataSet* data )
{
	BayesNet::Node* cnode = mNet->createNode( data->getNumAttributes() - 1 ); // assume last attribute is the class
	BayesNet::Node* node = NULL;
	for( int attr_i = 0; attr_i < data->getNumAttributes() - 1; ++attr_i )
	{
		node = mNet->createNode( attr_i,
			(int)data->getAttribute(attr_i).enumValues.size() );
		node->addInNode(cnode);
	}
}

void BayesClassifier::_learnNetParams( DataSet* data )
{
	BayesNet::Node* node = NULL;
	for( int attr_i = 0; attr_i < data->getNumAttributes(); ++attr_i )
	{
		node = mNet->getNode(attr_i);
		_learnNetNodeParams( data, node );
	}
}

void BayesClassifier::_learnNetNodeParams( DataSet* data, BayesNet::Node* node ) const
{
	int ri, vi;
	node->resetParams();

	for( int dpti = 0; dpti < data->getNumDataPoints(); ++dpti )
	{
		const DataPoint& dpt = data->getDataPoint(dpti);
		ri = node->getCPTRowIndex(dpt);
		vi = dpt.getEnumValue( node->getAttribIndex() );
		node->incCount( ri, vi, 1 );
	}

	node->initParams();
}

double BayesClassifier::_computeMutualInf( DataSet* data, int attr1, int attr2 ) const
{
	double minf = 0;

	int nv1 = (int)data->getAttribute(attr1).enumValues.size();
	int nv2 = (int)data->getAttribute(attr2).enumValues.size();
	int nv12 = nv1 * nv2;
	std::vector<double> ct1(nv1), ct2(nv2), ct12(nv12);
	int ndpt = data->getNumDataPoints();

	// Count the occurences of distinct values of attributes 1 and 2
	for( int dpti = 0; dpti < ndpt; ++dpti )
	{
		const DataPoint& dpt = data->getDataPoint(dpti);

		ct1[ dpt.getEnumValue(attr1) ] += 1;
		ct2[ dpt.getEnumValue(attr2) ] += 1;
		ct12[ dpt.getEnumValue(attr1) * nv1 + dpt.getEnumValue(attr2) ] += 1;
	}

	// Compute probability of each value
	for( int vi1 = 0; vi1 < nv1; ++vi1 )
		ct1[vi1] /= ndpt;
	for( int vi2 = 0; vi2 < nv2; ++vi2 )
		ct2[vi2] /= ndpt;
	for( int vi1 = 0; vi1 < nv1; ++vi1 )
		for( int vi2 = 0; vi2 < nv2; ++vi2 )
			ct12[vi1*nv1 + vi2] /= ndpt;

	// Compute mutual info
	double p1, p2, p12;
	for( int vi1 = 0; vi1 < nv1; ++vi1 )
	{
		for( int vi2 = 0; vi2 < nv2; ++vi2 )
		{
			p1 = ct1[vi1];
			p2 = ct2[vi2];
			p12 = ct12[vi1*nv1 + vi2];

			if( IS_ZERO(p1) || IS_ZERO(p2) || IS_ZERO(p12) )
			{
				continue;
			}

			double ld = log( p12/(p1*p2) )/log(2.0);
			minf += p12 * ld;
		}
	}

	return minf;
}

double BayesClassifier::_computeCondMutualInf( DataSet* data, int attr1, int attr2, int attrc ) const
{
	double mcinf = 0;
	int nvc = (int)data->getAttribute(attrc).enumValues.size();
	int nv1 = (int)data->getAttribute(attr1).enumValues.size();
	int nv2 = (int)data->getAttribute(attr2).enumValues.size();
	int nv1c = nv1 * nvc;
	int nv2c = nv2 * nvc;
	int nv12c = nv1 * nv2 * nvc;
	std::vector<double> ctc(nvc), ct1c(nv1c), ct2c(nv2c), ct12c(nv12c);
	int ndpt = data->getNumDataPoints();

	// Count the occurences of distinct values of the 3 attributes
	for( int dpti = 0; dpti < ndpt; ++dpti )
	{
		const DataPoint& dpt = data->getDataPoint(dpti);

		ctc[dpt.getEnumValue(attrc)] += 1;
		ct1c[ dpt.getEnumValue(attr1) * nv1 + dpt.getEnumValue(attrc) ] += 1;
		ct2c[ dpt.getEnumValue(attr2) * nv2 + dpt.getEnumValue(attrc) ] += 1;
		int i12c = dpt.getEnumValue(attr1) * nv1 * nv2 + dpt.getEnumValue(attr2) * nv2 + dpt.getEnumValue(attrc);		
		ct12c[i12c] += 1;
	}

	// Compute probability of each value
	for( int vic = 0; vic < nvc; ++vic )
		ctc[vic] /= ndpt;
	for( int vi1 = 0; vi1 < nv1; ++vi1 )
		for( int vic = 0; vic < nvc; ++vic )
			ct1c[vi1*nv1 + vic] /= ndpt;
	for( int vi2 = 0; vi2 < nv2; ++vi2 )
		for( int vic = 0; vic < nvc; ++vic )
			ct2c[vi2*nv2 + vic] /= ndpt;
	for( int vi1 = 0; vi1 < nv1; ++vi1 )
	{
		for( int vi2 = 0; vi2 < nv2; ++vi2 )
		{
			for( int vic = 0; vic < nvc; ++vic )
			{
				int i12c = vi1*nv1*nv2 + vi2*nv2 + vic;
				ct12c[i12c] /= ndpt;
			}
		}
	}

	// Compute mutual info
	double pc, p1c, p2c, p12c;
	for( int vi1 = 0; vi1 < nv1; ++vi1 )
	{
		for( int vi2 = 0; vi2 < nv2; ++vi2 )
		{
			for( int vic = 0; vic < nvc; ++vic )
			{
				pc = ctc[vic];
				p1c = ct1c[vi1*nv1 + vic];
				p2c = ct2c[vi2*nv2 + vic];
				int i12c = vi1*nv1*nv2 + vi2*nv2 + vic;
				p12c = ct12c[i12c];

				if( IS_ZERO(p1c) || IS_ZERO(p2c) || IS_ZERO(p12c) ||
					IS_ZERO(pc) )
				{
					continue;
				}

				double ld = log( (pc*p12c)/(p1c*p2c) )/log(2.0);
				mcinf += p12c * ld;
			}
		}
	}

	return mcinf;
}

void BayesClassifier::_buildNodeTree( BayesNet::Node* root, std::set<Arc>& arcs,
	std::set<BayesNet::Node*>& nodes )
{
	for( std::set<BayesNet::Node*>::iterator ni = nodes.begin();
		ni != nodes.end(); ++ni )
	{
		BayesNet::Node* tnode = *ni;
		Arc arc( root, tnode );

		if( arcs.count(arc) <= 0 )
			continue;

		tnode->addInNode(root);
		arcs.erase(arc);

		_buildNodeTree( tnode, arcs, nodes );
	}
}

double BayesClassifier::_BIC() const
{
	return mBICLogL + mBICPenalty;
}

double BayesClassifier::_BICInit( DataSet* data )
{
	mBICLogL = mBICPenalty = 0;

	// Compute log-likelihood term
	for( int dpti = 0; dpti < data->getNumDataPoints(); ++dpti )
	{
		const DataPoint& dpt = data->getDataPoint(dpti);

		for( int ni = 0; ni < mNet->getNumNodes(); ++ni )
		{
			BayesNet::Node* cnode = mNet->getNode(ni);
			int cattr = cnode->getAttribIndex();

			int row_i = cnode->getCPTRowIndex(dpt);
			double prob = cnode->getParam( row_i, dpt.getEnumValue(cattr) );
			prob = IS_ZERO(prob) ? 0 : log(prob);
			
			mBICLogL += prob;
		}
	}

	// Compute penalty term
	mBICPenalty = _BICPenalty(data);

	return _BIC();
}

double BayesClassifier::_BICPenalty( DataSet* data ) const
{
	double pen = 0;
	int num_params = 0,
		num_recs = data->getNumDataPoints();

	for( int ni = 0; ni < mNet->getNumNodes(); ++ni )
	{
		num_params += mNet->getNode(ni)->getNumParams();
	}

	return -((double)num_params)/2.0 * log((double)num_recs);
}

double BayesClassifier::_BICArcAddTest( DataSet* data, BayesNet::Node* node, BayesNet::Node* parent ) const
{
	if( node->hasInNode(parent) )
		// This node already is parent
		return -DBL_MAX;

	double cur_prob = _BICLogLAttr( data, node );

	// Save current CPT counts
	std::vector<std::vector<double>> cpt( node->getNumParams() );
	for( int row_i = 0; row_i < node->getNumParams(); ++row_i )
		for( int vi = 0; vi < node->getNumValues(); ++vi )
			cpt[row_i].push_back( node->getCount( row_i, vi ) );

	// Try to link the candidate parent to node
	if( !node->addInNode(parent) )
	{
		return -DBL_MAX;
	}
	_learnNetNodeParams( data, node );

	double new_prob = _BICLogLAttr( data, node );
	double logl = mBICLogL + new_prob - cur_prob;
	double pen = _BICPenalty(data);

	// Undo op
	node->removeInNode(parent);

	// Restore CPT counts
	for( int row_i = 0; row_i < node->getNumParams(); ++row_i )
		for( int vi = 0; vi < node->getNumValues(); ++vi )
			node->setCount( row_i, vi, cpt[row_i][vi] );
	node->initParams();

	return logl + pen;
}

double BayesClassifier::_BICArcAdd( DataSet* data, BayesNet::Node* node, BayesNet::Node* parent )
{
	double cur_prob = _BICLogLAttr( data, node );

	// Try to link the candidate parent to node
	node->addInNode(parent);
	_learnNetNodeParams( data, node );
	
	double new_prob = _BICLogLAttr( data, node );
	mBICLogL += new_prob - cur_prob;
	mBICPenalty = _BICPenalty(data);

	return _BIC();
}

double BayesClassifier::_BICLogLAttr( DataSet* data, BayesNet::Node* node ) const
{
	double prob = 0;
	int attr = node->getAttribIndex();

	for( int dpti = 0; dpti < data->getNumDataPoints(); ++dpti )
	{
		const DataPoint& dpt = data->getDataPoint(dpti);

		int row_i = node->getCPTRowIndex(dpt);
		double prob0 = node->getParam( row_i, dpt.getEnumValue(attr) );
		prob0 = IS_ZERO(prob0) ? 0 : log(prob0);
		
		prob += prob0;
	}

	return prob;
}

}
