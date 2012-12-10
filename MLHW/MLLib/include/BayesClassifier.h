
#ifndef __BayesClassifier_h__
#define __BayesClassifier_h__

#include "Prereqs.h"
#include "Classifier.h"
#include "BayesNet.h"

// Number of candidates considered by sparse candidate algorithm
#define BAYES_NUMCAND 3
#define BAYES_HCLIMBTHRESH 0.00001

namespace MLLib
{

/**
* Bayesian learning algorithm type.
*/
enum BayesType
{
	BT_Naive, ///< Naive Bayes.
	BT_TAN, ///< Tree-Augmented Naive Bayes.
	BT_SparseCandidate ///< Sparse Candidate Bayes.
};

/**
* Class representing a Bayesian Net classifier.
*/
class BayesClassifier : public Classifier
{

public:

	/**
	* Constructor.
	*/
	BayesClassifier();

	/**
	* Destructor.
	*/
	~BayesClassifier();

	/**
	* Gets the classifier type.
	*/
	ClassifierType getType() const { return CT_Bayes; }

	/**
	* Gets the Bayesian learning algorithm type.
	*/
	BayesType getBayesType() const { return mBType; }

	/**
	* Sets the Bayesian learning algorithm type.
	*/
	void setBayesType( BayesType bt ) { reset(); mBType = bt; }

	/**
	* Gets the current learned Bayesian Net.
	*/
	BayesNet* getBayesNet() const;

	/**
	* Trains the Bayesian Net classifier using the specified training dataset.
	*
	* @param Training data.
	*/
	void train( DataSet* data );

	/**
	* Resets the classifier, deleting the current Bayesian Net.
	*/
	void reset();

	/**
	* Classifies the instances from the specified dataset.
	*
	* @param data Dataset to classify.
	*/
	void classify( DataSet* data );

protected:

	/**
	* Bayesian Net arc.
	*/
	class Arc
	{

	public:

		Arc()
		{
			mNode1 = mNode2 = NULL;
		}

		Arc( BayesNet::Node* node1, BayesNet::Node* node2 )
			: mNode1(node1), mNode2(node2)
		{
			mlAssert( node1 != NULL && node2 != NULL );

			if( mNode1->getAttribIndex() > mNode2->getAttribIndex() )
				std::swap( mNode1, mNode2 );
		}

		BayesNet::Node* getNode1() const { return mNode1; }
		BayesNet::Node* getNode2() const { return mNode2; }

		bool operator<( const Arc& rhs ) const
		{
			mlAssert( mNode1 != NULL && mNode2 != NULL &&
				rhs.mNode1 != NULL && rhs.mNode2 != NULL );

			if( mNode1->getAttribIndex() < rhs.mNode1->getAttribIndex() ||
				mNode1->getAttribIndex() == rhs.mNode1->getAttribIndex() &&
				mNode2->getAttribIndex() < rhs.mNode2->getAttribIndex() )
				return true;

			return false;
		}

	private:

		BayesNet::Node* mNode1;
		BayesNet::Node* mNode2;

	};

	/**
	* Weighted Bayesian Net arc.
	*/
	struct WeightedArc
	{
		Arc arc;
		double weight;

		WeightedArc( const Arc& arc, double weight )
		{
			this->arc = arc;
			this->weight = weight;
		}

		bool operator<( const WeightedArc& rhs ) const
		{
			return weight > rhs.weight;
		}
	};

	/**
	* Weighted Bayesian Net node.
	*/
	struct WeightedNode
	{
		BayesNet::Node* node;
		double weight;

		WeightedNode( BayesNet::Node* node, double weight )
		{
			this->node = node;
			this->weight = weight;
		}

		bool operator<( const WeightedNode& rhs ) const
		{
			return weight > rhs.weight;
		}
	};

	virtual void _trainNaiveBayes( DataSet* data ); ///< Trains the Bayesian Net using Naive Bayes algorithm.
	virtual void _trainTAN( DataSet* data );  ///< Trains the Bayesian Net using TAN Bayes algorithm.
	virtual void _trainSparseCandidate( DataSet* data );  ///< Trains the Bayesian Net using Sparse Candidate algorithm.
	virtual void _createNaiveBayesStructure( DataSet* data ); ///< Creates the structure of the Bayesian net.
	virtual void _learnNetParams( DataSet* data ); ///< Learns the Bayesian Net parameters (doesn't support unknown values).
	virtual void _learnNetNodeParams( DataSet* data, BayesNet::Node* node ) const; ///< Learns the parameters of the specified Bayesian Net node (doesn't support unknown values).
	virtual double _computeMutualInf( DataSet* data, int attr1, int attr2 ) const; ///< Computes the mutual information of the two specified attributes.
	virtual double _computeCondMutualInf( DataSet* data, int attr1, int attr2, int attrc ) const; ///< Computes the conditional mutual information of the two specified attributes, given a third attribute.
	virtual void _buildNodeTree( BayesNet::Node* root, std::set<Arc>& arcs,
		std::set<BayesNet::Node*>& nodes ); ///< Builds a tree in the Bayesian Net, given a set of nodes and arcs.
	virtual double _BIC() const; ///< Gets the Bayesian Net's current BIC score.
	virtual double _BICInit( DataSet* data ); ///< Computes the Bayesian Net's initial BIC score.
	virtual double _BICPenalty( DataSet* data ) const; ///< Compute the BIC penalty term for the current Bayesian Net.
	virtual double _BICArcAddTest( DataSet* data, BayesNet::Node* node, BayesNet::Node* parent ) const; ///< Computes the BIC score of the Bayesian Net resulting from an arc add operation.
	virtual double _BICArcAdd( DataSet* data, BayesNet::Node* node, BayesNet::Node* parent ); ///< Add an arc to the Bayesian Net and updates the BIC score.
	virtual double _BICLogLAttr( DataSet* data, BayesNet::Node* node ) const; ///< Computes the log likelihood for the specified node in the Bayesian Net.

	BayesType mBType; ///< Bayesian learning algorithm type.
	BayesNet* mNet; ///< Bayesian Net.
	// Sparse candidate data:
	double mBICLogL; ///< BIC log likelihood score.
	double mBICPenalty; ///< BIC penalty.

};

}

#endif // __BayesClassifier_h__
