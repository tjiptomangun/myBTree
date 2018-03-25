FLAGS=-Wall -ggdb3 -I .
CC=gcc ${FLAGS}

btree : parserclass.c myB-Tree.c myB-TreeTest.c
	$(CC) parserclass.c myB-Tree.c myB-TreeTest.c -o myB-TreeTest 

internal: parserclass.c myB-Tree.c myB-TreeInternal.c
	$(CC) parserclass.c myB-Tree.c myB-TreeInternal.c -o myB-TreeInternal

all: btree internal
