#ifndef _MYB_TREE_STRING_H_
#define _MYB_TREE_STRING_H_
/**
 * myb7_string_node
 * 1. Every node has at most 7 children
 * 2. Every internal node has at least ceil(7) (which is 4) children
 * 3. The root has at least 2 children
 * 4. A non-leaf node with k children contains k-1 keys
 * 5. All leaves appear in the same level
 * myb7_string_node valid keys >= 0
 */
#define ORDER 7
#define NUMKEY (ORDER-1)
#define MEDIAN (ORDER+1)/2
#define B7_G_KSIZE 20
typedef struct myb7_string_node
{
	char k[6][B7_G_KSIZE];
	struct myb7_string_node *c[7];
}MYB7_STRING_NODE, *PMYB7_STRING_NODE; 
typedef struct myb7_string_tree
{
	char name[20];
	PMYB7_STRING_NODE root;
}MYB7_STRING_TREE, *PMYB7_STRING_TREE; 
PMYB7_STRING_NODE allocmyb7_string_treeNode ();
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
int myb7_tree_find(PMYB7_STRING_TREE tree, char *key, int (*fn) (char *, char *), PMYB7_STRING_NODE *out);

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
PMYB7_STRING_NODE myb7_tree_insert (PMYB7_STRING_TREE tree, char *key, int  (*fn) (char *, char *));

int myb7_tree_del(PMYB7_STRING_TREE tree, char *key, int (*fn) (char *, char *));

PMYB7_STRING_TREE myb7tree_new (char *name);
typedef struct string_item
{
	PMYB7_STRING_NODE pnd;
	int    e;
}STRING_ITEM, *PSTRING_ITEM;

#define MAX_STRING_STACK 100
typedef struct stack
{
	int top;
	STRING_ITEM el[MAX_STRING_STACK];
}STRING_STACK, *PSTRING_STACK;


int push (PSTRING_STACK S, PMYB7_STRING_NODE N, int dir);

PSTRING_ITEM pop (PSTRING_STACK S); 

void clean (PSTRING_STACK S); 

PSTRING_STACK stack_creat(); 

void printmyb7_tree (PMYB7_STRING_TREE pTree);

#endif
