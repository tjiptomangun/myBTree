FLAGS=-Wall -ggdb3 -I .
CC=gcc ${FLAGS}

all: btree internal btreegen btreestringi

btree : parserclass.c myB-Tree.c myB-TreeTest.c
	$(CC) parserclass.c myB-Tree.c myB-TreeTest.c -o myB-TreeTest 

btreegen : parserclass.c my_btree_string.c
	$(CC) my_btree_string.c -Wall -ggdb3 -D_STRING_NOINPUT_ -o my_btree_string

btreestringi : parserclass.c my_btree_string.c
	$(CC) my_btree_string.c -Wall -ggdb3 -D_STRING_INPUT_ -o btreestringi

internal: parserclass.c myB-Tree.c myB-TreeInternal.c
	$(CC) parserclass.c myB-Tree.c myB-TreeInternal.c -o myB-TreeInternal

