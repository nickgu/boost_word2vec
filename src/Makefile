

CC = g++

#Using -Ofast instead of -O3 might result in faster code, but is supported only by newer GCC versions
CFLAGS = -pthread -O3 -Wall 

# for debug.
CFLAGS_DEBUG = -lm -pthread -O0 -Wall -g

LIBS= -lm

INCLUDES= -I../include

OBJECTS=word2vec_dict.o
TARGETS= word2vec \
		word2phrase \
		distance \
		word-analogy \
		compute-accuracy \
		word2vec_gsm \
		word2vec_predict \
		dump_target_vector \
		syn0_distance



all: $(OBJECTS) $(TARGETS)

$(OBJECTS): %.o:%.cc
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) -c $^ -o $@ $(CFLAGS) $(INCLUDES)

word2vec : word2vec.c
	$(CC) word2vec.c -o word2vec $(CFLAGS) $(INCLUDES)

word2vec_gsm : word2vec_gsm.cc
	$(CC) word2vec_gsm.cc -o word2vec_gsm $(CFLAGS) $(LIBS) $(INCLUDES)

word2vec_predict : word2vec_predict.cc $(OBJECTS)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(INCLUDES)

syn0_distance : syn0_distance.cc $(OBJECTS)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(INCLUDES)

dump_target_vector : dump_target_vector.cc $(OBJECTS)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(INCLUDES)

word2phrase : word2phrase.c
	$(CC) word2phrase.c -o word2phrase $(CFLAGS) $(LIBS) $(INCLUDES)
distance : distance.c
	$(CC) distance.c -o distance $(CFLAGS) $(LIBS) $(INCLUDES)
word-analogy : word-analogy.c
	$(CC) word-analogy.c -o word-analogy $(CFLAGS) $(LIBS) $(INCLUDES)
compute-accuracy : compute-accuracy.c
	$(CC) compute-accuracy.c -o compute-accuracy $(CFLAGS) $(LIBS) $(INCLUDES)

clean:
	rm -rf $(OBJECTS) $(TARGETS)
