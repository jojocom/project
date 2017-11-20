OBJS1 	= main.o trie.o list.o
OBJS2 	= unit_test.o trie.o list.o
OUT1	= ngrams
OUT2	= unit
CC		= g++
FLAGS	= -c -Wall

all: main unit test

# main

main.o: main.cpp
	$(CC) $(FLAGS) main.cpp

trie.o:  trie.cpp
	$(CC) $(FLAGS) trie.cpp

list.o:  list.cpp
	$(CC) $(FLAGS) list.cpp

main: $(OBJS1)
	$(CC) $(OBJS1) -o $(OUT1)

unit_test.o: unit_test.cpp
	$(CC) $(FLAGS) unit_test.cpp

unit: $(OBJS2)
	$(CC) $(OBJS2) -o $(OUT2)

test.o: test.cpp
	$(CC) $(FLAGS) test.cpp

test: test.o
	$(CC) test.o -o test

# clean up

clean:
	rm -f $(OBJS1) $(OUT1) unit_test.o $(OUT2)
