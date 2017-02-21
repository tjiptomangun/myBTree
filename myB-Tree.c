/**
 * gcc myB-Tree.c -Wall -ggdb3 -o myB-Tree
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myB-Tree.h"

/*
 * key assigned from left to right (lower to greater)
 * unassigned key is < 0
 *
 * */
int push (PSTACK S, PMYB7_NODE N, int dir)
{ 
	(*S).el[++(*S).top].pnd = N;
	(*S).el[(*S).top].e = dir;
	return 0;
}

PITEM pop (PSTACK S)
{
	if ((*S).top <0)
		return NULL;
	PITEM r = &(*S).el[(*S).top--];
	return r; 
}

void clean (PSTACK S)
{
	while (S->top>=0)
	{
		(*S).el[(*S).top].e = -1;
		(*S).el[(*S).top--].pnd = NULL;
	}
}

PSTACK stack_creat()
{
	PSTACK st = calloc (1, sizeof (STACK));
	memset ((*st).el, 0, 100 * sizeof (ITEM));
	(*st).top = -1;
	return st;
} 

/**
 * myb7_node
 * 1. Every node has at most 7 children
 * 2. Every internal node has at least ceil(7) (which is 4) children
 * 3. The root has at least 2 children
 * 4. A non-leaf node with k children contains k-1 keys
 * 5. All leaves appear in the same level
 * myb7_node valid keys >= 0
 */

void printmyb7_node (PMYB7_NODE pNode, int level)
{
	int i = 0;//child and key iteration
	int j = 0;//level helper
	if (!pNode)
		return;
	for (i = 0; i < (ORDER-1); i++)
	{
		if (pNode->c[i])
			printmyb7_node (pNode->c[i], level+1);
		if (pNode->k[i] < 0)
			break;
		for (j = 0; j<level; j++)
			fprintf  (stdout, "        ");
		fprintf (stdout, "%d\n", pNode->k[i]);
	}
	if (i == (ORDER-1))
	{
		printmyb7_node (pNode->c[i], level+1);
	}
}

void printmyb7_tree (PMYB7_TREE pTree)
{
	if (pTree->root)
		printmyb7_node (pTree->root, 0);
	else
		fprintf (stderr, "NULL\n");
}

void printmyb7_tree_flowered (PMYB7_TREE pTree)
{

	fprintf (stdout, "------flower---------\n");
	printmyb7_tree(pTree);
}
PMYB7_NODE allocmyb7_TreeNode ()
{
	PMYB7_NODE ret;
	ret = (PMYB7_NODE)calloc (1, sizeof (MYB7_NODE));
	ret->k[0] = ret->k[1] = ret->k[2] = ret->k[3] = -1;
	ret->k[4] = ret->k[5] = -1;
#ifdef _DEBUG_
			fprintf (stdout, "alloc %p\n", ret);
#endif

	return ret;
}
unsigned short is_leaf (PMYB7_NODE node)
{
	int i = 0;
	for (i = 0; i< ORDER; i++)
		if (node->c[i] != NULL)
			return 0;
	return 1;
}

PMYB7_TREE myb7tree_new (char *name)
{
	PMYB7_TREE ret;
	ret = (PMYB7_TREE) calloc (1, sizeof (MYB7_TREE));
	if (ret)
		strncpy (ret->name, name, 19);
	return ret;
}
/**
 * Searching is similar to searching a binary search tree.
 * Starting at the root, the tree is recursively traversed
 * from top to bottom. At each level, the search chooses the
 * child pointer (subtree) whose separation values are on
 * either side of the search value;
 * Name		: myb7_tree_find
 * Description	: find a key in tree
 * Input
 * 	tree	: tree to find key
 * 	key	: key to find in tree
 * Output
 * 	out	: pointer to hold pointer to find result
 * Returns
 * 	>=0	: index of in in out
 * 	<0	: error
 */
int myb7_tree_find(PMYB7_TREE tree, int key, PMYB7_NODE *out)
{
	int i = 0;
	PMYB7_NODE current = tree->root;
	while (current)
	{
		if (key < current->k[0])
		{
			current = current->c[0];
			continue;
		}
		for (i = 0; i<	NUMKEY ; i++)
		{
			if (current->k[i]==-1)
			{
				current = current->c[i+1];
				break;
			}
			else if (current->k[i] == key)
			{
				*out = current;
				return i;
			}
			else if (i == (NUMKEY - 1))
			{
				current = current->c[i+1];
				break; 
			}
			else if ((key > current->k[i]) && (key <current->k[i+1]))
			{
				current = current->c[i+1];
				break;
			} 
		}
	}
	*out = NULL;
	return -1;
}

/**
 * 1. If the node contains fewer than the maximum legal
 * number of elements, then there is room for the new
 * element. Insert the new element in the node, keeping
 * the node’s elements ordered.
 * the node’s elements ordered.
 * 2. Otherwise the node is full, evenly split it into two
 * nodes so:
 * (a) A single median is chosen from among the
 * leaf’s elements and the new element.
 * (b) Values less than the median are put in the new
 * left node and values greater than the median
 * are put in the new right node, with the median
 * acting as a separation value.
 * (c) The separation value is inserted in the node’s
 * parent, which may cause it to be split, and so 
 * on. If the node has no parent (i.e., the node
 * was the root), create a new root above this node
 * (increasing the height of the tree).
 * 
 * Name 	: myb7_tree_insert
 * Description	: insert a key into a tree
 * Input
 * 	tree	: tree to insert ke into
 * 	key	: key to insert into tree
 * Returns
 * 	0	: success
 * 	others	: there is already such key, this is
 * 		  the pointer to it
 */
PMYB7_NODE myb7_tree_insert(PMYB7_TREE tree, int key)
{

	int i = 0; //go right of key array,
	int j = 0; //holder of current in
	int e = 0; //indicator/direction
	PMYB7_NODE current = tree->root;
	int in = key; //key to insert
	PMYB7_NODE left; //left child of key to insert
	PMYB7_NODE right;//right child of key to insert
	PMYB7_NODE B; //right child will be
	STACK h;
	PSTACK H = &h;
	memset (H, 0, sizeof (STACK));
	H->top = -1;
	int new_pos; /*calculated key position*/
	int cand_pos = 0; /*posisition of key that will be inserted to parent*/
#ifdef _DEBUG_
	fprintf (stdout, "mb7_tree_insert %d\n", key);
#endif	
	while (current)
	{
		for (i = 0; i<	NUMKEY ; i++)
		{
			if (key < current->k[i])
			{
				e = i;
				push (H, current, i);
				current = current->c[i];
				break;
			}
			else if (current->k[i]==-1)
			{
				e = i;
				push (H, current, e);
				current = current->c[e];
				break;
				
			}
			else if (current->k[i] == key)
			{
				return current;
			}
			else if (i == (NUMKEY - 1))
			{
				//e = i;
				e = NUMKEY;
				push (H, current, e);
				current = current->c[e];
				break;
			
			}
		}
#ifdef _DEBUG_
		fprintf (stdout, "e = %d\n", e);
#endif			
	}
	left = right = NULL;
	while (H->top >= 0)
	{
#ifdef _DEBUG_
		printmyb7_tree (tree);
#endif
		PITEM r = pop (H);
		current = r->pnd;
		e = r->e;
		//case 1. node has vacant key
		if (r->pnd->k[NUMKEY - 1]<0)
		{
			for (i = 0; i < (NUMKEY); i++)
			{
				if (r->pnd->k[i]==-1 || (r->pnd->k[i] > in))
				{ 
					if ( r->pnd->k[i] > in )
					{
						j = NUMKEY - 2;
						for (;j>=i; j--)
						{
							r->pnd->c[j+2] = r->pnd->c[j+1];
							r->pnd->k[j+1] = r->pnd->k[j];
						}
					}
					r->pnd->k[i] = in;
					r->pnd->c[i] = left;
					r->pnd->c[i+1] = right; 
					return NULL;
				}
			}
		}
		else
		{
			i= 0;
			while  (in >r->pnd->k[i] && i<NUMKEY)
				i++;
			new_pos = i;
			cand_pos = (NUMKEY+2)/2 - 1;
			if (new_pos == cand_pos)
			{
				B = allocmyb7_TreeNode();
				for (j = 0; j< cand_pos; j++)
				{
					B->c[j] = r->pnd->c[cand_pos+j];
					B->k[j] = r->pnd->k[cand_pos+j];
					r->pnd->c[cand_pos+j] = NULL;
					r->pnd->k[cand_pos+j] = -1; 
				}
				B->c[j] = r->pnd->c[cand_pos+j];
				r->pnd->c[cand_pos+j] = NULL;
				r->pnd->c[cand_pos] = left;
				B->c[0] = right;

				r->pnd->k[cand_pos] = in;
				//in is still in use
			}
			else if (new_pos < cand_pos)
			{
				B = allocmyb7_TreeNode();//right child
				for (j = 0; j< cand_pos; j++)
				{
					B->k[j] = r->pnd->k[cand_pos+j];
					B->c[j] = r->pnd->c[cand_pos+j];
					r->pnd->k[cand_pos+j] = -1;
					r->pnd->c[cand_pos+j] = NULL;
				}
				B->c[j] = r->pnd->c[cand_pos+j];
				r->pnd->c[cand_pos+j] = NULL;
				for (j = cand_pos; j >new_pos ; j--)
				{
					r->pnd->c[j] = r->pnd->c[j-1];
					r->pnd->k[j] = r->pnd->k[j-1];
				}
				r->pnd->k[new_pos] = in;
				r->pnd->c[new_pos] = left;
				r->pnd->c[new_pos+1] = right;
			}
			else
			{
				B = allocmyb7_TreeNode();//right child
				for (j = 0; j< cand_pos-1; j++)
				{
					B->k[j] = r->pnd->k[cand_pos+j+1];
					B->c[j] = r->pnd->c[cand_pos+j+1];
					r->pnd->k[cand_pos+j+1] = -1;
					r->pnd->c[cand_pos+j+1] = NULL;
				}
				B->c[j] = r->pnd->c[cand_pos+j+1];
				r->pnd->c[cand_pos+j+1] = NULL;

				new_pos = new_pos - (cand_pos+1);
				for (j = cand_pos-1 ; j>new_pos; j--)
				{
					B->c[j+1] = B->c[j];
					B->k[j] = B->k[j-1];
				}
				B->k[new_pos] = in;
				B->c[new_pos] = left;
				B->c[new_pos+1] = right; 
			}
			in = r->pnd->k[cand_pos];
			r->pnd->k[cand_pos] = -1;
			r->pnd->c[cand_pos+1] = NULL;
			left  = r->pnd;
			right = B; 
		} 
	}
	B = allocmyb7_TreeNode();
	B->k[0] = in;
	B->c[0] = left;
	B->c[1] = right;
	tree->root = B;
	return NULL;
}
PMYB7_NODE leftmost (PMYB7_NODE a, PSTACK A)
{
	if (!a->c[0])
		return a;
	push(A, a, 0);
	return leftmost (a->c[0], A);
}

PMYB7_NODE rightmost (PMYB7_NODE a, PSTACK A)
{
	int j = NUMKEY;
	for (j = NUMKEY; j>=0 ; j--)
	{
		if (a->c[j])
		{
			push (A, a, j);
			return rightmost(a->c[j], A);
		}
	}
	return a;
}

PMYB7_NODE inpre (PMYB7_NODE a, int pos, PSTACK A)
{
	push (A, a, pos);
	return rightmost (a->c[pos], A); 
}

PMYB7_NODE inpos (PMYB7_NODE a, int pos, PSTACK A)
{
	push (A, a, pos+1);
	return leftmost (a->c[pos+1], A);
}

int last_key (PMYB7_NODE p)
{
	int i;
	for (i = 0; i < NUMKEY; i++)
	{
		if (p->k[i] == -1)
			break;
	}
	return i-1;
}

/**
 * hole node has a single sub tree
 */
int myb7_tree_del(PMYB7_TREE tree, int key)
{
	PMYB7_NODE current; /*walk down the tree to find match key*/
	current = tree->root;
	STACK h, *H = &h;
	PITEM pitem;
	memset (H, 0, sizeof (STACK));
	H->top  = -1;
	int pos = -1;
	PMYB7_NODE slay; /*the node deletion actually occurs*/
	int del = 0; //actual key index deleted in slay
	PMYB7_NODE hole = NULL; /*deleted node with 1 key and 2 children*/
	PMYB7_NODE sibling = NULL; /*sibling of hole*/ 
	PMYB7_NODE parent = NULL; /*parent of hole*/
	int i = 0; /*indices*/ 
	int siblastkey = -1;/*sibling last key*/

#ifdef _DEBUG_
	fprintf (stdout, "myb7_tree_del %d\n", key);
#endif
	while (current && pos < 0)
	{
		for (i = 0; i< NUMKEY; i++)
		{
#ifdef _DEBUG_
	fprintf (stdout, "k[%d]=%d    ", i, current->k[i]);
#endif

			if (key < current->k[i])
			{
				push (H, current, i);
				current = current->c[i];
				break;
			}
			else if (current->k[i] == -1)
			{
				push (H, current, i);
				current = current->c[i];
				break;
			}
			else if (key == current->k[i])
			{
				pos = i;
				break;
			}
			else if (i == NUMKEY -1)
			{
				push (H, current, NUMKEY);
				current = current->c[i+1]; 
				break;
			}

		}
	}
#ifdef _DEBUG_
	fprintf (stdout, "\n");
#endif
	if (!current)
		return -1;
	slay = current;	
	if (current->c[0] != NULL)
	{
		slay = inpos (current, pos, H);		
	}
	if (slay!=current)
	{//deleted key is not in a terminal node
		current->k[pos] = slay->k[0];
		del = 0;
	}
	else
	{
		del = pos;
	}
	if (slay->k[1]>=0)
	{//slay has another key
	//move slay key to deleted parent key
		//i = pos+1;
		i = del+1;
		while(slay->k[i-1]>=0 &&i <NUMKEY)
		{
			slay->k[i-1] = slay->k[i];
			i++;
		}
		slay->k[i-1] = -1; 
	}
	else
	{//hole
		current->k[pos] = slay->k[0]; //<-- suspect
		hole = slay; 
	}
	while (hole)
	{ 
#ifdef _DEBUG_
		printmyb7_tree(tree); 
		fprintf (stdout, "\nhole\n");
		printmyb7_node(hole, 0);
#endif
		if (hole == tree->root)
		{
			tree->root = hole->c[0];
#ifdef _DEBUG_
			fprintf (stdout, "free %p\n", hole);
#endif

			free(hole);
			break;
		}
		pitem = pop (H);
		parent = pitem->pnd;
		if (pitem->e == 0)
			sibling = parent->c[1];
		else
			sibling = parent->c[pitem->e -1];
		if (parent->k[1] >= 0)
		{//parent has more than 2 nodes
			if (sibling->k[1] >= 0)
			{//sibling has more than 2 nodes
				if (hole->k[0] > sibling->k[0])
				{//del_19.png
				 //hole is on the right of parent
				 //so we'll move parents last key to
				 //hole->k[0], and sibling's last
				 //key to parent->k[0]
					siblastkey = last_key(sibling);
					hole->k[0] = parent->k[pitem->e-1];
					parent->k[pitem->e-1] = sibling->k[siblastkey];
					hole->c[1] = hole->c[0];
					hole->c[0] = sibling->c[siblastkey+1];
					sibling->k[siblastkey] = -1;
					sibling->c[siblastkey+1] = NULL; 
				}
				else
				{//del_21.png
					hole->k[0] = parent->k[0];
					hole->c[1] = sibling->c[0];
					parent->k[0] = sibling->k[0];
					for (i = 0; sibling->k[i]>=0 && i < NUMKEY; i++)
					{
						sibling->c[i] = sibling->c[i+1];
						sibling->k[i] = sibling->k[i+1];
					}	
					sibling->c[i-1] = sibling->c[i]; 
				}
				hole = NULL;
			}
			else
			{//sibling has only 2 nodes
				if (hole->k[0] > sibling->k[0])
				{//del_18.png
					sibling->k[1] = parent->k[pitem->e-1];
					sibling->c[2] = hole->c[0];
					for (i = pitem->e-1; parent->k[i]>=0 && i < NUMKEY-1; i++)
					{
							parent->k[i] = parent->k[i+1];
							parent->c[i+1] = parent->c[i+2];
					}
					//201601272118
					parent->k[i] = -1;
					parent->c[i+1] = NULL;
#ifdef _DEBUG_
			fprintf (stdout, "free %p\n", hole);
#endif
					free (hole);
					hole = NULL;
				}
				else
				{//del_20.png
					for (i = NUMKEY-1; sibling->k[i] < 0; )
						i--;
					for (;i>=0;i--)
					{
						sibling->k[i+1] = sibling->k[i];
						sibling->c[i+2] = sibling->c[i+1]; 
					}
					sibling->c[i+2] = sibling->c[i+1]; 
					sibling->k[0] = parent->k[0];
					sibling->c[0] = hole->c[0]; 
					for (i = pitem->e+1; parent->k[i]>=0 && i < NUMKEY; i++)
					{
						parent->c[i-1] = parent->c[i];
						parent->k[i-1] = parent->k[i];
					}
					parent->c[i-1] = parent->c[i]; 
					parent->c[i] = NULL;
					parent->k[i-1] = -1;
#ifdef _DEBUG_
			fprintf (stdout, "free %p\n", hole);
#endif
					free (hole);
					hole = NULL; 
				}

			}
		}
		else
		{//parent has only 2 nodes
			if (sibling->k[1] >= 0)
			{//sibling has more than 2 nodes
				if (hole->k[0] > sibling->k[0])
				{//del_028.png
					siblastkey = last_key(sibling);
					hole->c[1] = hole->c[0];
					hole->c[0] = sibling->c[siblastkey+1];
					hole->k[0] = parent->k[pitem->e-1];
					parent->k[pitem->e-1] = sibling->k[siblastkey];
					sibling->c[siblastkey+1] = NULL;
					sibling->k[siblastkey] = -1;
					hole = NULL; 
				}
				else
				{//del_026.png
					hole->c[1] = sibling->c[0];
					hole->k[0] = parent->k[pitem->e];
					parent->k[pitem->e] = sibling->k[0];
					/*
					for (i = 0; sibling->k[i]>=0 && i < NUMKEY; i++)
					{
						sibling->c[i] = sibling->c[i+1];
						sibling->k[i] = sibling->k[i+1];
					}
					sibling->c[i-1] = sibling->c[i]; 
					*/
					for (i = 1; sibling->k[i]>=0 && i < NUMKEY; i++)
					{
						sibling->c[i-1] = sibling->c[i];
						sibling->k[i-1] = sibling->k[i];
					}
					sibling->c[i-1] = sibling->c[i]; 
					sibling->k[i-1] = -1;
					sibling->c[i] = NULL;

					hole = NULL; 
				}
			}
			else
			{//sibling has only 2 nodes
				if (hole->k[0] > sibling->k[0])
				{//del_027.png
					sibling->c[2] = hole->c[0];
					sibling->k[1] = parent->k[pitem->e-1];
#ifdef _DEBUG_
			fprintf (stdout, "free %p\n", hole);
#endif
					free (hole);
					hole = parent;
					hole->c[1] = NULL;
				}
				else
				{//del_029.png
					sibling->c[2] = sibling->c[1];
					sibling->k[1] = sibling->k[0];
					sibling->c[1] = sibling->c[0];
					sibling->k[0] = parent->k[pitem->e];
					sibling->c[0] = hole->c[0];
					parent->c[0] = parent->c[1];
#ifdef _DEBUG_
			fprintf (stdout, "free %p\n", hole);
#endif
					free(hole);
					hole = parent;
					hole->c[1] = NULL; 
				} 
			} 
		}
	} 
	return 0;
}
