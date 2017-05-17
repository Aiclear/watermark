CFLAGS=-g -Wall
CC=gcc

bmp_mark: bmp_mark.c
	$(CC) $(CFLAGS) -o $@ $^
