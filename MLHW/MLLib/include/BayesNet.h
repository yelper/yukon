
#ifndef __BayesNet_h__
#define __BayesNet_h__

#include "Prereqs.h"
#include "DataPoint.h"

namespace MLLib
{

/**
* Class representing a Bayesian net.
*/
class BayesNet
{

public:

	/**
	* Bayesian Net node.
	*/
	class Node
	{

	public:

		/**
		* Constructor.
		*
		* @param attribIndex Corresponding attribute index.
		* @param numValues Number of possible attribute values.
		*/
		Node( int attribIndex, int numValues = 2 );

		/**
		* Destructor.
		*/
		~Node();

		/**
		* Gets the corresponding attribute index.
		*/
		int getAttribIndex() const;

		/**
		* Adds as input node to this node.
		*
		* @param Input node.
		* @return true if the node was added, false if it could not be added
		* (e.g. because it would introduce a cycle).
		*/
		bool addInNode( Node* node );

		/**
		* Removes an input node from this node.
		*
		* @param index Index of the input node.
		*/
		void removeInNode( int index );

		/**
		* Removes an input node from this node.
		*
		* @param node Input node.
		*/
		void removeInNode( Node* node );

		/**
		* Removes all input nodes from this node.
		*/
		void removeAllInNodes();

		/**
		* Gets one of this node's input nodes.
		*
		* @param index Index of the input node.
		* @return Input node.
		*/
		Node* getInNode( int index ) const;

		/**
		* Gets the number of this node's input nodes.
		*
		* @return Number of input nodes.
		*/
		int getNumInNodes() const;

		/**
		* Checks whether this node has the specified input node.
		*
		* @param node Input node to check.
		* @return true if this node has the specified input node,
		* false otherwise.
		*/
		bool hasInNode( Node* node ) const;

		/**
		* Gets one of this node's output nodes.
		*
		* @param index Index of the output node.
		* @return Output node.
		*/
		Node* getOutNode( int index ) const;

		/**
		* Gets the number of this node's output nodes.
		*
		* @return Number of output nodes.
		*/
		int getNumOutNodes() const;

		/**
		* Gets the number of possible values of this node's
		* corresponding attribute.
		*/
		int getNumValues() const;

		/**
		* Gets one of this node's stored parameters.
		* 
		* @param rowIndex CPT row index.
		* @param valueIndex Attribute value.
		*/
		double getParam( int rowIndex, int valueIndex ) const;

		/**
		* Gets the number of stored parameters in the node.
		* (i.e. number of CPT entries).
		* 
		* @return Number of parameters.
		*/
		int getNumParams() const;

		/**
		* Increments one of the node's pseudocounts.
		*
		* @param rowIndex CPT row index.
		* @param valueIndex Attribute value.
		* @param count Value by which to increment the pseudocount.
		*/
		void incCount( int rowIndex, int valueIndex, double count );

		/**
		* Gets one of the node's pseudocounts.
		*
		* @param rowIndex CPT row index.
		* @param valueIndex Attribute value.
		* @return Pseudocount.
		*/
		double getCount( int rowIndex, int valueIndex ) const;

		/**
		* Sets one of the node's pseudocounts.
		*
		* @param rowIndex CPT row index.
		* @param valueIndex Attribute value.
		* @param count Pseudocount.
		*/
		void setCount( int rowIndex, int valueIndex, double count );

		/**
		* Initializes the node's parameters from pseudocounts.
		*/
		void initParams();

		/**
		* Resets the node's parameters and pseudocounts.
		*/
		void resetParams();

		/**
		* Gets the CPT row index corresponding to the values of the specified
		* data instance.
		*/
		int getCPTRowIndex( const DataPoint& dpt ) const;

	private:

		bool _detectCycle( Node* node, std::set<Node*> visitedNodes ) const; ///< true if there is a cycle in the Bayesian Net, false otherwise.

		int mAttribIndex;
		std::vector<Node*> mInNodes;
		std::vector<Node*> mOutNodes;
		std::vector< std::vector<double> > mParams;
		std::vector< std::vector<double> > mHyperParams; ///< Pseudocounts.
		int mNumValues;

	};

	/**
	* Constructor.
	*/
	BayesNet();

	/**
	* Destructor.
	*/
	~BayesNet();

	/**
	* Creates a node in the Bayesian Net.
	*
	* @param attribIndex Corresponding attribute index.
	* @param numValues Number of possible attribute values.
	* @return Node.
	*/
	Node* createNode( int attribIndex, int numValues = 2 );

	/**
	* Deletes all nodes in the Bayesian Net.
	*/
	void deleteAllNodes();

	/**
	* Checks if the Bayesian Net contains a node for the specified
	* attribute.
	*
	* @param attribIndex Attribute index.
	* @return true if the node exists, false otherwise.
	*/
	bool hasNode( int attribIndex ) const;

	/**
	* Gets the specified node in the Bayesian Net.
	*
	* @param attribIndex Attribute index.
	* @return Node.
	*/
	Node* getNode( int attribIndex ) const;

	/**
	* Gets the number of nodes in the Bayesian Net.
	*
	* @return Number of nodes.
	*/
	int getNumNodes() const;
	
	/**
	* Performs inference on the Bayesian Net for the specified data instance.
	*
	* @param dpt Instance.
	* @return Probability for the given instance.
	*/
	double query( const DataPoint& dpt ) const;
	
	/**
	* Performs inference on the Bayesian Net for the specified data instance.
	*
	* @param dpt Instance.
	* @return Log probability for the given instance.
	*/
	double queryLog( const DataPoint& dpt ) const;
	
	/**
	* Classifies the specified data instance according to the most probable
	* classification.
	*
	* @param dpt Instance.
	* @return Probability of the assigned classification.
	*/
	double classify( DataPoint& dpt ) const;

private:

	std::map<int, Node*> mNodes;

};

}

#endif // __BayesNet_h__

