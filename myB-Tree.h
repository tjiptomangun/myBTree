/**
 * myb7_node
 * 1. Every node has at most 7 children
 * 2. Every internal node has at least ceil(7) (which is 4) children
 * 3. The root has at least 2 children
 * 4. A non-leaf node with k children contains k-1 keys
 * 5. All leaves appear in the same level
 * myb7_node valid keys >= 0
 */
#define ORDER 7
#define NUMKEY (ORDER-1)
#define MEDIAN (ORDER+1)/2
typedef struct myb7_node
{
	int k[6];
	struct myb7_node *c[7];
}MYB7_NODE, *PMYB7_NODE; 
typedef struct myB7_Tree
{
	char name[20];
	PMYB7_NODE root;
}MYB7_TREE, *PMYB7_TREE; 
PMYB7_NODE allocmyB7_TreeNode ();
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
int myb7_tree_find(PMYB7_TREE tree, int key, PMYB7_NODE *out);

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
PMYB7_NODE myb7_tree_insert(PMYB7_TREE tree, int key); 
int myb7_tree_del(PMYB7_TREE tree, int key); 
PMYB7_TREE myb7tree_new (char *name);
typedef struct item
{
	PMYB7_NODE pnd;
	int    e;
}ITEM, *PITEM;

#define MAX_STACK 100
typedef struct stack
{
	int top;
	ITEM el[MAX_STACK];
}STACK, *PSTACK;


int push (PSTACK S, PMYB7_NODE N, int dir);

PITEM pop (PSTACK S); 

void clean (PSTACK S); 

PSTACK stack_creat(); 
