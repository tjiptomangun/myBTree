/**
 * gcc -m32 -I. parserclass.c my23.c my23test.c -Wall -ggdb3 -o my23test
 **/
#include <stdio.h>
#include <inttypes.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <parserclass.h>
#include <myB-Tree.h>

#define MAX_PROCESSED_DATA 30000
extern PMYB7_NODE myb7_tree_insert (PMYB7_TREE tree, int k);
extern void printmyb7_tree(PMYB7_TREE pTree);
int option ()
{
	fprintf (stdout, "type:\n"
			"add key - add a key\n"
			"print - print properties of active item\n"
			"del - delete a key\n"
			"q - quit application\n"); 
	return 0;
}


int stdparse (FILE *fp, PMYB7_TREE pTree)
{
	char tokenlist[] = {" \n"};
	char buffer[MAX_PROCESSED_DATA];
	int token = -1;
	int length = 0;

	int state = 0;
	unsigned char * newvar;
	PSTACK_PTR stack = newstackptr(); 
	int numvar = 0;
#define STATE_OTHER	0x00
#define STATE_ADD 	0x01
#define STATE_DELETE	0x02
	//option();
	token = stream_gettoken (fp, tokenlist, buffer, MAX_PROCESSED_DATA, &length);
	state = STATE_OTHER;
	while (token != -1)
	{
		switch (tokenlist[token])
		{
		case ' ':
		case '\n':
			trimstring (buffer);
			if (!strcmp (buffer, "q"))
			{
				fprintf (stdout, "bye...\n");
				return 0;
			}
			else if (!strncmp (buffer, "help", 3))
			{
				option();
				state = STATE_OTHER; 
			}
			else if (!strncmp (buffer, "add", 3))
			{
				state = STATE_ADD; 
			}
			else if (!strncmp (buffer, "delete", 6))
			{
				state = STATE_DELETE; 
			}
			else if (!strncmp (buffer, "del", 3))
			{
				state = STATE_DELETE; 
			}
			else if (!strncmp (buffer, "print", 5))
			{
				printmyb7_tree(pTree);
				state = STATE_OTHER; 
				break;
			}
			else if (strlen(buffer))
			{
				if ((numvar ++)	> MAX_STACKPTR)
				{
					fprintf (stderr, "stack overflow\n");
					continue;
				}
				newvar = (unsigned char *)  calloc (length +1, 1);
				if (!newvar)
				{
					fprintf (stderr, "out of memory\n");
					continue;
				}
				numvar ++;
				memcpy (newvar, buffer, length);
				stack->push (stack, newvar); 
			} 
			if (tokenlist[token] == ' ')
				break;
			char * a;
			int b;
			switch (state)
			{
			case STATE_ADD:
				if (stack->top < 0)
					break;
				do
				{
					a = stack->pop(stack);
					b = atoi (a);
					free (a);
					if (b < 0)
						fprintf (stderr, "value <0 %d ignored\n", b);
					else
					{
						myb7_tree_insert(pTree, b);
					}

				} while (stack->top>=0);
				break;
			case STATE_DELETE:
				if (stack->top < 0)
					break;
				do
				{
					a = stack->pop(stack);
					b = atoi (a);
					free (a);
					myb7_tree_del(pTree, b);

				} while (stack->top>=0);
				break;
			case STATE_OTHER:
				while (stack->top>=0)
				{
					a = stack->pop(stack);
					free (a);

				}
				break;
			default:
				break;
			}
			numvar = 0;
			state = STATE_OTHER;
		break;
		default:
			break;
		}

		token = stream_gettoken (fp, tokenlist, buffer, MAX_PROCESSED_DATA, &length);
	}
	return 0;

}

int main (int argc, char **argv)
{
	PMYB7_TREE pTree = myb7tree_new ("first_tree");
	stdparse (stdin, pTree);
	exit (EXIT_SUCCESS);
}
