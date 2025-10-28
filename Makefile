# Macros
CC = gcc217
# CC = gcc217m
CFLAGS = 
# CFLAGS = -g
# CFLAGS = -D NDEBUG
# CFLAGS = -D NDEBUG -O

# Dependency rules for non-file targets
all: testsymtablelist testsymtablehash
clean:
	rm -f *.o testsymtablelist testsymtablehash

# Dependency rules for file targets
testsymtablelist: testsymtable.o symtablelist.o
	$(CC) -o testsymtablelist testsymtable.o symtablelist.o

testsymtablehash: testsymtable.o symtablehash.o
	$(CC) -o testsymtablehash testsymtable.o symtablehash.o

testsymtable.o: testsymtable.c symtable.h
	$(CC) -c testsymtable.c

symtablelist.o: symtablelist.c symtable.h
	$(CC) -c symtablelist.c

symtablehash.o: symtablehash.c symtable.h 
	$(CC) -c symtablehash.c
