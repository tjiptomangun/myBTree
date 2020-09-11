/**
 * gcc my_btree_string.c -Wall -ggdb3 -o my_btree_string
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "my_btree_string.h"

static void key_strcpy(char *dst, char *src) {
	strncpy(dst, src, B7_G_KSIZE - 1);
}
/*
 * key assigned from left to right (lower to greater)
 * unassigned key is < 0
 *
 * */
int push (PSTRING_STACK S, PMYB7_STRING_NODE N, int dir)
{ 
	(*S).el[++(*S).top].pnd = N;
	(*S).el[(*S).top].e = dir;
	return 0;
}

PSTRING_ITEM pop (PSTRING_STACK S)
{
	if ((*S).top <0)
		return NULL;
	PSTRING_ITEM r = &(*S).el[(*S).top--];
	return r; 
}

void clean (PSTRING_STACK S)
{
	while (S->top>=0)
	{
		(*S).el[(*S).top].e = -1;
		(*S).el[(*S).top--].pnd = NULL;
	}
}

PSTRING_STACK stack_creat()
{
	PSTRING_STACK st = calloc (1, sizeof (STRING_STACK));
	memset ((*st).el, 0, 100 * sizeof (STRING_ITEM));
	(*st).top = -1;
	return st;
} 

/**
 * myb7_node
 * 1. Every node has at most 7 children
 * 2. Every internal node has at least ceil(7/2) (which is 4) children
 * 3. The root has at least 2 children
 * 4. A non-leaf node with k children contains k-1 keys
 * 5. All leaves appear in the same level
 * myb7_node valid keys >= 0
 */

void printmyb7_node (PMYB7_STRING_NODE pNode, int level)
{
	int i = 0;//child and key iteration
	int j = 0;//level helper
	if (!pNode)
		return;
	for (i = 0; i < (ORDER-1); i++)
	{
		if (pNode->c[i])
			printmyb7_node (pNode->c[i], level+1);
		if (pNode->k[i][0] == '\0')
			break;
		for (j = 0; j<level; j++)
			fprintf  (stdout, "        ");
		fprintf (stdout, "%s\n", pNode->k[i]);
	}
	if (i == (ORDER-1))
	{
		printmyb7_node (pNode->c[i], level+1);
	}
}

void printmyb7_tree (PMYB7_STRING_TREE pTree)
{
	if (pTree->root)
		printmyb7_node (pTree->root, 0);
	else
		fprintf (stderr, "NULL\n");
}

void printmyb7_tree_flowered (PMYB7_STRING_TREE pTree)
{

	fprintf (stdout, "------flower---------\n");
	printmyb7_tree(pTree);
}
PMYB7_STRING_NODE allocmyb7_TreeNode ()
{
	PMYB7_STRING_NODE ret;
	ret = (PMYB7_STRING_NODE)calloc (1, sizeof (MYB7_STRING_NODE));
	ret->k[0][0] = ret->k[1][0] = ret->k[2][0] = ret->k[3][0] = '\0';
	ret->k[4][0] = ret->k[5][0] = '\0';
#ifdef _DEBUG_
			fprintf (stdout, "alloc %p\n", ret);
#endif

	return ret;
}
unsigned short is_leaf (PMYB7_STRING_NODE node)
{
	int i = 0;
	for (i = 0; i< ORDER; i++)
		if (node->c[i] != NULL)
			return 0;
	return 1;
}

PMYB7_STRING_TREE myb7tree_new (char *name)
{
	PMYB7_STRING_TREE ret;
	ret = (PMYB7_STRING_TREE) calloc (1, sizeof (MYB7_STRING_TREE));
	if (ret)
		key_strcpy(ret->name, name);
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
int myb7_tree_find(PMYB7_STRING_TREE tree, char *key, int  (*fn) (char *, char*), PMYB7_STRING_NODE *out)
{
	int i = 0;
	PMYB7_STRING_NODE current = tree->root;
	while (current)
	{
		if (fn(key,  current->k[0]) < 0)
		{
			current = current->c[0];
			continue;
		}
		for (i = 0; i<	NUMKEY ; i++)
		{
			if (current->k[i][0]=='\0')
			{
				current = current->c[i+1];
				break;
			}
			else if (!fn(current->k[i],key))
			{
				*out = current;
				return i;
			}
			else if (i == (NUMKEY - 1))
			{
				current = current->c[i+1];
				break; 
			}
			else if ((fn(key, current->k[i]) > 0) && (current->k[i+1][0] == '\0' || fn(key, current->k[i+1]) < 0) )
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
PMYB7_STRING_NODE myb7_tree_insert(PMYB7_STRING_TREE tree, char *key, int  (*fn) (char *, char *))
{

	int i = 0; //go right of key array,
	int j = 0; //holder of current in
	int e = 0; //indicator/direction
	PMYB7_STRING_NODE current = tree->root;
	char  in[30] = {0};
	strcpy(in,  key); //key to insert
	PMYB7_STRING_NODE left; //left child of key to insert
	PMYB7_STRING_NODE right;//right child of key to insert
	PMYB7_STRING_NODE B; //right child will be
	STRING_STACK h;
	PSTRING_STACK H = &h;
	memset (H, 0, sizeof (STRING_STACK));
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
			if (fn(in, current->k[i]) < 0)
			{
				e = i;
				push (H, current, i);
				current = current->c[i];
				break;
			}
			else if (current->k[i][0] == '\0')
			{
				e = i;
				push (H, current, e);
				current = current->c[e];
				break;
				
			}
			else if (!fn(current->k[i], key))
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
		PSTRING_ITEM r = pop (H);
		current = r->pnd;
		e = r->e;
		//case 1. node has vacant key
		if (r->pnd->k[NUMKEY - 1][0] == '\0')
		{
			for (i = 0; i < (NUMKEY); i++)
			{
				if (r->pnd->k[i][0]=='\0' || (fn(r->pnd->k[i], in) > 0))
				{ 
					if ( fn(r->pnd->k[i], in) > 0 )
					{
						j = NUMKEY - 2;
						for (;j>=i; j--)
						{
							r->pnd->c[j+2] = r->pnd->c[j+1];
							key_strcpy(r->pnd->k[j+1], r->pnd->k[j]);
						}
					}
					key_strcpy(r->pnd->k[i], in);
					r->pnd->c[i] = left;
					r->pnd->c[i+1] = right; 
					return NULL;
				}
			}
		}
		else
		{
			i= 0;
			while  (fn(in, r->pnd->k[i]) > 0 && i<NUMKEY)
				i++;
			new_pos = i;
			cand_pos = (NUMKEY+2)/2 - 1;
			if (new_pos == cand_pos)
			{
				B = allocmyb7_TreeNode();
				for (j = 0; j< cand_pos; j++)
				{
					B->c[j] = r->pnd->c[cand_pos+j];
					key_strcpy(B->k[j],r->pnd->k[cand_pos+j]);
					r->pnd->c[cand_pos+j] = NULL;
					r->pnd->k[cand_pos+j][0] = '\0'; 
				}
				B->c[j] = r->pnd->c[cand_pos+j];
				r->pnd->c[cand_pos+j] = NULL;
				r->pnd->c[cand_pos] = left;
				B->c[0] = right;

				key_strcpy(r->pnd->k[cand_pos], in);
				//in is still in use
			}
			else if (new_pos < cand_pos)
			{
				B = allocmyb7_TreeNode();//right child
				for (j = 0; j< cand_pos; j++)
				{
					key_strcpy(B->k[j], r->pnd->k[cand_pos+j]);
					B->c[j] = r->pnd->c[cand_pos+j];
					r->pnd->k[cand_pos+j][0] = '\0';
					r->pnd->c[cand_pos+j] = NULL;
				}
				B->c[j] = r->pnd->c[cand_pos+j];
				r->pnd->c[cand_pos+j] = NULL;
				for (j = cand_pos; j >new_pos ; j--)
				{
					r->pnd->c[j] = r->pnd->c[j-1];
					key_strcpy(r->pnd->k[j], r->pnd->k[j-1]);
				}
				key_strcpy(r->pnd->k[new_pos], in);
				r->pnd->c[new_pos] = left;
				r->pnd->c[new_pos+1] = right;
			}
			else
			{
				B = allocmyb7_TreeNode();//right child
				for (j = 0; j< cand_pos-1; j++)
				{
					key_strcpy(B->k[j], r->pnd->k[cand_pos+j+1]);
					B->c[j] = r->pnd->c[cand_pos+j+1];
					r->pnd->k[cand_pos+j+1][0] = '\0';
					r->pnd->c[cand_pos+j+1] = NULL;
				}
				B->c[j] = r->pnd->c[cand_pos+j+1];
				r->pnd->c[cand_pos+j+1] = NULL;

				new_pos = new_pos - (cand_pos+1);
				for (j = cand_pos-1 ; j>new_pos; j--)
				{
					B->c[j+1] = B->c[j];
					key_strcpy(B->k[j], B->k[j-1]);
				}
				key_strcpy(B->k[new_pos], in);
				B->c[new_pos] = left;
				B->c[new_pos+1] = right; 
			}
			key_strcpy(in, r->pnd->k[cand_pos]);
			r->pnd->k[cand_pos][0] = '\0';
			r->pnd->c[cand_pos+1] = NULL;
			left  = r->pnd;
			right = B; 
		} 
	}
	B = allocmyb7_TreeNode();
	key_strcpy(B->k[0], in);
	B->c[0] = left;
	B->c[1] = right;
	tree->root = B;
	return NULL;
}
PMYB7_STRING_NODE leftmost (PMYB7_STRING_NODE a, PSTRING_STACK A)
{
	if (!a->c[0])
		return a;
	push(A, a, 0);
	return leftmost (a->c[0], A);
}

PMYB7_STRING_NODE rightmost (PMYB7_STRING_NODE a, PSTRING_STACK A)
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

PMYB7_STRING_NODE inpre (PMYB7_STRING_NODE a, int pos, PSTRING_STACK A)
{
	push (A, a, pos);
	return rightmost (a->c[pos], A); 
}

PMYB7_STRING_NODE inpos (PMYB7_STRING_NODE a, int pos, PSTRING_STACK A)
{
	push (A, a, pos+1);
	return leftmost (a->c[pos+1], A);
}

int last_key (PMYB7_STRING_NODE p)
{
	int i;
	for (i = 0; i < NUMKEY; i++)
	{
		if (p->k[i][0] == '\0')
			break;
	}
	return i-1;
}

/**
 * hole node has a single sub tree
 */
int myb7_tree_del(PMYB7_STRING_TREE tree, char *key, int (*fn) (char *, char *))
{
	PMYB7_STRING_NODE current; /*walk down the tree to find match key*/
	current = tree->root;
	STRING_STACK h, *H = &h;
	PSTRING_ITEM pitem;
	memset (H, 0, sizeof (STRING_STACK));
	H->top  = -1;
	int pos = -1;
	PMYB7_STRING_NODE slay; /*the node deletion actually occurs*/
	int del = 0; //actual key index deleted in slay
	PMYB7_STRING_NODE hole = NULL; /*deleted node with 1 key and 2 children*/
	PMYB7_STRING_NODE sibling = NULL; /*sibling of hole*/ 
	PMYB7_STRING_NODE parent = NULL; /*parent of hole*/
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
	fprintf (stdout, "k[%d]=%s    ", i, current->k[i]);
#endif

			if (fn(key, current->k[i]) < 0)
			{
				push (H, current, i);
				current = current->c[i];
				break;
			}
			else if (current->k[i][0] == '\0')
			{
				push (H, current, i);
				current = current->c[i];
				break;
			}
			else if (!fn(key, current->k[i]))
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
		key_strcpy(current->k[pos], slay->k[0]);
		del = 0;
	}
	else
	{
		del = pos;
	}
	if (slay->k[1][0])
	{//slay has another key
	//move slay key to deleted parent key
		//i = pos+1;
		i = del+1;
		while(slay->k[i-1][0] &&i <NUMKEY)
		{
			key_strcpy(slay->k[i-1], slay->k[i]);
			i++;
		}
		slay->k[i-1][0] = '\0'; 
	}
	else
	{//hole
		key_strcpy(current->k[pos], slay->k[0]); //<-- suspect
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
		if (parent->k[1][0] != '\0')
		{//parent has more than 2 nodes
			if (sibling->k[1][0] != '\0')
			{//sibling has more than 2 nodes
				if (fn(hole->k[0], sibling->k[0]) > 0)
				{//del_19.png
				 //hole is on the right of parent
				 //so we'll move parents last key to
				 //hole->k[0], and sibling's last
				 //key to parent->k[0]
					siblastkey = last_key(sibling);
					key_strcpy(hole->k[0], parent->k[pitem->e-1]);
					key_strcpy(parent->k[pitem->e-1], sibling->k[siblastkey]);
					hole->c[1] = hole->c[0];
					hole->c[0] = sibling->c[siblastkey+1];
					sibling->k[siblastkey][0] = '\0';
					sibling->c[siblastkey+1] = NULL; 
				}
				else
				{//del_21.png
					key_strcpy(hole->k[0], parent->k[0]);
					hole->c[1] = sibling->c[0];
					key_strcpy(parent->k[0], sibling->k[0]);
					for (i = 0; sibling->k[i][0]!='\0' && i < NUMKEY; i++)
					{
						sibling->c[i] = sibling->c[i+1];
						key_strcpy(sibling->k[i], sibling->k[i+1]);
					}	
					sibling->c[i-1] = sibling->c[i]; 
				}
				hole = NULL;
			}
			else
			{//sibling has only 2 nodes
				if (fn(hole->k[0], sibling->k[0]) > 0)
				{//del_18.png
					key_strcpy(sibling->k[1],  parent->k[pitem->e-1]);
					sibling->c[2] = hole->c[0];
					for (i = pitem->e-1; parent->k[i][0]!='\0' && i < NUMKEY-1; i++)
					{
							key_strcpy(parent->k[i], parent->k[i+1]);
							parent->c[i+1] = parent->c[i+2];
					}
					//201601272118
					parent->k[i][0] = '\0';
					parent->c[i+1] = NULL;
#ifdef _DEBUG_
			fprintf (stdout, "free %p\n", hole);
#endif
					free (hole);
					hole = NULL;
				}
				else
				{//del_20.png
					for (i = NUMKEY-1; sibling->k[i][0] == '\0'; )
						i--;
					for (;i>=0;i--)
					{
						key_strcpy(sibling->k[i+1], sibling->k[i]);
						sibling->c[i+2] = sibling->c[i+1]; 
					}
					sibling->c[i+2] = sibling->c[i+1]; 
					key_strcpy(sibling->k[0],  parent->k[0]);
					sibling->c[0] = hole->c[0]; 
					for (i = pitem->e+1; parent->k[i][0] != '\0' && i < NUMKEY; i++)
					{
						parent->c[i-1] = parent->c[i];
						key_strcpy(parent->k[i-1], parent->k[i]);
					}
					parent->c[i-1] = parent->c[i]; 
					parent->c[i] = NULL;
					parent->k[i-1][0] = '\0';
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
			if (sibling->k[1][0] != '\0')
			{//sibling has more than 2 nodes
				if (fn(hole->k[0], sibling->k[0]) > 0)
				{//del_028.png
					siblastkey = last_key(sibling);
					hole->c[1] = hole->c[0];
					hole->c[0] = sibling->c[siblastkey+1];
					key_strcpy(hole->k[0], parent->k[pitem->e-1]);
					key_strcpy(parent->k[pitem->e-1], sibling->k[siblastkey]);
					sibling->c[siblastkey+1] = NULL;
					sibling->k[siblastkey][0] = '\0';
					hole = NULL; 
				}
				else
				{//del_026.png
					hole->c[1] = sibling->c[0];
					key_strcpy(hole->k[0], parent->k[pitem->e]);
					key_strcpy(parent->k[pitem->e],  sibling->k[0]);
					/*
					for (i = 0; sibling->k[i]>=0 && i < NUMKEY; i++)
					{
						sibling->c[i] = sibling->c[i+1];
						sibling->k[i] = sibling->k[i+1];
					}
					sibling->c[i-1] = sibling->c[i]; 
					*/	
					for (i = 1; sibling->k[i][0] != '\0' && i < NUMKEY; i++)
					{
						sibling->c[i-1] = sibling->c[i];
						key_strcpy(sibling->k[i-1], sibling->k[i]);
					}
					sibling->c[i-1] = sibling->c[i]; 
					sibling->k[i-1][0] = '\0';
					sibling->c[i] = NULL;

					hole = NULL; 
				}
			}
			else
			{//sibling has only 2 nodes
				if (fn(hole->k[0], sibling->k[0]) > 0)
				{//del_027.png
					sibling->c[2] = hole->c[0];
					key_strcpy(sibling->k[1], parent->k[pitem->e-1]);
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
					key_strcpy(sibling->k[1],  sibling->k[0]);
					sibling->c[1] = sibling->c[0];
					key_strcpy(sibling->k[0], parent->k[pitem->e]);
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
#ifdef _STRING_INPUT_
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
void usage(char *app) {
	fprintf (stdout, "usage : %s i:pf:c:hsr:mudta:e:uj:o:g:l:x:w:\n\
		-i --       : insert a value\n\
		-p -- print : tree\n\
		-f -- find  : find single key\n\
		-t -- top   : go to root tree\n\
		-r -- remove: remove a key\n\
		-h --help   : this help\n\
		-m --memtest : memory leak test, loop create update and delete test\n\
		-e           : echo something\n\
", app);
}

int main (int argc, char **argv) {
	int c;
	char buff[200] = {0};
	PMYB7_STRING_NODE child_found = NULL;
	int key_index = 0;
	PMYB7_STRING_TREE pTree = myb7tree_new ("first tree"); 
	while(1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"i", required_argument, 0, 'i'},
			{"find", required_argument, 0, 'f'},
			{"print", no_argument, 0, 'p'},
			{"help", no_argument, 0, 'h'},
			{"remove", required_argument, 0, 'r'},
			{"echo", required_argument, 0, 'e'},
			{"memtest", no_argument, 0, 'm'},
			{0, 0, 0, 0}
		};
		c  = getopt_long(argc, argv, "i:pf:hr:me:", long_options, &option_index);
		
		if (c == -1){
			break;
		}
		
		switch (c) {
			case 'i' :
				strcpy(buff, optarg);
				myb7_tree_insert(pTree, buff, (int (*)(char *, char *))strcmp);
				break;
			case 'p' :
				printmyb7_tree(pTree);
				break;
			case 'f' :
				child_found = NULL;
				key_index = -1;
				strcpy(buff, optarg);
				key_index = myb7_tree_find(pTree, buff, (int (*)(char *, char *))strcmp, &child_found);
				if (key_index < 0) {
					fprintf(stdout, "key not found %s\n", buff);
				}
				break;
			case 'r':
				strcpy(buff, optarg);
				myb7_tree_del(pTree, buff, (int (*)(char *, char *))strcmp);
				break;
			case 'm' :
				optind = 1;
				usleep(100000);
				break;
			case 'e' :
				strcpy(buff, optarg);
				fprintf(stdout, "%s\n", buff);
				break;
			default: 
				break;
		}
		
	}
}
#endif

#ifdef _STRING_NOINPUT_
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
int wait_user = 0;
void myb7_tree_insert_wait(PMYB7_STRING_TREE pTree, char * key){
	if(wait_user)
		fprintf(stdout, "==================\ninsert : %s \n", key);
	myb7_tree_insert(pTree, key, (int (*)(char *, char *))strcmp);
	if(wait_user){
		printmyb7_tree(pTree);
		getchar();
	}
}

void myb7_tree_del_wait(PMYB7_STRING_TREE pTree, char *key){
	if(wait_user)
		fprintf(stdout, "==================\ndelete : %s \n", key);
	myb7_tree_del(pTree, key, (int (*)(char *, char *))strcmp);
	if(wait_user){
		printmyb7_tree(pTree);
		getchar();
	}
}

void usage(char *app) {
	fprintf(stdout, "%s [-(n)o wait user key| -(w)ait user key| -(h)elp/list case number -(m)emleak test]\n", app);
}
void assertion_found_and_wait(PMYB7_STRING_TREE pTree, char *in_key) {
	PMYB7_STRING_NODE found = NULL;
	if (myb7_tree_find(pTree, in_key, (int (*) (char *, char *))strcmp, &found) < 0){
		fprintf(stderr, "assertion found failed with key %s\n", in_key);
		exit(1);
	}
	if(wait_user){
		printmyb7_tree(pTree);
		getchar();
	}
}

void assertion_notfound_and_wait(PMYB7_STRING_TREE pTree, char *in_key) {
	PMYB7_STRING_NODE found = NULL;
	if (myb7_tree_find(pTree, in_key, (int (*) (char *, char *))strcmp, &found) >= 0){
		fprintf(stderr, "assertion notfound failed with key %s\n", in_key);
		exit(1);
	}
	if(wait_user){
		printmyb7_tree(pTree);
		getchar();
	}
}



int main (int argc, char **argv)
{
	PMYB7_STRING_TREE pTree = myb7tree_new ("first tree"); 
	
	static struct option long_options[]	 = {
		{"nowait", no_argument, 0, 'n'},
		{"wait", no_argument, 0, 'w'},
		{"memleak", no_argument, 0, 'm'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};
	
	int option_index = 0;
	int opt;
	int optlong;	

	while ((opt = getopt_long(argc, argv, "nwmh", long_options,&option_index))!= -1) {
		switch (opt) {
			case 'h':
				usage(argv[0]);
				exit(0);
				break;
			case 'n':
				wait_user = 0;
				break;
			case 'w':
				wait_user = 1;
				break;
			case 'm':
				wait_user = 0;
				optlong = opt;
				break;
			
		}
		
	}
	do{
		myb7_tree_insert_wait (pTree, "492"); 
		myb7_tree_insert_wait (pTree, "28");
		myb7_tree_insert_wait (pTree, "562");
		myb7_tree_insert_wait (pTree, "222");
		myb7_tree_insert_wait (pTree, "583");
		myb7_tree_insert_wait (pTree, "886");
		myb7_tree_insert_wait (pTree, "212");
		myb7_tree_insert_wait (pTree, "941");
		myb7_tree_insert_wait (pTree, "45");
		myb7_tree_insert_wait (pTree, "639");
		myb7_tree_insert_wait (pTree, "388");
		myb7_tree_insert_wait (pTree, "675");
		myb7_tree_insert_wait (pTree, "717");
		myb7_tree_insert_wait (pTree, "431");
		myb7_tree_insert_wait (pTree, "563");
		myb7_tree_insert_wait (pTree, "196");
		myb7_tree_insert_wait (pTree, "6");
		myb7_tree_insert_wait (pTree, "997");
		myb7_tree_insert_wait (pTree, "834");
		myb7_tree_insert_wait (pTree, "265");
		myb7_tree_insert_wait (pTree, "667");
		myb7_tree_insert_wait (pTree, "557");
		myb7_tree_insert_wait (pTree, "471");
		myb7_tree_insert_wait (pTree, "844");
		myb7_tree_insert_wait (pTree, "910");
		myb7_tree_insert_wait (pTree, "499");
		myb7_tree_insert_wait (pTree, "603");
		myb7_tree_insert_wait (pTree, "684");
		myb7_tree_insert_wait (pTree, "68");
		myb7_tree_insert_wait (pTree, "936");
		myb7_tree_insert_wait (pTree, "536");
		myb7_tree_insert_wait (pTree, "270");
		myb7_tree_insert_wait (pTree, "469");
		myb7_tree_insert_wait (pTree, "700");
		myb7_tree_insert_wait (pTree, "10");
		myb7_tree_insert_wait (pTree, "666");
		myb7_tree_insert_wait (pTree, "840");
		myb7_tree_insert_wait (pTree, "596");
		myb7_tree_insert_wait (pTree, "553");
		myb7_tree_insert_wait (pTree, "394");
		myb7_tree_insert_wait (pTree, "946");
		myb7_tree_insert_wait (pTree, "980");
		myb7_tree_insert_wait (pTree, "112");
//		assertion_find (pTree, "492");
		myb7_tree_insert_wait (pTree, "520");
		myb7_tree_insert_wait (pTree, "425");
		myb7_tree_insert_wait (pTree, "590");
		myb7_tree_insert_wait (pTree, "386");
		myb7_tree_insert_wait (pTree, "765");
		myb7_tree_insert_wait (pTree, "329");
		myb7_tree_insert_wait (pTree, "277");
		myb7_tree_insert_wait (pTree, "818");
		myb7_tree_insert_wait (pTree, "847");
		myb7_tree_insert_wait (pTree, "670");
		myb7_tree_insert_wait (pTree, "996");
		myb7_tree_insert_wait (pTree, "240");
		myb7_tree_insert_wait (pTree, "226");
		myb7_tree_insert_wait (pTree, "431");
		myb7_tree_insert_wait (pTree, "691");
		myb7_tree_insert_wait (pTree, "148");
		myb7_tree_insert_wait (pTree, "822");
		myb7_tree_insert_wait (pTree, "467");
		myb7_tree_insert_wait (pTree, "860");
		myb7_tree_insert_wait (pTree, "260");
		myb7_tree_insert_wait (pTree, "827");
		myb7_tree_insert_wait (pTree, "883");
		myb7_tree_insert_wait (pTree, "437");
		myb7_tree_insert_wait (pTree, "112");
		myb7_tree_insert_wait (pTree, "446");
		myb7_tree_insert_wait (pTree, "674");
		myb7_tree_insert_wait (pTree, "597");
		myb7_tree_insert_wait (pTree, "282");
		myb7_tree_insert_wait (pTree, "426");
		myb7_tree_insert_wait (pTree, "555");
		myb7_tree_insert_wait (pTree, "545");
		myb7_tree_insert_wait (pTree, "134");
		myb7_tree_insert_wait (pTree, "684");
		myb7_tree_insert_wait (pTree, "261");
		myb7_tree_insert_wait (pTree, "108");
		myb7_tree_insert_wait (pTree, "971");
		myb7_tree_insert_wait (pTree, "506");
		myb7_tree_insert_wait (pTree, "631");
		myb7_tree_insert_wait (pTree, "846");
		myb7_tree_insert_wait (pTree, "607");
		myb7_tree_insert_wait (pTree, "444");
		myb7_tree_insert_wait (pTree, "564");
		myb7_tree_insert_wait (pTree, "573");
		myb7_tree_insert_wait (pTree, "77");
		myb7_tree_insert_wait (pTree, "633");
		myb7_tree_insert_wait (pTree, "224");
		myb7_tree_insert_wait (pTree, "312");
		myb7_tree_insert_wait (pTree, "562");
		myb7_tree_insert_wait (pTree, "664");
		myb7_tree_insert_wait (pTree, "1000");
		myb7_tree_insert_wait (pTree, "846");
		myb7_tree_insert_wait (pTree, "714");
		myb7_tree_insert_wait (pTree, "437");
		myb7_tree_insert_wait (pTree, "787");
		myb7_tree_insert_wait (pTree, "539");
		myb7_tree_insert_wait (pTree, "164");
		myb7_tree_insert_wait (pTree, "248");
		myb7_tree_insert_wait (pTree, "41");
		myb7_tree_insert_wait (pTree, "760");
		myb7_tree_insert_wait (pTree, "291");
		myb7_tree_insert_wait (pTree, "895");
		myb7_tree_insert_wait (pTree, "284");
		myb7_tree_insert_wait (pTree, "26");
		myb7_tree_insert_wait (pTree, "737");
		myb7_tree_insert_wait (pTree, "779");
		myb7_tree_insert_wait (pTree, "501");
		myb7_tree_insert_wait (pTree, "125");
		myb7_tree_insert_wait (pTree, "923");
		myb7_tree_insert_wait (pTree, "186");
		myb7_tree_insert_wait (pTree, "281");
		myb7_tree_insert_wait (pTree, "34");
		myb7_tree_insert_wait (pTree, "367");
		myb7_tree_insert_wait (pTree, "955");
		myb7_tree_insert_wait (pTree, "348");
		myb7_tree_insert_wait (pTree, "789");
		myb7_tree_insert_wait (pTree, "71");
		myb7_tree_insert_wait (pTree, "584");
		myb7_tree_insert_wait (pTree, "336");
		myb7_tree_insert_wait (pTree, "739");
		myb7_tree_insert_wait (pTree, "973");
		myb7_tree_insert_wait (pTree, "563");
		myb7_tree_insert_wait (pTree, "59");
		myb7_tree_insert_wait (pTree, "175");
		myb7_tree_insert_wait (pTree, "272");
		myb7_tree_insert_wait (pTree, "539");
		myb7_tree_insert_wait (pTree, "498");
		myb7_tree_insert_wait (pTree, "428");
		myb7_tree_insert_wait (pTree, "980");
		myb7_tree_insert_wait (pTree, "170");
		myb7_tree_insert_wait (pTree, "476");
		myb7_tree_insert_wait (pTree, "265");
		myb7_tree_insert_wait (pTree, "158");
		myb7_tree_insert_wait (pTree, "529");
		myb7_tree_insert_wait (pTree, "548");
		myb7_tree_insert_wait (pTree, "612");
		myb7_tree_insert_wait (pTree, "880");
		myb7_tree_insert_wait (pTree, "315");
		myb7_tree_insert_wait (pTree, "867");
		myb7_tree_insert_wait (pTree, "889");
		myb7_tree_insert_wait (pTree, "843");
		myb7_tree_insert_wait (pTree, "350");
		myb7_tree_insert_wait (pTree, "709");
		myb7_tree_insert_wait (pTree, "602");
		myb7_tree_insert_wait (pTree, "883");
		myb7_tree_insert_wait (pTree, "788");
		myb7_tree_insert_wait (pTree, "453");
		myb7_tree_insert_wait (pTree, "356");
		myb7_tree_insert_wait (pTree, "635");
		myb7_tree_insert_wait (pTree, "796");
		myb7_tree_insert_wait (pTree, "680");
		myb7_tree_insert_wait (pTree, "798");
		myb7_tree_insert_wait (pTree, "291");
		myb7_tree_insert_wait (pTree, "543");
		myb7_tree_insert_wait (pTree, "701");
		myb7_tree_insert_wait (pTree, "868");
		myb7_tree_insert_wait (pTree, "583");
		myb7_tree_insert_wait (pTree, "328");
		myb7_tree_insert_wait (pTree, "976");
		myb7_tree_insert_wait (pTree, "715");
		myb7_tree_insert_wait (pTree, "11");
		myb7_tree_insert_wait (pTree, "970");
		myb7_tree_insert_wait (pTree, "741");
		myb7_tree_insert_wait (pTree, "570");
		myb7_tree_insert_wait (pTree, "673");
		myb7_tree_insert_wait (pTree, "233");
		myb7_tree_insert_wait (pTree, "768");
		myb7_tree_insert_wait (pTree, "1015");
		myb7_tree_insert_wait (pTree, "495");
		myb7_tree_insert_wait (pTree, "517");
		myb7_tree_insert_wait (pTree, "179");
		myb7_tree_insert_wait (pTree, "241");
		myb7_tree_insert_wait (pTree, "491");
		myb7_tree_insert_wait (pTree, "945");
		myb7_tree_insert_wait (pTree, "149");
		myb7_tree_insert_wait (pTree, "527");
		myb7_tree_insert_wait (pTree, "153");
		myb7_tree_insert_wait (pTree, "680");
		myb7_tree_insert_wait (pTree, "699");
		myb7_tree_insert_wait (pTree, "860");
		myb7_tree_insert_wait (pTree, "505");
		myb7_tree_insert_wait (pTree, "137");
		myb7_tree_insert_wait (pTree, "97");
		myb7_tree_insert_wait (pTree, "446");
		myb7_tree_insert_wait (pTree, "984");
		myb7_tree_insert_wait (pTree, "373");
		myb7_tree_insert_wait (pTree, "916");
		myb7_tree_insert_wait (pTree, "892");
		myb7_tree_insert_wait (pTree, "316");
		myb7_tree_insert_wait (pTree, "5");
		myb7_tree_insert_wait (pTree, "998");
		myb7_tree_insert_wait (pTree, "465");
		myb7_tree_insert_wait (pTree, "297");
		myb7_tree_insert_wait (pTree, "507");
		myb7_tree_insert_wait (pTree, "826");
		myb7_tree_insert_wait (pTree, "682");
		myb7_tree_insert_wait (pTree, "762");
		myb7_tree_insert_wait (pTree, "251");
		myb7_tree_insert_wait (pTree, "835");
		myb7_tree_insert_wait (pTree, "254");
		myb7_tree_insert_wait (pTree, "26");
		myb7_tree_insert_wait (pTree, "258");
		myb7_tree_insert_wait (pTree, "92");
		myb7_tree_insert_wait (pTree, "987");
		myb7_tree_insert_wait (pTree, "792");
		myb7_tree_insert_wait (pTree, "492");
		myb7_tree_insert_wait (pTree, "852");
		myb7_tree_insert_wait (pTree, "316");
		myb7_tree_insert_wait (pTree, "175");
		myb7_tree_insert_wait (pTree, "562");
		myb7_tree_insert_wait (pTree, "763");
		myb7_tree_insert_wait (pTree, "59");
		myb7_tree_insert_wait (pTree, "176");
		myb7_tree_insert_wait (pTree, "748");
		myb7_tree_insert_wait (pTree, "554");
		myb7_tree_insert_wait (pTree, "92");
		myb7_tree_insert_wait (pTree, "56");
		myb7_tree_insert_wait (pTree, "911");
		myb7_tree_insert_wait (pTree, "918");
		myb7_tree_insert_wait (pTree, "875");
		myb7_tree_insert_wait (pTree, "732");
		myb7_tree_insert_wait (pTree, "490");
		myb7_tree_insert_wait (pTree, "663");
		myb7_tree_insert_wait (pTree, "638");
		myb7_tree_insert_wait (pTree, "769");
		myb7_tree_insert_wait (pTree, "574");
		myb7_tree_insert_wait (pTree, "871");
		myb7_tree_insert_wait (pTree, "508");
		myb7_tree_insert_wait (pTree, "225");
		myb7_tree_insert_wait (pTree, "577");
		myb7_tree_insert_wait (pTree, "161");
		myb7_tree_insert_wait (pTree, "62");
		myb7_tree_insert_wait (pTree, "489");
		myb7_tree_insert_wait (pTree, "624");
		myb7_tree_insert_wait (pTree, "135");
		myb7_tree_insert_wait (pTree, "476");
		myb7_tree_insert_wait (pTree, "317");
		myb7_tree_insert_wait (pTree, "289");
		myb7_tree_insert_wait (pTree, "577");
		myb7_tree_insert_wait (pTree, "116");
		myb7_tree_insert_wait (pTree, "386");
		myb7_tree_insert_wait (pTree, "14");
		myb7_tree_insert_wait (pTree, "529");
		myb7_tree_insert_wait (pTree, "84");
		myb7_tree_insert_wait (pTree, "692");
		myb7_tree_insert_wait (pTree, "667");
		myb7_tree_insert_wait (pTree, "198");
		myb7_tree_insert_wait (pTree, "205");
		myb7_tree_insert_wait (pTree, "178");
		myb7_tree_insert_wait (pTree, "427");
		myb7_tree_insert_wait (pTree, "212");
		myb7_tree_insert_wait (pTree, "94");
		myb7_tree_insert_wait (pTree, "745");
		myb7_tree_insert_wait (pTree, "88");
		myb7_tree_insert_wait (pTree, "791");
		myb7_tree_insert_wait (pTree, "261");
		myb7_tree_insert_wait (pTree, "349");
		myb7_tree_insert_wait (pTree, "293");
		myb7_tree_insert_wait (pTree, "346");
		myb7_tree_insert_wait (pTree, "163");
		myb7_tree_insert_wait (pTree, "88");
		myb7_tree_insert_wait (pTree, "561");
		myb7_tree_insert_wait (pTree, "951");
		myb7_tree_insert_wait (pTree, "13");
		myb7_tree_insert_wait (pTree, "306");
		myb7_tree_insert_wait (pTree, "870");
		myb7_tree_insert_wait (pTree, "922");
		myb7_tree_insert_wait (pTree, "201");
		myb7_tree_insert_wait (pTree, "927");
		myb7_tree_insert_wait (pTree, "563");
		myb7_tree_insert_wait (pTree, "611");
		myb7_tree_insert_wait (pTree, "302");
		myb7_tree_insert_wait (pTree, "346");
		myb7_tree_insert_wait (pTree, "886");
		myb7_tree_insert_wait (pTree, "909");
		myb7_tree_insert_wait (pTree, "487");
		myb7_tree_insert_wait (pTree, "488");
		myb7_tree_insert_wait (pTree, "795");
		myb7_tree_insert_wait (pTree, "1016");
		myb7_tree_insert_wait (pTree, "84");
		myb7_tree_insert_wait (pTree, "706");
		myb7_tree_insert_wait (pTree, "124");
		myb7_tree_insert_wait (pTree, "326");
		myb7_tree_insert_wait (pTree, "483");
		myb7_tree_insert_wait (pTree, "763");
		myb7_tree_insert_wait (pTree, "498");
		myb7_tree_insert_wait (pTree, "939");
		myb7_tree_insert_wait (pTree, "186");
		myb7_tree_insert_wait (pTree, "205");
		myb7_tree_insert_wait (pTree, "809");
		myb7_tree_insert_wait (pTree, "236");
		myb7_tree_insert_wait (pTree, "74");
		myb7_tree_insert_wait (pTree, "255");
		myb7_tree_insert_wait (pTree, "81");
		myb7_tree_insert_wait (pTree, "115");
		myb7_tree_insert_wait (pTree, "105");
		myb7_tree_insert_wait (pTree, "966");
		myb7_tree_insert_wait (pTree, "359");
		
		assertion_found_and_wait (pTree, "492"); 
		assertion_found_and_wait (pTree, "28");
		assertion_found_and_wait (pTree, "562");
		assertion_found_and_wait (pTree, "222");
		assertion_found_and_wait (pTree, "583");
		assertion_found_and_wait (pTree, "886");
		assertion_found_and_wait (pTree, "212");
		assertion_found_and_wait (pTree, "941");
		assertion_found_and_wait (pTree, "45");
		assertion_found_and_wait (pTree, "639");
		assertion_found_and_wait (pTree, "388");
		assertion_found_and_wait (pTree, "675");
		assertion_found_and_wait (pTree, "717");
		assertion_found_and_wait (pTree, "431");
		assertion_found_and_wait (pTree, "563");
		assertion_found_and_wait (pTree, "196");
		assertion_found_and_wait (pTree, "6");
		assertion_found_and_wait (pTree, "997");
		assertion_found_and_wait (pTree, "834");
		assertion_found_and_wait (pTree, "265");
		assertion_found_and_wait (pTree, "667");
		assertion_found_and_wait (pTree, "557");
		assertion_found_and_wait (pTree, "471");
		assertion_found_and_wait (pTree, "844");
		assertion_found_and_wait (pTree, "910");
		assertion_found_and_wait (pTree, "499");
		assertion_found_and_wait (pTree, "603");
		assertion_found_and_wait (pTree, "684");
		assertion_found_and_wait (pTree, "68");
		assertion_found_and_wait (pTree, "936");
		assertion_found_and_wait (pTree, "536");
		assertion_found_and_wait (pTree, "270");
		assertion_found_and_wait (pTree, "469");
		assertion_found_and_wait (pTree, "700");
		assertion_found_and_wait (pTree, "10");
		assertion_found_and_wait (pTree, "666");
		assertion_found_and_wait (pTree, "840");
		assertion_found_and_wait (pTree, "596");
		assertion_found_and_wait (pTree, "553");
		assertion_found_and_wait (pTree, "394");
		assertion_found_and_wait (pTree, "946");
		assertion_found_and_wait (pTree, "980");
		assertion_found_and_wait (pTree, "112");
		assertion_found_and_wait (pTree, "520");
		assertion_found_and_wait (pTree, "425");
		assertion_found_and_wait (pTree, "590");
		assertion_found_and_wait (pTree, "386");
		assertion_found_and_wait (pTree, "765");
		assertion_found_and_wait (pTree, "329");
		assertion_found_and_wait (pTree, "277");
		assertion_found_and_wait (pTree, "818");
		assertion_found_and_wait (pTree, "847");
		assertion_found_and_wait (pTree, "670");
		assertion_found_and_wait (pTree, "996");
		assertion_found_and_wait (pTree, "240");
		assertion_found_and_wait (pTree, "226");
		assertion_found_and_wait (pTree, "431");
		assertion_found_and_wait (pTree, "691");
		assertion_found_and_wait (pTree, "148");
		assertion_found_and_wait (pTree, "822");
		assertion_found_and_wait (pTree, "467");
		assertion_found_and_wait (pTree, "860");
		assertion_found_and_wait (pTree, "260");
		assertion_found_and_wait (pTree, "827");
		assertion_found_and_wait (pTree, "883");
		assertion_found_and_wait (pTree, "437");
		assertion_found_and_wait (pTree, "112");
		assertion_found_and_wait (pTree, "446");
		assertion_found_and_wait (pTree, "674");
		assertion_found_and_wait (pTree, "597");
		assertion_found_and_wait (pTree, "282");
		assertion_found_and_wait (pTree, "426");
		assertion_found_and_wait (pTree, "555");
		assertion_found_and_wait (pTree, "545");
		assertion_found_and_wait (pTree, "134");
		assertion_found_and_wait (pTree, "684");
		assertion_found_and_wait (pTree, "261");
		assertion_found_and_wait (pTree, "108");
		assertion_found_and_wait (pTree, "971");
		assertion_found_and_wait (pTree, "506");
		assertion_found_and_wait (pTree, "631");
		assertion_found_and_wait (pTree, "846");
		assertion_found_and_wait (pTree, "607");
		assertion_found_and_wait (pTree, "444");
		assertion_found_and_wait (pTree, "564");
		assertion_found_and_wait (pTree, "573");
		assertion_found_and_wait (pTree, "77");
		assertion_found_and_wait (pTree, "633");
		assertion_found_and_wait (pTree, "224");
		assertion_found_and_wait (pTree, "312");
		assertion_found_and_wait (pTree, "562");
		assertion_found_and_wait (pTree, "664");
		assertion_found_and_wait (pTree, "1000");
		assertion_found_and_wait (pTree, "846");
		assertion_found_and_wait (pTree, "714");
		assertion_found_and_wait (pTree, "437");
		assertion_found_and_wait (pTree, "787");
		assertion_found_and_wait (pTree, "539");
		assertion_found_and_wait (pTree, "164");
		assertion_found_and_wait (pTree, "248");
		assertion_found_and_wait (pTree, "41");
		assertion_found_and_wait (pTree, "760");
		assertion_found_and_wait (pTree, "291");
		assertion_found_and_wait (pTree, "895");
		assertion_found_and_wait (pTree, "284");
		assertion_found_and_wait (pTree, "26");
		assertion_found_and_wait (pTree, "737");
		assertion_found_and_wait (pTree, "779");
		assertion_found_and_wait (pTree, "501");
		assertion_found_and_wait (pTree, "125");
		assertion_found_and_wait (pTree, "923");
		assertion_found_and_wait (pTree, "186");
		assertion_found_and_wait (pTree, "281");
		assertion_found_and_wait (pTree, "34");
		assertion_found_and_wait (pTree, "367");
		assertion_found_and_wait (pTree, "955");
		assertion_found_and_wait (pTree, "348");
		assertion_found_and_wait (pTree, "789");
		assertion_found_and_wait (pTree, "71");
		assertion_found_and_wait (pTree, "584");
		assertion_found_and_wait (pTree, "336");
		assertion_found_and_wait (pTree, "739");
		assertion_found_and_wait (pTree, "973");
		assertion_found_and_wait (pTree, "563");
		assertion_found_and_wait (pTree, "59");
		assertion_found_and_wait (pTree, "175");
		assertion_found_and_wait (pTree, "272");
		assertion_found_and_wait (pTree, "539");
		assertion_found_and_wait (pTree, "498");
		assertion_found_and_wait (pTree, "428");
		assertion_found_and_wait (pTree, "980");
		assertion_found_and_wait (pTree, "170");
		assertion_found_and_wait (pTree, "476");
		assertion_found_and_wait (pTree, "265");
		assertion_found_and_wait (pTree, "158");
		assertion_found_and_wait (pTree, "529");
		assertion_found_and_wait (pTree, "548");
		assertion_found_and_wait (pTree, "612");
		assertion_found_and_wait (pTree, "880");
		assertion_found_and_wait (pTree, "315");
		assertion_found_and_wait (pTree, "867");
		assertion_found_and_wait (pTree, "889");
		assertion_found_and_wait (pTree, "843");
		assertion_found_and_wait (pTree, "350");
		assertion_found_and_wait (pTree, "709");
		assertion_found_and_wait (pTree, "602");
		assertion_found_and_wait (pTree, "883");
		assertion_found_and_wait (pTree, "788");
		assertion_found_and_wait (pTree, "453");
		assertion_found_and_wait (pTree, "356");
		assertion_found_and_wait (pTree, "635");
		assertion_found_and_wait (pTree, "796");
		assertion_found_and_wait (pTree, "680");
		assertion_found_and_wait (pTree, "798");
		assertion_found_and_wait (pTree, "291");
		assertion_found_and_wait (pTree, "543");
		assertion_found_and_wait (pTree, "701");
		assertion_found_and_wait (pTree, "868");
		assertion_found_and_wait (pTree, "583");
		assertion_found_and_wait (pTree, "328");
		assertion_found_and_wait (pTree, "976");
		assertion_found_and_wait (pTree, "715");
		assertion_found_and_wait (pTree, "11");
		assertion_found_and_wait (pTree, "970");
		assertion_found_and_wait (pTree, "741");
		assertion_found_and_wait (pTree, "570");
		assertion_found_and_wait (pTree, "673");
		assertion_found_and_wait (pTree, "233");
		assertion_found_and_wait (pTree, "768");
		assertion_found_and_wait (pTree, "1015");
		assertion_found_and_wait (pTree, "495");
		assertion_found_and_wait (pTree, "517");
		assertion_found_and_wait (pTree, "179");
		assertion_found_and_wait (pTree, "241");
		assertion_found_and_wait (pTree, "491");
		assertion_found_and_wait (pTree, "945");
		assertion_found_and_wait (pTree, "149");
		assertion_found_and_wait (pTree, "527");
		assertion_found_and_wait (pTree, "153");
		assertion_found_and_wait (pTree, "680");
		assertion_found_and_wait (pTree, "699");
		assertion_found_and_wait (pTree, "860");
		assertion_found_and_wait (pTree, "505");
		assertion_found_and_wait (pTree, "137");
		assertion_found_and_wait (pTree, "97");
		assertion_found_and_wait (pTree, "446");
		assertion_found_and_wait (pTree, "984");
		assertion_found_and_wait (pTree, "373");
		assertion_found_and_wait (pTree, "916");
		assertion_found_and_wait (pTree, "892");
		assertion_found_and_wait (pTree, "316");
		assertion_found_and_wait (pTree, "5");
		assertion_found_and_wait (pTree, "998");
		assertion_found_and_wait (pTree, "465");
		assertion_found_and_wait (pTree, "297");
		assertion_found_and_wait (pTree, "507");
		assertion_found_and_wait (pTree, "826");
		assertion_found_and_wait (pTree, "682");
		assertion_found_and_wait (pTree, "762");
		assertion_found_and_wait (pTree, "251");
		assertion_found_and_wait (pTree, "835");
		assertion_found_and_wait (pTree, "254");
		assertion_found_and_wait (pTree, "26");
		assertion_found_and_wait (pTree, "258");
		assertion_found_and_wait (pTree, "92");
		assertion_found_and_wait (pTree, "987");
		assertion_found_and_wait (pTree, "792");
		assertion_found_and_wait (pTree, "492");
		assertion_found_and_wait (pTree, "852");
		assertion_found_and_wait (pTree, "316");
		assertion_found_and_wait (pTree, "175");
		assertion_found_and_wait (pTree, "562");
		assertion_found_and_wait (pTree, "763");
		assertion_found_and_wait (pTree, "59");
		assertion_found_and_wait (pTree, "176");
		assertion_found_and_wait (pTree, "748");
		assertion_found_and_wait (pTree, "554");
		assertion_found_and_wait (pTree, "92");
		assertion_found_and_wait (pTree, "56");
		assertion_found_and_wait (pTree, "911");
		assertion_found_and_wait (pTree, "918");
		assertion_found_and_wait (pTree, "875");
		assertion_found_and_wait (pTree, "732");
		assertion_found_and_wait (pTree, "490");
		assertion_found_and_wait (pTree, "663");
		assertion_found_and_wait (pTree, "638");
		assertion_found_and_wait (pTree, "769");
		assertion_found_and_wait (pTree, "574");
		assertion_found_and_wait (pTree, "871");
		assertion_found_and_wait (pTree, "508");
		assertion_found_and_wait (pTree, "225");
		assertion_found_and_wait (pTree, "577");
		assertion_found_and_wait (pTree, "161");
		assertion_found_and_wait (pTree, "62");
		assertion_found_and_wait (pTree, "489");
		assertion_found_and_wait (pTree, "624");
		assertion_found_and_wait (pTree, "135");
		assertion_found_and_wait (pTree, "476");
		assertion_found_and_wait (pTree, "317");
		assertion_found_and_wait (pTree, "289");
		assertion_found_and_wait (pTree, "577");
		assertion_found_and_wait (pTree, "116");
		assertion_found_and_wait (pTree, "386");
		assertion_found_and_wait (pTree, "14");
		assertion_found_and_wait (pTree, "529");
		assertion_found_and_wait (pTree, "84");
		assertion_found_and_wait (pTree, "692");
		assertion_found_and_wait (pTree, "667");
		assertion_found_and_wait (pTree, "198");
		assertion_found_and_wait (pTree, "205");
		assertion_found_and_wait (pTree, "178");
		assertion_found_and_wait (pTree, "427");
		assertion_found_and_wait (pTree, "212");
		assertion_found_and_wait (pTree, "94");
		assertion_found_and_wait (pTree, "745");
		assertion_found_and_wait (pTree, "88");
		assertion_found_and_wait (pTree, "791");
		assertion_found_and_wait (pTree, "261");
		assertion_found_and_wait (pTree, "349");
		assertion_found_and_wait (pTree, "293");
		assertion_found_and_wait (pTree, "346");
		assertion_found_and_wait (pTree, "163");
		assertion_found_and_wait (pTree, "88");
		assertion_found_and_wait (pTree, "561");
		assertion_found_and_wait (pTree, "951");
		assertion_found_and_wait (pTree, "13");
		assertion_found_and_wait (pTree, "306");
		assertion_found_and_wait (pTree, "870");
		assertion_found_and_wait (pTree, "922");
		assertion_found_and_wait (pTree, "201");
		assertion_found_and_wait (pTree, "927");
		assertion_found_and_wait (pTree, "563");
		assertion_found_and_wait (pTree, "611");
		assertion_found_and_wait (pTree, "302");
		assertion_found_and_wait (pTree, "346");
		assertion_found_and_wait (pTree, "886");
		assertion_found_and_wait (pTree, "909");
		assertion_found_and_wait (pTree, "487");
		assertion_found_and_wait (pTree, "488");
		assertion_found_and_wait (pTree, "795");
		assertion_found_and_wait (pTree, "1016");
		assertion_found_and_wait (pTree, "84");
		assertion_found_and_wait (pTree, "706");
		assertion_found_and_wait (pTree, "124");
		assertion_found_and_wait (pTree, "326");
		assertion_found_and_wait (pTree, "483");
		assertion_found_and_wait (pTree, "763");
		assertion_found_and_wait (pTree, "498");
		assertion_found_and_wait (pTree, "939");
		assertion_found_and_wait (pTree, "186");
		assertion_found_and_wait (pTree, "205");
		assertion_found_and_wait (pTree, "809");
		assertion_found_and_wait (pTree, "236");
		assertion_found_and_wait (pTree, "74");
		assertion_found_and_wait (pTree, "255");
		assertion_found_and_wait (pTree, "81");
		assertion_found_and_wait (pTree, "115");
		assertion_found_and_wait (pTree, "105");
		assertion_found_and_wait (pTree, "966");
		assertion_found_and_wait (pTree, "359");

		myb7_tree_del_wait (pTree, "557");
		assertion_notfound_and_wait (pTree, "557");
		assertion_found_and_wait (pTree, "492"); 
		myb7_tree_del_wait (pTree, "492");
		assertion_notfound_and_wait (pTree, "492"); 
		myb7_tree_del_wait (pTree, "222");
		assertion_notfound_and_wait (pTree, "222"); 
		myb7_tree_del_wait (pTree, "196");
		assertion_notfound_and_wait (pTree, "196"); 
		myb7_tree_del_wait (pTree, "6");
		assertion_notfound_and_wait (pTree, "6"); 
		myb7_tree_del_wait (pTree, "28");
		assertion_notfound_and_wait (pTree, "28"); 
		myb7_tree_del_wait (pTree, "562");
		assertion_notfound_and_wait (pTree, "562"); 
		myb7_tree_del_wait (pTree, "45");
		assertion_notfound_and_wait (pTree, "45"); 
		myb7_tree_del_wait (pTree, "639");
		assertion_notfound_and_wait (pTree, "639"); 
		myb7_tree_del_wait (pTree, "388");
		assertion_notfound_and_wait (pTree, "388"); 
		myb7_tree_del_wait (pTree, "684");
		assertion_notfound_and_wait (pTree, "684"); 
		myb7_tree_del_wait (pTree, "68");
		assertion_notfound_and_wait (pTree, "68"); 
		myb7_tree_del_wait (pTree, "603");
		assertion_notfound_and_wait (pTree, "603"); 
		myb7_tree_del_wait (pTree, "557");
		myb7_tree_del_wait (pTree, "471");
		myb7_tree_del_wait (pTree, "270");
		myb7_tree_del_wait (pTree, "666");
		myb7_tree_del_wait (pTree, "469");
		myb7_tree_del_wait (pTree, "700");
		myb7_tree_del_wait (pTree, "10");
		myb7_tree_del_wait (pTree, "431");
		myb7_tree_del_wait (pTree, "860");
		myb7_tree_del_wait (pTree, "611");
		myb7_tree_del_wait (pTree, "563");
		myb7_tree_del_wait (pTree, "927");
		myb7_tree_del_wait (pTree, "201");
		myb7_tree_del_wait (pTree, "922");
		myb7_tree_del_wait (pTree, "870");
		myb7_tree_del_wait (pTree, "306");
		myb7_tree_del_wait (pTree, "13");
		myb7_tree_del_wait (pTree, "951");
		myb7_tree_del_wait (pTree, "561");
		myb7_tree_del_wait (pTree, "88");
		myb7_tree_del_wait (pTree, "163");
		myb7_tree_del_wait (pTree, "346");
		myb7_tree_del_wait (pTree, "293");
		myb7_tree_del_wait (pTree, "349");
		myb7_tree_del_wait (pTree, "261");
		myb7_tree_del_wait (pTree, "791");
		myb7_tree_del_wait (pTree, "88");
		myb7_tree_del_wait (pTree, "745");
		myb7_tree_del_wait (pTree, "94");
		myb7_tree_del_wait (pTree, "212");
		myb7_tree_del_wait (pTree, "427");
		myb7_tree_del_wait (pTree, "178");
		myb7_tree_del_wait (pTree, "205");
		myb7_tree_del_wait (pTree, "198");
		myb7_tree_del_wait (pTree, "667");
		myb7_tree_del_wait (pTree, "692");
		myb7_tree_del_wait (pTree, "84");
		myb7_tree_del_wait (pTree, "529");
		myb7_tree_del_wait (pTree, "14");
		myb7_tree_del_wait (pTree, "386");
		myb7_tree_del_wait (pTree, "116");
		myb7_tree_del_wait (pTree, "577");
		myb7_tree_del_wait (pTree, "359");
		myb7_tree_del_wait (pTree, "966");
		myb7_tree_del_wait (pTree, "105");
		myb7_tree_del_wait (pTree, "115");
		myb7_tree_del_wait (pTree, "81");
		myb7_tree_del_wait (pTree, "255");
		myb7_tree_del_wait (pTree, "74");
		myb7_tree_del_wait (pTree, "236");
		myb7_tree_del_wait (pTree, "809");
		myb7_tree_del_wait (pTree, "205");
		myb7_tree_del_wait (pTree, "186");
		myb7_tree_del_wait (pTree, "939");
		myb7_tree_del_wait (pTree, "498");
		myb7_tree_del_wait (pTree, "763");
		myb7_tree_del_wait (pTree, "483");
		myb7_tree_del_wait (pTree, "326");
		myb7_tree_del_wait (pTree, "124");
		myb7_tree_del_wait (pTree, "706");
		myb7_tree_del_wait (pTree, "84");
		myb7_tree_del_wait (pTree, "1016");
		myb7_tree_del_wait (pTree, "795");
		myb7_tree_del_wait (pTree, "488");
		myb7_tree_del_wait (pTree, "487");
		myb7_tree_del_wait (pTree, "909");
		myb7_tree_del_wait (pTree, "886");
		myb7_tree_del_wait (pTree, "346");
		myb7_tree_del_wait (pTree, "302");
		myb7_tree_del_wait (pTree, "289");
		myb7_tree_del_wait (pTree, "317");
		myb7_tree_del_wait (pTree, "476");
		myb7_tree_del_wait (pTree, "135");
		myb7_tree_del_wait (pTree, "624");
		myb7_tree_del_wait (pTree, "489");
		myb7_tree_del_wait (pTree, "62");
		myb7_tree_del_wait (pTree, "161");
		myb7_tree_del_wait (pTree, "577");
		myb7_tree_del_wait (pTree, "225");
		myb7_tree_del_wait (pTree, "508");
		myb7_tree_del_wait (pTree, "871");
		myb7_tree_del_wait (pTree, "574");
		myb7_tree_del_wait (pTree, "769");
	// 	printmyb7_tree (pTree);	    //check switch
		myb7_tree_del_wait (pTree, "638");
		myb7_tree_del_wait (pTree, "663");
		myb7_tree_del_wait (pTree, "490");
		myb7_tree_del_wait (pTree, "732");
		myb7_tree_del_wait (pTree, "875");
		myb7_tree_del_wait (pTree, "918");
		myb7_tree_del_wait (pTree, "911");
		myb7_tree_del_wait (pTree, "56");
		myb7_tree_del_wait (pTree, "92");
		myb7_tree_del_wait (pTree, "554");
		myb7_tree_del_wait (pTree, "748");
		myb7_tree_del_wait (pTree, "176");
		myb7_tree_del_wait (pTree, "59");
		myb7_tree_del_wait (pTree, "763");
		myb7_tree_del_wait (pTree, "562");
		myb7_tree_del_wait (pTree, "175");
		myb7_tree_del_wait (pTree, "316");
		myb7_tree_del_wait (pTree, "852");
		myb7_tree_del_wait (pTree, "792");
		myb7_tree_del_wait (pTree, "987");
		myb7_tree_del_wait (pTree, "92");
		myb7_tree_del_wait (pTree, "258");
		myb7_tree_del_wait (pTree, "26");
		myb7_tree_del_wait (pTree, "254");
		myb7_tree_del_wait (pTree, "835");
		myb7_tree_del_wait (pTree, "251");
		myb7_tree_del_wait (pTree, "762");
		myb7_tree_del_wait (pTree, "682");
		myb7_tree_del_wait (pTree, "826");
		myb7_tree_del_wait (pTree, "507");
		myb7_tree_del_wait (pTree, "297");
		myb7_tree_del_wait (pTree, "465");
		myb7_tree_del_wait (pTree, "998");
		myb7_tree_del_wait (pTree, "5");
		myb7_tree_del_wait (pTree, "316");
		myb7_tree_del_wait (pTree, "892");
		myb7_tree_del_wait (pTree, "916");
		myb7_tree_del_wait (pTree, "373");
		myb7_tree_del_wait (pTree, "984");
		myb7_tree_del_wait (pTree, "446");
		myb7_tree_del_wait (pTree, "97");
		myb7_tree_del_wait (pTree, "137");
		myb7_tree_del_wait (pTree, "505");
		myb7_tree_del_wait (pTree, "699");
		myb7_tree_del_wait (pTree, "680");
		myb7_tree_del_wait (pTree, "153");
		myb7_tree_del_wait (pTree, "527");
		myb7_tree_del_wait (pTree, "149");
		myb7_tree_del_wait (pTree, "945");
		myb7_tree_del_wait (pTree, "491");
		myb7_tree_del_wait (pTree, "241");
		myb7_tree_del_wait (pTree, "179");
		myb7_tree_del_wait (pTree, "517");
		myb7_tree_del_wait (pTree, "495");
		myb7_tree_del_wait (pTree, "1015");
		myb7_tree_del_wait (pTree, "768");
		myb7_tree_del_wait (pTree, "233");
		myb7_tree_del_wait (pTree, "673");
		myb7_tree_del_wait (pTree, "570");
		myb7_tree_del_wait (pTree, "741");
		myb7_tree_del_wait (pTree, "970");
		myb7_tree_del_wait (pTree, "11");
		myb7_tree_del_wait (pTree, "715");
		myb7_tree_del_wait (pTree, "976");
		myb7_tree_del_wait (pTree, "328");
		myb7_tree_del_wait (pTree, "583");
		myb7_tree_del_wait (pTree, "868");
		myb7_tree_del_wait (pTree, "701");
		myb7_tree_del_wait (pTree, "543");
		myb7_tree_del_wait (pTree, "291");
		myb7_tree_del_wait (pTree, "798");
		myb7_tree_del_wait (pTree, "680");
		myb7_tree_del_wait (pTree, "796");
		myb7_tree_del_wait (pTree, "635");
		myb7_tree_del_wait (pTree, "356");
		myb7_tree_del_wait (pTree, "453");
		myb7_tree_del_wait (pTree, "788");
		myb7_tree_del_wait (pTree, "883");
		myb7_tree_del_wait (pTree, "602");
		myb7_tree_del_wait (pTree, "709");
		myb7_tree_del_wait (pTree, "350");
		myb7_tree_del_wait (pTree, "843");
		myb7_tree_del_wait (pTree, "889");
		myb7_tree_del_wait (pTree, "867");
		myb7_tree_del_wait (pTree, "315");
		myb7_tree_del_wait (pTree, "880");
		myb7_tree_del_wait (pTree, "612");
		myb7_tree_del_wait (pTree, "548");
		myb7_tree_del_wait (pTree, "529");
		myb7_tree_del_wait (pTree, "158");
		myb7_tree_del_wait (pTree, "265");
		myb7_tree_del_wait (pTree, "476");
		myb7_tree_del_wait (pTree, "170");
		myb7_tree_del_wait (pTree, "980");
		myb7_tree_del_wait (pTree, "428");
		myb7_tree_del_wait (pTree, "498");
		myb7_tree_del_wait (pTree, "539");
		myb7_tree_del_wait (pTree, "272");
		myb7_tree_del_wait (pTree, "175");
		myb7_tree_del_wait (pTree, "59");
		myb7_tree_del_wait (pTree, "563");
		myb7_tree_del_wait (pTree, "973");
		myb7_tree_del_wait (pTree, "739");
		myb7_tree_del_wait (pTree, "336");
		myb7_tree_del_wait (pTree, "584");
		myb7_tree_del_wait (pTree, "71");
		myb7_tree_del_wait (pTree, "789");
		myb7_tree_del_wait (pTree, "348");
		myb7_tree_del_wait (pTree, "955");
		myb7_tree_del_wait (pTree, "367");
		myb7_tree_del_wait (pTree, "34");
		myb7_tree_del_wait (pTree, "281");
		myb7_tree_del_wait (pTree, "186");
		myb7_tree_del_wait (pTree, "923");
		myb7_tree_del_wait (pTree, "125");
		myb7_tree_del_wait (pTree, "501");
		myb7_tree_del_wait (pTree, "779");
		myb7_tree_del_wait (pTree, "737");
		myb7_tree_del_wait (pTree, "26"); 
		myb7_tree_del_wait (pTree, "284");
		myb7_tree_del_wait (pTree, "895");
		myb7_tree_del_wait (pTree, "291");
		myb7_tree_del_wait (pTree, "760");
		myb7_tree_del_wait (pTree, "41");
		myb7_tree_del_wait (pTree, "248");
		myb7_tree_del_wait (pTree, "164");
		myb7_tree_del_wait (pTree, "539");
		myb7_tree_del_wait (pTree, "787");
		myb7_tree_del_wait (pTree, "437");
		myb7_tree_del_wait (pTree, "714");
		myb7_tree_del_wait (pTree, "846");
		myb7_tree_del_wait (pTree, "1000");
		myb7_tree_del_wait (pTree, "664");
		myb7_tree_del_wait (pTree, "562");
		myb7_tree_del_wait (pTree, "312");
		myb7_tree_del_wait (pTree, "224");
		myb7_tree_del_wait (pTree, "633");
		myb7_tree_del_wait (pTree, "77");
		myb7_tree_del_wait (pTree, "573");
		myb7_tree_del_wait (pTree, "564");
		myb7_tree_del_wait (pTree, "444");
		myb7_tree_del_wait (pTree, "607");
		myb7_tree_del_wait (pTree, "846");
		myb7_tree_del_wait (pTree, "631");
		myb7_tree_del_wait (pTree, "506");
		myb7_tree_del_wait (pTree, "971");
		myb7_tree_del_wait (pTree, "108");
		myb7_tree_del_wait (pTree, "261");
		myb7_tree_del_wait (pTree, "684");
		myb7_tree_del_wait (pTree, "134");
		myb7_tree_del_wait (pTree, "545");
		myb7_tree_del_wait (pTree, "555");
		myb7_tree_del_wait (pTree, "426");
		myb7_tree_del_wait (pTree, "282");
		myb7_tree_del_wait (pTree, "597");
		myb7_tree_del_wait (pTree, "674");
		myb7_tree_del_wait (pTree, "446");
		myb7_tree_del_wait (pTree, "112");
		myb7_tree_del_wait (pTree, "437");
		myb7_tree_del_wait (pTree, "883");
		myb7_tree_del_wait (pTree, "827");
		myb7_tree_del_wait (pTree, "260");
		myb7_tree_del_wait (pTree, "860");
		myb7_tree_del_wait (pTree, "467");
		myb7_tree_del_wait (pTree, "822");
		myb7_tree_del_wait (pTree, "148");
		myb7_tree_del_wait (pTree, "691");
		myb7_tree_del_wait (pTree, "226");
		myb7_tree_del_wait (pTree, "240");
		myb7_tree_del_wait (pTree, "996");
		myb7_tree_del_wait (pTree, "670");
		myb7_tree_del_wait (pTree, "847");
		myb7_tree_del_wait (pTree, "818");
		myb7_tree_del_wait (pTree, "277");
		myb7_tree_del_wait (pTree, "329");
		myb7_tree_del_wait (pTree, "765");
		myb7_tree_del_wait (pTree, "386");
		myb7_tree_del_wait (pTree, "590");
		myb7_tree_del_wait (pTree, "425");
		myb7_tree_del_wait (pTree, "520");
		myb7_tree_del_wait (pTree, "112");
		myb7_tree_del_wait (pTree, "980");
		myb7_tree_del_wait (pTree, "946");
		myb7_tree_del_wait (pTree, "394");
		myb7_tree_del_wait (pTree, "553");
		myb7_tree_del_wait (pTree, "596");
		myb7_tree_del_wait (pTree, "840");
		myb7_tree_del_wait (pTree, "666");
		myb7_tree_del_wait (pTree, "10");
		myb7_tree_del_wait (pTree, "700");
		myb7_tree_del_wait (pTree, "469");
		myb7_tree_del_wait (pTree, "270");
		myb7_tree_del_wait (pTree, "536");
		myb7_tree_del_wait (pTree, "936");
		myb7_tree_del_wait (pTree, "68");
		myb7_tree_del_wait (pTree, "684");
		myb7_tree_del_wait (pTree, "603");
		myb7_tree_del_wait (pTree, "499");
		myb7_tree_del_wait (pTree, "910");
		myb7_tree_del_wait (pTree, "844");
		myb7_tree_del_wait (pTree, "471");
		myb7_tree_del_wait (pTree, "667");
		myb7_tree_del_wait (pTree, "265");
		myb7_tree_del_wait (pTree, "834");
		myb7_tree_del_wait (pTree, "997");
		myb7_tree_del_wait (pTree, "6");
		myb7_tree_del_wait (pTree, "196");
		myb7_tree_del_wait (pTree, "563");
		myb7_tree_del_wait (pTree, "431");
		myb7_tree_del_wait (pTree, "717");
		myb7_tree_del_wait (pTree, "675");
		myb7_tree_del_wait (pTree, "388");
		myb7_tree_del_wait (pTree, "639");
		myb7_tree_del_wait (pTree, "45");
		myb7_tree_del_wait (pTree, "941");
		myb7_tree_del_wait (pTree, "212");
		myb7_tree_del_wait (pTree, "886");
		myb7_tree_del_wait (pTree, "583");
		myb7_tree_del_wait (pTree, "222");
		myb7_tree_del_wait (pTree, "562");
		myb7_tree_del_wait (pTree, "492");
		myb7_tree_del_wait (pTree, "28");
		myb7_tree_del_wait (pTree, "998");
		myb7_tree_del_wait (pTree, "909");
		myb7_tree_del_wait (pTree, "554");
		myb7_tree_del_wait (pTree, "297");
		myb7_tree_del_wait (pTree, "487");
		myb7_tree_del_wait (pTree, "226");
		myb7_tree_del_wait (pTree, "149");
		myb7_tree_del_wait (pTree, "867");
		myb7_tree_del_wait (pTree, "867");
		myb7_tree_del_wait (pTree, "844");
		myb7_tree_del_wait (pTree, "779");
		myb7_tree_del_wait (pTree, "674");
		myb7_tree_del_wait (pTree, "663");
		myb7_tree_del_wait (pTree, "633");
		myb7_tree_del_wait (pTree, "573");
		myb7_tree_del_wait (pTree, "529");
		myb7_tree_del_wait (pTree, "499");
		myb7_tree_del_wait (pTree, "59");
		myb7_tree_del_wait (pTree, "483");
		
		printmyb7_tree (pTree);
		usleep(100000);
	}while(optlong == 'm');

	exit (0);
}
#endif
