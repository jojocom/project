OBJS1 	= main.o trie.o list.o hash_functions.o heap.o jobScheduler.o
OBJS2 	= unit_test.o trie.o list.o hash_functions.o heap.o jobScheduler.o
OUT1	= ngrams
OUT2	= unit
CC		= g++
FLAGS	= -c -Wall -O3

all: main unit

# main

main.o: main.cpp
	$(CC) $(FLAGS) main.cpp

trie.o:  trie.cpp
	$(CC) $(FLAGS) trie.cpp

list.o:  list.cpp
	$(CC) $(FLAGS) list.cpp

hash_functions.o: hash_functions.cpp
	$(CC) $(FLAGS) hash_functions.cpp

heap.o: heap.cpp
	$(CC) $(FLAGS) heap.cpp

jobScheduler.o: jobScheduler.cpp
	$(CC) $(FLAGS) jobScheduler.cpp

main: $(OBJS1)
	$(CC) $(OBJS1) -o $(OUT1) -lpthread

unit_test.o: unit_test.cpp
	$(CC) $(FLAGS) unit_test.cpp

unit: $(OBJS2)
	$(CC) $(OBJS2) -o $(OUT2) -lpthread

# temp.o: temp.cpp
# 	$(CC) $(FLAGS) temp.cpp
#
# temp: temp.o
# 	$(CC) temp.o jobScheduler.o  -o temp -lpthread

# clean up

clean:
	rm -f $(OBJS1) $(OUT1) unit_test.o $(OUT2) temp.o
