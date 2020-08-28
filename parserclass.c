/* parserclass.c 
 * tool to parse configuration file.
 * current tool have static predefined parameter.
 * this tool is a layered one.
 * gcc xparsecfg.c -Wall -ggdb3 -o xparsecfg
 * here we try object oriented paradigm to our structures.
 * to do this, our structures shall contain pointers to 
 * functions that each of them eligible to call.
 * the implementation of the function, namely kernel function
 * always begins with __ (double under score) followed by
 * class name that it is the implemented to. 
 * if this kernel function calls another functions either within
 * its own class or of another class, then it is better to use 
 * class implementation of the function rather than kernel function. 
 * collection class : tree and list
 * take** will detach
 * get** will not detach item
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parserclass.h"

void print_identation (int ident)
{
	int i;
	for (i = 0; i < ident; i++)
		fprintf (stdout, "\t"); 
} 

char *trimstring (char *instr)
{
	int len = strlen(instr);
	int first = 0;
	int last = len;
	int k = 0;
	int i = 0;
	if (!len)
		return instr;

	k = (int)instr[i++];

	while(k != 0)
	{
		if (k != ' ')
			break;	
		k = (int)instr[i++];
	}
	first = i-1; 
	i = len-1;

	do
	{
		k = (int)instr[i--];
		if(k != ' ')
			break;
	}
	while ((k != 0) && (i > first)) ;
	
	last = i+1; 
	len = last - first + 1;
	memcpy (instr, instr + first, len);
	instr[len]  = 0; 
	return instr;
}


static int __class_delete (PCLASS p)
{
	memset (p->name, 0, MAX_NAME_LENGTH);
	p->this = NULL;
	free (p); 
	p = NULL;
	return 0;
}
static int __class_printattributes(PCLASS p, int ident)
{
	print_identation (ident);
	if (p->name[0])
		fprintf (stdout, "class.name : %s", p->name);
	else
		fprintf (stdout, "class.name : ");
	fprintf (stdout, "\n");
	return 0;
}

void __class_ctor (PCLASS pclass,char *name)
{ 
	pclass->this = pclass;
	pclass->type = CLASS_CLASS;
	strncpy (pclass->name, name, MAX_NAME_LENGTH); 
	pclass->printattributes = __class_printattributes; 
	pclass->delete = __class_delete; 
}

PCLASS newclass (char *name)
{
	PCLASS pclass = (PCLASS) calloc (1, sizeof (CLASS));
	if (pclass)
		__class_ctor (pclass, name);
	return pclass; 
} 

static int __l_item_printattributes(PL_ITEM p, int ident)
{
	return __class_printattributes(&p->class, ident); 
}
/**
 * Name		: __l_item_delete 
 * Description	: override __class_delete for ITEM structure
 * Returns	
 *		0 on success
 */
static int __l_item_delete(PL_ITEM p)
{
	p->next = NULL;
	return __class_delete(& p->class);
}

void __l_item_ctor (PL_ITEM pl_item, char *name)
{
	__class_ctor (&pl_item->class, name); 
	pl_item->class.type = CLASS_L_ITEM; 
	pl_item->next = 0;
	pl_item->class.printattributes = 
		(int (*) (PCLASS, int))(__l_item_printattributes);
	pl_item->class.delete = 
		(int (*) (PCLASS))(__l_item_delete);
}

PL_ITEM newl_item (char *name)
{
	PL_ITEM pl_item = (PL_ITEM) calloc (1, sizeof (L_ITEM));
	if (pl_item)
		__l_item_ctor (pl_item, name);
	return pl_item;
}


static int __list_printattributes(PLIST p, int ident)
{
	PL_ITEM pl_item = NULL;
	__l_item_printattributes(&p->l_item, ident);
	pl_item = p->head;
	while (pl_item)
	{
		pl_item->class.printattributes(&pl_item->class, ident+1);
		pl_item = pl_item->next;
	} 
	fprintf (stdout, "\n");
	return 0;
}

/**
 * NAME		: __list_addproperty
 * DESCRIPTION	: add new property
 * INPUT
 *		- plist , this pointer
 *		- name , name of this property
 *		- value, value of this property
 **/
static int  __list_addproperty (PLIST plist, char *name, char *value)
{
	PPROPERTY add = newproperty2 (name, value);
	if (!add)
		return -1; 
	if (plist->head == NULL)
	{
		plist->head = &add->l_item;
		plist->tail = plist->head; 
		plist->currptr = plist->head;
		plist->count++; 
	}
	else
	{
		plist->tail->next = &add->l_item;
		plist->tail = &add->l_item; 
		plist->count++;
	}
	plist->tail->next = NULL;
	return 0;
}

/**
 * NAME		: __list_add
 * DESCRIPTION	: static function that add time to list class 
 */
static void __list_add (PLIST plist, PL_ITEM add)
{
	if (plist->head == NULL)
	{
		plist->head = add;
		plist->tail = plist->head; 
		plist->currptr = plist->head;
		plist->count++;
	}
	else
	{
		plist->tail->next = add;
		plist->tail = add; 
		plist->count++;
	}
	plist->tail->next = NULL;
}

/**
 * NAME		: __list_detach
 * DESCRIPTION	: detach a specified list item
 * RETURNS	: 0 if success
 *		  pointer to detached if it is not in the list
 *
 */
static PL_ITEM  __list_detach (PLIST plist, PL_ITEM detached)
{
	PL_ITEM curr = plist->head;
	while (curr)
	{
		if (detached == curr){
			if (detached == plist->currptr)
				plist->currptr = plist->currptr->next;
			return NULL;
		}
		curr = curr->next;
	} 
	return detached;
}

/**
 * NAME		: __list_take
 * DESCRIPTION	: take head
 *                returned head detached
 */
static PL_ITEM __list_take (PLIST plist)
{
	PL_ITEM ret;
	if (plist->head)
	{
		ret = plist->head;
		plist->head = plist->head->next;
		plist->count --; 
		return ret;
	}
	else
	{
		return NULL;
	}
} 

/**
 * NAME 	: __list_delete
 * DESCRIPTION	: clean up the list and delete object
 */
static int __list_delete(PLIST plist)
{

	PL_ITEM curr;
	while (( curr = __list_take(plist)))
	{
		curr->class.delete (&curr->class);
		plist->count --;
	}
	return __class_delete (&plist->l_item.class);
}

/**
 * NAME		: __list_takename
 * DESCRIPTION	: take member element of name xx
 *                returned item is detached
 */
static PL_ITEM __list_takename(PLIST plist, char* name)
{
	PL_ITEM curr = plist->head;
	PL_ITEM prev = NULL;
		
	while (curr)
	{ 
		if (!strncmp (name, curr->class.name, MAX_NAME_LENGTH))
		{
			if (prev)
			{
				prev->next = curr->next;
				curr->next = NULL;
			}
			else
			{
				plist->head = curr->next;
				curr->next = NULL;	
			}
			return curr;
		}
		prev = curr;	
		curr = curr->next;
	} 
	return NULL;
}

/**
 * NAME		: __list_getname
 * DESCRIPTION	: get member element of name xx
 *                returned item is not detached from list
 */
static PL_ITEM __list_getname(PLIST plist, char* name)
{
	PL_ITEM curr = plist->head; 
	while (curr)
	{ 
		if (!strncmp (name, curr->class.name, MAX_NAME_LENGTH))
		{
			return curr;
		}
		curr = curr->next;
	} 
	return NULL;
}

/**
 * NAME		: __list_getfirstchild
 * DESCRIPTION	: returns the first child of this item
 *		  move the curr to next item of the first child
 */
static PL_ITEM __list_getfirstchild (PLIST plist)
{
	plist->currptr = plist->head;
	if (plist->currptr) 
		plist->currptr = plist->currptr->next;
	return plist->head;
}

/**
 * NAME		: __list_getnextchild
 * DESCRIPTION	: returns the next  child of this list.
 *		  move the curr to next item of curr item.
 */
static PL_ITEM __list_getnextchild (PLIST plist)
{
	PL_ITEM ret = plist->currptr;
	if (plist->currptr)
		plist->currptr = plist->currptr->next;
	return ret;	
}

void __list_ctor (PLIST plist, char *list_name)
{ 
	__l_item_ctor (&plist->l_item, list_name);
	plist->l_item.class.type = CLASS_LIST; 
	plist->currptr = NULL; 
	plist->l_item.next = NULL;
	plist->count = 0;
	plist->add = __list_add;
	plist->take = __list_take;
	plist->takename = __list_takename; 
	plist->getname = __list_getname; 
	plist->getfirstchild = __list_getfirstchild; 
	plist->getnextchild= __list_getnextchild; 
	plist->l_item.class.printattributes = 
		(int(*)(PCLASS, int))(__list_printattributes);
	plist->l_item.class.delete= 
		(int(*)(PCLASS))(__list_delete); 
	plist->detach = __list_detach; 
	plist->addproperty = __list_addproperty;
}

void list_resetlist (PLIST plist, char *list_name)
{ 
    __list_ctor(plist, list_name);
}


PLIST newlist (char *list_name)
{
	PLIST plist = (PLIST) calloc (1, sizeof (LIST));
	list_resetlist(plist, list_name);
	return plist;
}


static int __property_delete(PPROPERTY p) 
{
	memset (p->value, 0, sizeof(p->value));
	__class_delete (&p->l_item.class);
	return 0;
}

static int __property_setvalue (PPROPERTY p, char *value)
{
	strncpy (p->value, value, sizeof(p->value));
	return 0;
}

static int __property_getvalue (PPROPERTY p, char *value)
{
	strncpy (value, p->value, sizeof(p->value));
	return 0;
}

static char* __property_getvalue_ptr (PPROPERTY p) {
	return p->value;
}

static int __property_printattributes(PPROPERTY p, int ident)
{
	__l_item_printattributes(&p->l_item, ident);	
	print_identation (ident);
	fprintf (stdout, "property.value : %s", p->value);
	fprintf (stdout, "\n");
	return 0;
}

/**
 * NAME		: __property_ctor 
 * DESCRIPTION	: Constructor for propery
 * INPUT	  
 *		- prop, self pointer
 *		- name, object name
 */
void __property_ctor (PPROPERTY prop, char *name)
{
	__l_item_ctor (&prop->l_item, name);
	prop->l_item.class.this = (PCLASS) prop;
	prop->l_item.class.type = CLASS_PROPERTY;
	strncpy (prop->l_item.class.name, name, MAX_NAME_LENGTH - 1);
	memset (prop->value, 0, 256);
	prop->setvalue = __property_setvalue;
	prop->getvalue_ptr = __property_getvalue_ptr;
	prop->getvalue= __property_getvalue;
	prop->l_item.next = NULL;
	prop->l_item.class.printattributes = 
		(int (*) (PCLASS, int))(__property_printattributes);
	prop->l_item.class.delete = 
		(int (*) (PCLASS))(__property_delete);
}

/**
 * NAME		: __property_ctor2 
 * DESCRIPTION	: Constructor for propery
 * INPUT	  
 *		- prop, self pointer
 *		- name, object name
 *		- value, object value
 */
void __property_ctor2 (PPROPERTY prop, char *name, char *value)
{ 
	__l_item_ctor (&prop->l_item, name);
	prop->l_item.class.this = (PCLASS) prop;
	prop->l_item.class.type = CLASS_PROPERTY;
	strncpy (prop->l_item.class.name, name, MAX_NAME_LENGTH - 1);
	memset (prop->value, 0, 256);
	prop->setvalue = __property_setvalue;
	prop->getvalue_ptr = __property_getvalue_ptr;
	prop->getvalue= __property_getvalue;
	prop->l_item.next = NULL;
	__property_setvalue (prop, value);
	prop->l_item.class.printattributes = 
		(int (*) (PCLASS, int))(__property_printattributes);
	prop->l_item.class.delete = 
		(int (*) (PCLASS))(__property_delete);
}

PPROPERTY newproperty (char *name)
{
	PPROPERTY prop = (PPROPERTY) calloc (1, sizeof (PROPERTY));
	if (prop)
	{	
		__property_ctor (prop, name);
	}
	return prop;
}

PPROPERTY newproperty2 (char *name, char *value)
{
	
	PPROPERTY prop = (PPROPERTY) calloc (1, sizeof (PROPERTY));
	if (prop)
	{	
		__property_ctor2 (prop, name, value);
	}
	return prop;
}

int __stack_ptr_init (PSTACK_PTR p)
{
	p->top  = -1;
	memset (p->c, 0, MAX_STACKPTR * sizeof(void *));
	return 0;
}

int __stack_ptr_push (PSTACK_PTR p, void * v)
{
	if (p->top >= MAX_STACKPTR)
		return -1;
	p->top++; 
	p->c[p->top] = v;
	return 0;
}

void * __stack_ptr_pop (PSTACK_PTR p)
{
	void *ret;
	if (p->top < 0)
		return NULL;
	ret = p->c[p->top--];
	return ret;
}

/**
 * Name		: __stack_ptr_cleanup
 * Description	: release all pointers in this stack
 **/
int  __stack_ptr_cleanup (PSTACK_PTR p)
{
	void * ptr;
	while (p->top >= 0)
	{
		ptr = __stack_ptr_pop (p);
		if (!ptr)
			continue;
		free (ptr);
	}
	return __stack_ptr_init (p);

} 

int __stack_ptr_isempty (PSTACK_PTR p)
{
	int ret = 0;
	ret = (p->top < 0)? 1: 0;
	return ret;
}

PSTACK_PTR newstackptr ()
{
	PSTACK_PTR p = (PSTACK_PTR) calloc (1, sizeof (STACK_PTR));
	p->top = -1;
	p->init = __stack_ptr_init;
	p->push = __stack_ptr_push;
	p->pop = __stack_ptr_pop;
	p->cleanup = __stack_ptr_cleanup;
	p->is_empty = __stack_ptr_isempty;
	return p;	
}
 
PMINIPARSER newminiparser (FILE *in, char *tokenlist, int (*f)(PMINIPARSER, PLIST *))
{
	PMINIPARSER p = (PMINIPARSER) calloc (1, sizeof (MINIPARSER));
	p->input = in;
	p->tokenlist = tokenlist;
	p->parse = f;	
	return p;
}
/*
 * NAME		: newcharparser.
 * DESCRIPTION	: new char parser
 * 		  Do not delete tokenlist and in until parsing done.
 */ 

PCHARPARSER newcharparser (char *in, int inlen, char *tokenlist, 
				int (*f)(PCHARPARSER, PLIST *))
{
	PCHARPARSER p = (PCHARPARSER) calloc (1, sizeof (CHARPARSER)); 
	p->input = in;
	p->currpos = -1;
	p->charlen = inlen;
	p->tokenlist = tokenlist;
	p->parse  = f;	
	return p;
}
/**
 * Name : stream_gettoken
 * Description : get the first token that match any member of token list.
 *               copy buffer read from first read until token found to buffer.
 * Input 
 *      fp        : file pointer to inspect.
 *      tokenlist : list of accpetable token, must ended with character zero.
 *      buff      : pointer to buffer where to  store bytes read from file until
 *                  token found. 
 *      max_buffsize : max bytes can be stored in buff.
 *      length_read  : length of bytes copied to buffer.
 *      
 * 	
 * Returns : index to match pointer, EOF if we already in the end of file.
 *      If no token is found then either EOF or index >= pointer to zero.  
 */
int stream_gettoken (FILE *fp, char tokenlist[], char * buff, 
	int max_buffsize, int *length_read)
{
	int ch  = 0;
	int i = 0;
	int j = 0;
	*length_read = 0;
	while ((ch = fgetc (fp)) != EOF)
	{
		i = 0;
		while (tokenlist[i] !=0 )
		{
			if (tokenlist[i] == (char) ch)
			{
				*length_read = j;	
				buff[j] = 0;
				return i;
			}
			i++;
		} 
		if ( j < max_buffsize )	
			buff[j++] = (int) ch; 
	}
	return -1;
}

/*
 * Name : stream_expectnexttoken  
 * Description : Wait for a specified tokens.
 * INPUT 
 *	fp	  : file to process
 *	tokenlist : list of available token
 *	expected_token: next token that expected to present	
 * OUTPUT
 *	buff	  : buffer to store chars until next token found
 *	*bufflen  : number of bytes stored in buffer
 * RETURN VALUE
 * 	-1	: fails 
 *	others	: token index 
 *
 */
int stream_expectnexttoken (FILE *fp, char *tokenlist, char *buff, 
	int max_buffsize, int *readlen,  char *expected_token)
{
	int i = 0;
	int token = stream_gettoken (fp, tokenlist, buff, max_buffsize, readlen);
	while (expected_token[i] != 0)
	{
		if (tokenlist[token] == expected_token[i])
			return token;
		i ++;
	}
	return -1;
	
}

/**
 * Name : string_gettoken
 * Description : get the first token that match any member of token list.
 *               copy buffer read from first read until token found to buffer.
 * Input 
 *      fp        : file pointer to inspect.
 *      tokenlist : list of accpetable token, must ended with character zero.
 *      buff      : pointer to buffer where to  store bytes read from file until
 *                  token found. 
 *      max_buffsize : max bytes can be stored in buff.
 *      length_read  : length of bytes copied to buffer.
 *      
 * 	
 * Returns : index to match pointer, EOF if we already in the end of file.
 *      If no token is found then either EOF or index >= pointer to zero.  
 */
int string_gettoken (char *input, char tokenlist[], char * buff, 
	int max_buffsize, int *length_read)
{
	int ch  = 0;
	int i = 0;
	int j = 0;
	int ndx = 0;
	*length_read = 0;
	while ((ch = input[ndx++]) != 0)
	{
		i = 0;
		while (tokenlist[i] !=0 )
		{
			if (tokenlist[i] == (char) ch)
			{
				*length_read = j;	
				buff[j] = 0;
				return i;
			}
			i++;
		} 
		if ( j < max_buffsize )	
		{
			buff[j++] = (int) ch; 
			*length_read = j;
		}
	}
	return -1;
}


/*
 * Name : string_expectnexttoken  
 * Description : Wait for a specified tokens.
 * INPUT 
 *	fp	  : file to process
 *	tokenlist : list of available token
 *	expected_token: next token that expected to present	
 * OUTPUT
 *	buff	  : buffer to store chars until next token found
 *	*bufflen  : number of bytes stored in buffer
 * RETURN VALUE
 * 	-1	: fails 
 *	others	: token index 
 *
 */
int string_expectnexttoken (char *input, char *tokenlist, char *buff, 
	int max_buffsize, int *readlen,  char *expected_token)
{
	int i = 0;
	int token = string_gettoken (input, tokenlist, buff, max_buffsize, readlen);
	while (expected_token[i] != 0)
	{
		if (tokenlist[token] == expected_token[i])
			return token;
		i ++;
	}
	return -1;
	
}

/**
 * NAME		: parse_env_str
 * DESCRIPTION	: parse input string, interpret all prefixed $ string as
 *		  environment variables. Get the environment vars, replace
 *		  the prefixed $ string with env variable
 * INPUT	
 *	instring	: input string to be interpreted 
 *	newenv_max	: max length of buffer to store interpretted value
 *
 * OUTPUT
 *	newenv		: buffer to store interpretted value
 *
 * RETURNS
 *			: on return, returns length of interrpretted string
 */
#define ENVBUF_SZ 4096
int parse_env_str (char *instring, char *newenv, int newenv_max )
{
	char *pnewenv = &newenv [0];
	int token ;
	char buff [ENVBUF_SZ];
	int  readlen = 0;
	char *orig_env = 0;
	char *pvalue = instring;
 
	memset (newenv, 0, newenv_max);
	memset (buff, 0, ENVBUF_SZ);
	token = -1;

		
	token = string_gettoken (pvalue, 
		"$", buff, ENVBUF_SZ, &readlen); 
	while (token >=0)
	{
		if (readlen)
		{
			if ((newenv_max - (pnewenv - &newenv[0])) <readlen)
				break;
			memcpy (pnewenv, buff, readlen ); 
			pnewenv += readlen < ENVBUF_SZ ? readlen : ENVBUF_SZ; 
			pvalue +=readlen;
		}
		readlen = 1; /*advance 1 token*/
		pvalue += readlen;
		readlen = 0;
		memset (buff, 0, ENVBUF_SZ);

		token = string_gettoken(pvalue, "$/:", buff, ENVBUF_SZ, &readlen);
		if (!readlen)
			break;
		
		/*
		 * getenv here
		 * write to new env
		 */
		pvalue += readlen;
		orig_env = getenv(buff);
		if (orig_env)
		{
			memcpy (pnewenv, orig_env, strlen (orig_env));
			pnewenv += strlen (orig_env);
		} 
		memset (buff, 0, ENVBUF_SZ);
		readlen = 0;
		if (token == -1)
		{
			break;
		}
		if (token == 0)
		{
			continue;
		}
		else
		{ 
			token = string_gettoken (pvalue, "$", buff, ENVBUF_SZ, &readlen); 
		} 
	} 
	if (readlen)
	{
		if ((ENVBUF_SZ - (pnewenv - &newenv[0])) >readlen)
		{ 
			memcpy (pnewenv, buff, readlen ); 
			pnewenv += readlen < ENVBUF_SZ? readlen : ENVBUF_SZ; 
		}
	}
	return pnewenv - newenv; 
}

static int __treeitem_add (struct tree_item *root, struct tree_item *addedchild)
{
	if (!root->head)
	{
		root->head = addedchild;
		root->tail = addedchild;	
	}
	else
	{
		root->tail->next = addedchild;
		root->tail = addedchild;
	}
	addedchild->parent = root;
	return 0;
}

static struct tree_item * __treeitem_getparent (struct tree_item *root)
{
	return root->parent;
}

/**
 * Name		: __treeitem_getfirstchild
 * Description	: get first child of this root
 *		  currchild will point to next item of 
 *		  head. 
 */
static struct tree_item * __treeitem_getfirstchild (struct tree_item *root)
{
	struct tree_item *ret = root->head;
	if (root->head){
		root->curr = root->head;
		root->curr = (PTREE_ITEM) root->curr->next;
	}
	return ret;
}

/**
 * Name		: __treeitem_getnextchild
 * Description	: return currchild
 *		: move currchild to current currchild next item
 */
static struct tree_item * __treeitem_getnextchild (struct tree_item *root)
{
	struct tree_item *ret = root->curr;
	if (root->curr)
		root->curr = (PTREE_ITEM) root->curr->next;
	return ret;
}

/**
 * Name		: __treeitem_takechild
 * Description	: take and detach child head
 *		  next child becomes current head 
 */
static struct tree_item * __treeitem_takechild (struct tree_item *root)
{
	struct tree_item * ret = 0;
	if (root->head)
	{
		ret = root->head;
		root->head = (PTREE_ITEM) root->head->next; 
	}
	return ret;
} 

/**
 * NAME		: __treeitem_listdelete
 * DESCRIPTION	: clean up the list and delete object
 */
static int __treeitem_listdelete(struct tree_item *root)
{
	PL_ITEM curr;
	while (( curr = root->list.take(&root->list)))
	{
		curr->class.delete (&curr->class);
		root->list.count --;
	}
	return 0;
}

/*
 * NAME 									: __treeitem_detach
 * DESCRIPTION 						: detach specified list item
 * RETURNS								: 
 * 					NULL					: if success 
 *	 pointer to detached 	: if not found
 */
static PTREE_ITEM  __treeitem_detach(struct tree_item *root, struct tree_item *detached)
{
	PTREE_ITEM curritem = root->head;
	PTREE_ITEM prev = NULL;
	while (curritem)
	{
		if (detached == curritem) {
			if (prev){
				prev->next = curritem->next;
			}
			else {
				root->head = curritem->next;
			}
			return NULL;
		}
		prev = curritem;
		curritem = curritem->next;
	}
	return detached;
}

/**
 * NAME		: __treeitem_delete
 * DESCRIPTION	: delete a treeitem along with its child
 */
static void  __treeitem_delete (struct tree_item *root)
{
	struct tree_item* child = __treeitem_takechild(root);
	while (child)
	{ 
		__treeitem_delete (child); 
		child = __treeitem_takechild(root);
	}
	__treeitem_listdelete (root);
	__class_delete((PCLASS)root);	
}

/**
 * NAME		:__treeitem_takename
 * DESCRIPTION	: take member element of name xx
 *		  returned item is detached
 */
static PTREE_ITEM __treeitem_takename (PTREE_ITEM root, char *name)
{
	PTREE_ITEM curr = root->head;
	PTREE_ITEM prev = NULL;

	while (curr)
	{
		if (!strncmp (name, curr->list.l_item.class.name, MAX_NAME_LENGTH))
		{
			if (prev)
			{
				prev->next = curr->next;
				curr->next = NULL;
			}
			else
			{
				root->head = curr->next;
				curr->next = NULL;
			}
			return curr;
		}
		prev = curr;
		curr = curr->next;
	}
	return NULL;
}

/**
 * NAME		: __treeitem_getname
 * Description	: get member element of name xx
 *		  returned item is not detached from list
 */
static PTREE_ITEM __treeitem_getname (PTREE_ITEM root, char *name)
{
	PTREE_ITEM curr = root->head;

	while (curr)
	{
		if (!strncmp (name, curr->list.l_item.class.name, MAX_NAME_LENGTH))
		{
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

static int __treeitem_printattributes(PTREE_ITEM root, int ident){
	__list_printattributes(&root->list, ident + 1);
	PTREE_ITEM curr = root->head;
	while(curr){
		__treeitem_printattributes(curr, ident + 1);
		curr = curr->next;
	}
	return 0;
}


void __treeitem_ctor (PTREE_ITEM  new, PTREE_ITEM parent, char *name)
{
	__list_ctor (&new->list, name);
	new->head = new->tail = new->curr = new->next = 0;
	new->parent = parent;
	list_resetlist (&new->list, name);
	new->list.l_item.class.delete = (int  (*) (PCLASS) ) (__treeitem_listdelete);
	new->add = __treeitem_add;
	new->getparent = __treeitem_getparent;
	new->getfirstchild = __treeitem_getfirstchild;
	new->getnextchild = __treeitem_getnextchild;
	new->takechild = __treeitem_takechild;
	new->takename = __treeitem_takename;
	new->getname = __treeitem_getname;
	new->detach = __treeitem_detach;
	new->delete = __treeitem_delete;
	new->list.l_item.class.printattributes = 
		(int(*)(PCLASS, int))(__treeitem_printattributes);
	
}
/*
 * NAME		: newtreeitem
 * Description	: alloc memory for treeitem
 *		  assign values and pointers
 */

struct tree_item * newtreeitem(struct tree_item *parent, char *name)
{
	PTREE_ITEM new = (PTREE_ITEM)calloc (1, sizeof (TREE_ITEM));
	if (new)
			__treeitem_ctor(new, parent, name);
	
	return new;
}

/**
 * Name		: __circularitem_delete
 * Description	: delete the supplied circular item
 * 		  set data to null and free the data
 **/
void __circularitem_delete(PCIRCULARITEM item)
{
	item->next = item->prev = NULL; 
	if (item->data)
	{
		memset (item->data, 0, item->datasize); 
		free (item->data);
	}
	memset (item, 0 , sizeof (CIRCULARITEM));
	free (item);
}

int  __circularitem_printattributes (PCIRCULARITEM citem, int ident)
{
	__class_printattributes (&citem->class, ident);
	print_identation (ident);
	fprintf (stdout, "prev : %p\n",  citem->prev);
	print_identation (ident);
	fprintf (stdout, "next : %p\n",  citem->next); 
	return 0;
	
} 
void __circularlist_add (PCIRCULARLIST clist, PCIRCULARITEM citem)
{
	if (!clist->current)
	{
		clist->current = citem;
		citem->prev = citem->next = citem; 
	}
	else
	{
		if (clist->current->prev)
		{
			clist->current->prev->next = citem;
			citem->next = clist->current;
			citem->prev = clist->current->prev;
			clist->current->prev = citem;
		}
	}
	clist->count ++;
}
int __circularlist_printattributes (PCIRCULARLIST clist, int ident)
{
	PCIRCULARITEM citem; 
	int k = clist->count;
	__class_printattributes (&clist->class, ident);
	citem = clist->current;
	while (k)
	{
		citem->class.printattributes(&citem->class, ident+1);
		citem = citem->next;	
		k--;
	}
	return 0;	
		
}
/**
 * Name		: __circularitem_take 
 * Description	: take current item from list 
 *		  taken item is detached from list
 */
PCIRCULARITEM __circularlist_take (PCIRCULARLIST clist)
{
	PCIRCULARITEM citem = clist->current;
	if (citem)
	{
		if (citem->prev)
		{
			citem->prev->next = citem->next; 
		}
		if (citem->next)
		{
			citem->next->prev = citem->prev;
		}
		if (citem->next == citem)
		{ 
			clist->current = NULL;		
		}
		else
		{
			clist->current = citem->next;
		}
		citem->next = citem->prev = NULL;
		clist->count --; 
		
	}
	return citem;
	
}

/**
 * Name		: __circularitem_takename 
 * Description	: take an item of name from list
 * Returns 	: the item if there is
 *		  NULL others 
 *
 */
PCIRCULARITEM  __circularlist_takename (PCIRCULARLIST clist, char *name)
{
	PCIRCULARITEM citem = clist->current;
	int i = 0;
	for (i = 0; i < clist->count ; i++)
	{
		if (!strcmp (citem->class.name, name) )
		{
			if (citem->prev)
			{
				citem->prev->next = citem->next;
			}
			if (citem->next)
			{
				citem->next->prev = citem->prev;
			}
			if (citem == clist->current)
			{
				if (citem->next == citem)
					clist->current = NULL;
				else
					clist->current = citem->next;
			}
			return citem;
		}
		citem = citem->next;

	}
	return NULL;
	
}

/*
 * Name		: __circularlist_remove
 * Description	: remove particular item from the list.
 * Returns	  NULL if item is of this list
 *		  Others if item is not of this list
 */
PCIRCULARITEM  __circularlist_remove (PCIRCULARLIST clist, PCIRCULARITEM citem)
{
	PCIRCULARITEM temp;
	temp = clist->current;
	if (!temp)
		return citem;
	do
	{
		if (citem == temp)
		{
			if (citem->prev)
				citem->prev->next  = citem->next;
			if (citem->next)
				citem->next->prev  = citem->prev;
			if (citem->next == citem)
				clist->current = NULL;
			else
				clist->current = citem->next;
			clist->count --;
			citem->class.delete ((PCLASS)citem);
			return NULL;
		}
		temp = temp->next;
	}while (temp != clist->current);
	return citem;
}
/**
 * Name		: __circularlist_delete
 * Description	: override class delete
 */
void __circularlist_delete (PCIRCULARLIST clist)
{
	while (clist->current)
	{
		__circularlist_remove (clist, clist->current);
	}
	clist->add = NULL;
	clist->remove = NULL;
	memset (clist, 0, sizeof (CIRCULARLIST));
	free (clist); 
} 

void __circularitem_ctor (PCIRCULARITEM pciri, char *name, void *data, int size)
{

	pciri->class.this = (PCLASS) pciri;
	pciri->class.type = CLASS_CIRCULARITEM;
	strncpy (pciri->class.name, name, MAX_NAME_LENGTH - 1); 
	pciri->next = pciri->prev = NULL;
	pciri->class.delete = (int (*)(PCLASS))(__circularitem_delete);
	pciri->class.printattributes = 
		(int (*) (PCLASS, int))(__circularitem_printattributes);
	pciri->datasize = size;
	pciri->data = data; 
}

/**
 * Name		: newcircularitem
 * Description	: new circularitem
 */

PCIRCULARITEM newcircularitem (char *name, void *data, int size)
{
	PCIRCULARITEM pciri = (PCIRCULARITEM) calloc (1, sizeof (CIRCULARITEM));
	if (pciri)
	{
		__circularitem_ctor (pciri, name, data, size);
	}
	return pciri;
}
void __circularlist_ctor (PCIRCULARLIST pcirlist, char *name)
{
	__class_ctor (&pcirlist->class, name); 
	pcirlist->class.type = CLASS_CIRCULARLIST; 
	pcirlist->add = (void (*) (PCIRCULARLIST, PCIRCULARITEM)) 
			 (__circularlist_add);
	pcirlist->remove = (int (*) (PCIRCULARLIST, PCIRCULARITEM)) 
			 (__circularlist_remove);
	pcirlist->take = (PCIRCULARITEM (*) (PCIRCULARLIST))
			 (__circularlist_take);
	pcirlist->takename = (PCIRCULARITEM (*) (PCIRCULARLIST, char *))
			 (__circularlist_takename);
	pcirlist->class.delete = (int (*)(PCLASS))(__circularlist_delete);
	pcirlist->class.printattributes = 
		(int (*) (PCLASS, int))(__circularlist_printattributes);

}
/**
 * Name		: newcircularlist
 * Description	: new circularlist
 *
 */
PCIRCULARLIST newcircularlist (char *name)
{
	PCIRCULARLIST pcirlist = (PCIRCULARLIST) calloc (1, sizeof (CIRCULARLIST));
	if (pcirlist)
	{
		__circularlist_ctor (pcirlist, name);
	}
	return pcirlist;
}

static int __primclass_delete (PPRIMCLASS p)
{
	p->this = NULL;
	free (p); 
	p = NULL;
	return 0;
}

static int __primclass_printattributes(PPRIMCLASS p, int ident)
{
	print_identation (ident);
	fprintf (stdout, "\n");
	return 0;
}

PPRIMCLASS __newprimclass ()
{
	PPRIMCLASS pclass = (PPRIMCLASS) calloc (1, sizeof (PRIMCLASS));
	pclass->this = pclass;
	pclass->type = PRIMCLASS_PRIMCLASS;
	pclass->printattributes = __primclass_printattributes; 
	pclass->delete = __primclass_delete; 
	return pclass;
}


static int __priml_item_printattributes(PPRIML_ITEM p, int ident)
{
	return __primclass_printattributes(&p->primclass, ident); 
}

static PPRIML_ITEM __priml_item_set_data (PPRIML_ITEM p, void *data) {
	if (!data){
		return NULL;
	}
	else {
		p->data = data;
		return p;
	}
}

static int __priml_item_delete(PPRIML_ITEM node) {
	if (!node) {
		return 1;
	}
	else {
		if(node->data) {
			free(node->data);
			node->data = NULL;
		}
		node->next = NULL;
		node->set_data = NULL;
		node->delete = NULL;
		free(node); 
	}
	return 0;
} 

PPRIML_ITEM newpriml_item ()
{
	PPRIML_ITEM ppriml_item = (PPRIML_ITEM) calloc (1, sizeof (PRIML_ITEM));
	ppriml_item->primclass.this = (PPRIMCLASS) ppriml_item;
	ppriml_item->primclass.type = PRIMCLASS_PRIMLITEM; 
	ppriml_item->next = 0;
	ppriml_item->data = NULL;
	ppriml_item->primclass.printattributes = 
		(int (*) (PPRIMCLASS, int))(__priml_item_printattributes);
	ppriml_item->primclass.delete = 
		(int (*) (PPRIMCLASS))(__priml_item_delete);
	ppriml_item->set_data = __priml_item_set_data;
	ppriml_item->delete = __priml_item_delete;
	return ppriml_item;
}


static int __primlist_printattributes(PPRIMLIST p, int ident)
{
	PPRIML_ITEM ppriml_item = NULL;
	__priml_item_printattributes(&p->priml_item, ident);
	ppriml_item = p->head;
	while (ppriml_item)
	{
		ppriml_item->primclass.printattributes(&ppriml_item->primclass, ident+1);
		ppriml_item = ppriml_item->next;
	} 
	fprintf (stdout, "\n");
	return 0;
}

/**
 * NAME		: __primlist_add
 * DESCRIPTION	: static function that add time to list class 
 */
static void __primlist_add (PPRIMLIST plist, PPRIML_ITEM add)
{
	if (plist->head == NULL)
	{
		plist->head = add;
		plist->tail = plist->head; 
		plist->count++;
	}
	else
	{
		plist->tail->next = add;
		plist->tail = add; 
		plist->count++;
	}
	plist->tail->next = NULL;
}

/**
 * NAME		: __primlist_detach
 * DESCRIPTION	: detach a specified list item
 * RETURNS	: 0 if success
 *		  pointer to detached if it is not in the list
 *
 */
static PPRIML_ITEM  __primlist_detach (PPRIMLIST plist, PPRIML_ITEM detached)
{
	PPRIML_ITEM curr = plist->head;
	while (curr)
	{
		if (detached == curr)
			return NULL;
		curr = curr->next;
	} 
	return detached;
}

/**
 * NAME		: __primlist_take
 * DESCRIPTION	: take head
 *                returned head detached
 */
static PPRIML_ITEM __primlist_take (PPRIMLIST plist)
{
	PPRIML_ITEM ret;
	if (plist->head)
	{
		ret = plist->head;
		plist->head = plist->head->next;
		plist->count --; 
		return ret;
	}
	else
	{
		return NULL;
	}
} 

/**
 * NAME 	: __primlist_delete
 * DESCRIPTION	: clean up the list and delete object
 */
static int __primlist_delete(PPRIMLIST plist)
{

	PPRIML_ITEM curr;
	while (( curr = __primlist_take(plist)))
	{
		curr->primclass.delete (&curr->primclass);
		plist->count --;
	}
	__primclass_delete (&plist->priml_item.primclass);
	return 0;
}

/**
 * NAME		: __list_takeitem
 * DESCRIPTION	: take member element of name xx
 *                returned item is detached
 */
static PPRIML_ITEM __primlist_takeitem(PPRIMLIST plist, unsigned char* data, int length)
{
	PPRIML_ITEM curr = plist->head;
	PPRIML_ITEM prev = NULL;
	while (curr)
	{ 
		if (!memcmp(data, curr->data, length))
		{
			if (prev)
			{
				prev->next = curr->next;
				curr->next = NULL;
			}
			else
			{
				plist->head = curr->next;
				curr->next = NULL;	
			}
			return curr;
		}
		prev = curr;	
		curr = curr->next;
	}
	return NULL;
}

/**
 * NAME		: __primlist_getitem
 * DESCRIPTION	: get member element of name xx
 *                returned item is not detached from list
 */
static PPRIML_ITEM __primlist_getitem(PPRIMLIST plist, unsigned char* data, int length)
{
	PPRIML_ITEM curr = plist->head;
	PPRIML_ITEM prev = NULL;
		
	while (curr)
	{ 
		if (!memcmp (data, curr->data, length))
		{
			if (prev)
			{
				prev->next = curr->next; 
			}
			else
			{
				plist->head = curr->next; 
			}
			return curr;
		}
		prev = curr;	
		curr = curr->next;
	} 
	return NULL;
}

/**
 * NAME		: __primlist_getfirstchild
 * DESCRIPTION	: returns the first child of this item
 *		  move the curr to next item of the first child
 */
static PPRIML_ITEM __primlist_getfirstchild (PPRIMLIST plist)
{
	plist->currptr = plist->head;
	if (plist->currptr) 
		plist->currptr = plist->currptr->next;
	return plist->head;
}

/**
 * NAME		: __primlist_getnextchild
 * DESCRIPTION	: returns the next  child of this list.
 *		  move the curr to next item of curr item.
 */
static PPRIML_ITEM __primlist_getnextchild (PPRIMLIST plist)
{
	PPRIML_ITEM ret = plist->currptr;
	if (plist->currptr)
		plist->currptr = plist->currptr->next;
	return ret;
}

static PPRIMLIST __primlist_ctor(PPRIMLIST plist) {
	if (plist) {
		plist->priml_item.primclass.this = (PPRIMCLASS) plist;
		plist->priml_item.primclass.type = PRIMCLASS_PRIMLIST;
		plist->currptr = NULL;
		plist->priml_item.next = NULL;
		plist->count = 0;
		plist->add = __primlist_add;
		plist->take = __primlist_take;
		plist->takeitem = __primlist_takeitem;
		plist->getitem = __primlist_getitem;
		plist->getfirstchild = __primlist_getfirstchild;
		plist->getnextchild= __primlist_getnextchild;
		plist->priml_item.primclass.printattributes = 
			(int(*)(PPRIMCLASS, int))(__primlist_printattributes);
		plist->priml_item.primclass.delete =
			(int(*)(PPRIMCLASS))(__primlist_delete);
		plist->detach = __primlist_detach;
		plist->delete = __primlist_delete;
	}
	return plist;
}

PPRIMLIST newprimlist ()
{
	PPRIMLIST plist = (PPRIMLIST) calloc (1, sizeof (PRIMLIST));
	if (plist)
	{
		return __primlist_ctor(plist);
	}
	return plist;
}

static struct primtree_item * __primtreeitem_getparent (struct primtree_item *node)
{
	if (node){
		return node->parent;
	}
	else {
		return NULL;
	}
}

static struct primtree_item * __primtreeitem_getfirstchild (struct primtree_item *node) {
	struct primtree_item *ret=node->head;
	if (node->head){
		node->curr = node->head->next;
	}
	return ret;	
}

static struct primtree_item * __primtreeitem_getnextchild (struct primtree_item *node)
{
	struct primtree_item *ret = node->curr;
	if (node->curr){
		node->curr = node->curr->next;
	}
	return ret;
}

/**
 * detach head and return detached head
 */
static struct primtree_item * __primtreeitem_detach_head (struct primtree_item *node)
{
	struct primtree_item * ret = NULL;
	if (node->head)
	{
		ret = node->head;
		node->head = (PPRIMTREE_ITEM) node->head->next; 
	}
	return ret;
}

/**
 * delete list items 
 */
static int __primtreeitem_listdelete(struct primtree_item *node)
{
	PPRIML_ITEM curr;
	while (( curr = node->list.take(&node->list)))
	{
		curr-> delete(curr);
		node->list.count --;
	}
	return 0;
}

static PPRIMTREE_ITEM  __primtreeitem_detach_node(struct primtree_item *node, struct primtree_item *detached)
{
	PPRIMTREE_ITEM curritem = node->head;
	PPRIMTREE_ITEM prev = NULL;
	while (curritem)
	{
		if (detached == curritem) {
			if (prev){
				prev->next = curritem->next;
			}
			else {
				node->head = curritem->next;
			}
			return NULL;
		}
		prev = curritem;
		curritem = curritem->next;
	}
	return detached;
}

static PPRIMTREE_ITEM __primtreeitem_get_child(PPRIMTREE_ITEM node, int (*fn) (PPRIMTREE_ITEM)) {
	PPRIMTREE_ITEM curritem = node->head;
	while (curritem)
	{
		if (fn(curritem))
			return curritem;
		curritem = curritem->next;
	}
	return NULL; 
}

static int __primtreeitem_delete(PPRIMTREE_ITEM pitem) {
	PPRIMTREE_ITEM child = NULL;
	if (pitem){
		while((child = __primtreeitem_detach_head(pitem))) {
			__primtreeitem_delete(child);
		}
	}
	__primtreeitem_listdelete(pitem);
	free(pitem);
	return 0;
}

static PPRIMTREE_ITEM __primtreeitem_ctor(PPRIMTREE_ITEM pitem) {
	if (pitem) {
		__primlist_ctor(&pitem->list);
		pitem->get_parent = __primtreeitem_getparent;
		pitem->get_first_child= __primtreeitem_getfirstchild;
		pitem->get_next_child= __primtreeitem_getnextchild;
		pitem->detach_head= __primtreeitem_detach_head;
		pitem->detach_node= __primtreeitem_detach_node;
		pitem->get_child = __primtreeitem_get_child;
		pitem->list.priml_item.primclass.delete = 
			(int (*) (PPRIMCLASS))(__primtreeitem_delete);
		pitem->delete= __primtreeitem_delete;
	}
	return pitem;
}
PPRIMTREE_ITEM newprimtreeitem() {
	PPRIMTREE_ITEM pitem = (PPRIMTREE_ITEM) calloc (1, sizeof(PRIMTREE_ITEM));
	if (pitem) {
		__primtreeitem_ctor(pitem);
	}
	return pitem;
}

#ifdef _PARSER_CLASS_TEST_
int main (int argc, char **argv) {
	
}
#endif
