FLAGS=-Wall -ggdb3 -I .
CC=gcc ${FLAGS}

btree : parserclass.c myB-Tree.c myB-TreeTest.c
	$(CC) parserclass.c myB-Tree.c myB-TreeTest.c -o myB-TreeTest 

btreegen : parserclass.c my_btree_generic.c
	$(CC) my_btree_generic.c -Wall -ggdb3 -o my_btree_generic

internal: parserclass.c myB-Tree.c myB-TreeInternal.c
	$(CC) parserclass.c myB-Tree.c myB-TreeInternal.c -o myB-TreeInternal

all: btree internal
