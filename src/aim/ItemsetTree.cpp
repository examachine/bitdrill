#include "ItemsetTree.h"

#include <cstring>

SChildNodes::SChildNodes(int init)
{
	children = new pCItemsetNode[init];
	childrenLength = init;
	childrenCount = 0;
}

SChildNodes::~SChildNodes()
{
	for (int loop = 0; loop < childrenCount; loop++)
		delete children[loop];

	delete[] children;
}

CItemsetNode::~CItemsetNode()
{
	if (children != NULL)
		delete children;
}

bool CItemsetNode::IsContaining(CExtendIntArray *itemset)
{
	CItemsetNode *node = this;
	int pos = itemset->Count - 1;

	while (true)
	{
		if (node->item == -1)
			return false;

			// Found item in node
		if ((*itemset)[pos] == node->item)
		{
			if (pos == 0)
				return true;

			pos = pos - 1;
			node = node->parent;
		}
			// Current item is smaller than the node
		else if ((*itemset)[pos] < node->item)
		{
			node = node->parent;
		}
			// Current item is larger than the node... abort
		else 
			return false;
	}

	return false;
}

CItemsetNode *CItemsetNode::Insert(CExtendIntArray *other, int pos)
{
			// If the position in the itemset is at the end of the
			// itemset then the current node represent the end of the 
			// itemset in the tree
	if (pos == other->Count)
		return this;

	if (children == NULL)
		children = new SChildNodes(1);

			// Search for the child node for the given item,
			// if found - recurse into.
	for (int loop = 0; loop < children->childrenCount; loop++)
	{
		if ((*other)[pos] == children->children[loop]->item)
			return children->children[loop]->Insert(other, pos + 1);
	}

			// No child was found for the item - create a new child, and
			// recurse into.
	CItemsetNode *newNode = new CItemsetNode((*other)[pos], this);
	if (children->childrenLength == children->childrenCount)
	{
		CItemsetNode **newChilds = new pCItemsetNode[children->childrenLength * 2];
		memcpy(newChilds, children->children, children->childrenCount * sizeof(pCItemsetNode));
		delete[] children->children;
		children->children = newChilds;
		children->childrenLength *= 2;
	}
	children->children[children->childrenCount++] = newNode;

	return newNode->Insert(other, pos + 1);
}
