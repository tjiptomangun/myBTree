#ifndef _MYB_TREE_GENERIC_H_
#define _MYB_TREE_GENERIC_H_
/**
 * myb7_generic_node
 * 1. Every node has at most 7 children
 * 2. Every internal node has at least ceil(7) (which is 4) children
 * 3. The root has at least 2 children
 * 4. A non-leaf node with k children contains k-1 keys
 * 5. All leaves appear in the same level
 * myb7_generic_node valid keys >= 0
 */
#define ORDER 7
#define NUMKEY (ORDER-1)
#define MEDIAN (ORDER+1)/2
#define B7_G_KSIZE 20
typedef struct myb7_generic_node
{
	char k[6][B7_G_KSIZE];
	struct myb7_generic_node *c[7];
}MYB7_GENERIC_NODE, *PMYB7_GENERIC_NODE; 
typedef struct myb7_generic_tree
{
	char name[20];
	PMYB7_GENERIC_NODE root;
}MYB7_GENERIC_TREE, *PMYB7_GENERIC_TREE; 
PMYB7_GENERIC_NODE allocmyb7_generic_treeNode ();
/*
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
int myb7_tree_find(PMYB7_GENERIC_TREE tree, char *key, int (*fn) (char *, char *), PMYB7_GENERIC_NODE *out);

/*
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
PMYB7_GENERIC_NODE myb7_tree_insert (PMYB7_GENERIC_TREE tree, char *key, int  (*fn) (char *, char *));

int myb7_tree_del(PMYB7_GENERIC_TREE tree, char *key, int (*fn) (char *, char *));

PMYB7_GENERIC_TREE myb7tree_new (char *name);
typedef struct generic_item
{
	PMYB7_GENERIC_NODE pnd;
	int    e;
}GENERIC_ITEM, *PGENERIC_ITEM;

#define MAX_GENERIC_STACK 100
typedef struct stack
{
	int top;
	GENERIC_ITEM el[MAX_GENERIC_STACK];
}GENERIC_STACK, *PGENERIC_STACK;


int push (PGENERIC_STACK S, PMYB7_GENERIC_NODE N, int dir);

PGENERIC_ITEM pop (PGENERIC_STACK S); 

void clean (PGENERIC_STACK S); 

PGENERIC_STACK stack_creat(); 

void printmyb7_tree (PMYB7_GENERIC_TREE pTree);

#endif
