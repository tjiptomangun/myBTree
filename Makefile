FLAGS=-Wall -ggdb3 -I .
CC=gcc ${FLAGS}

all: btree internal btreegen btreegeni

btree : parserclass.c myB-Tree.c myB-TreeTest.c
	$(CC) parserclass.c myB-Tree.c myB-TreeTest.c -o myB-TreeTest 

btreegen : parserclass.c my_btree_generic.c
	$(CC) my_btree_generic.c -Wall -ggdb3 -D_GENERIC_NOINPUT_ -o my_btree_generic

btreegeni : parserclass.c my_btree_generic.c
	$(CC) my_btree_generic.c -Wall -ggdb3 -D_GENERIC_INPUT_ -o btreegeni

internal: parserclass.c myB-Tree.c myB-TreeInternal.c
	$(CC) parserclass.c myB-Tree.c myB-TreeInternal.c -o myB-TreeInternal

