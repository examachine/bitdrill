#ifndef __CITEMSET_TREE_H
#define __CITEMSET_TREE_H

#include <vector>

#include "SortedIntArray.h"

#include <cstdio>

class CItemsetNode;
typedef CItemsetNode * pCItemsetNode;

struct SChildNodes
{
	SChildNodes(int init);
	~SChildNodes();

	CItemsetNode **children;
	int childrenLength;
	int childrenCount;
};

class CItemsetNode
{
public:
	CItemsetNode(int id, CItemsetNode *parentNode): item(id), parent(parentNode), support(-1), children(NULL) {};
	~CItemsetNode();

			// Check if the node up-trail contains part of the itemset
	bool IsContaining(CExtendIntArray *itemset);				

			// Insert an itemset into the node
	CItemsetNode *Insert(CExtendIntArray *itemset, int pos);

	void DebugCount(int &count, int &zero, int &one)
	{
		count++;

		if (children == NULL)
		{
			zero++;
			return;
		}
		if (children->childrenCount == 1)
			one++;

		for (int loop = 0; loop < children->childrenCount; loop++)
			children->children[loop]->DebugCount(count, zero, one);
	}

	int support;
private:
	int item;
	CItemsetNode *parent;

	SChildNodes *children;
};

class CItemsetTree
{
public:
	CItemsetTree(void): root(-1, NULL) {}
	~CItemsetTree(void) 
	{
		int count = 0, zero = 0, one = 0;
		root.DebugCount(count, zero, one);
		printf("%d %d %d\n", count, zero, one);
	}

	CItemsetNode *Insert(CExtendIntArray *other) { return root.Insert(other,0); }
private:
	CItemsetNode root;
};

// My tree
class CNode
{
public:
	CNode(int val): value(val), right(NULL), left(NULL) {}
	~CNode() { if (left!=NULL) delete left; if (right!=NULL) delete right; }

	CNode *Find(int val)
	{
		if (value == val)
			return this;

		if (value < val)
		{
			if (right == NULL)
			{
				right = new CNode(val);
				return right;
			}
			else
				return right->Find(val);
		}

		//else - left
		if (left == NULL)
		{
			left = new CNode(val);
			return left;
		}
		else
			return left->Find(val);
	}

	std::vector<CItemsetNode *> v;
	int value;
private:
	CNode *left, *right;
};

class CTree
{
public:
	CTree(): root(NULL) {}
	~CTree() { if (root != NULL) delete root; }

	CNode *Find(int val)
	{
		if (root == NULL)
		{
			root = new CNode(val);
			return root;
		}

		return root->Find(val);		
	}

private:
	CNode *root;
};
#endif

