

export CC=g++
#CC=~/.jumbo/opt/gcc48/bin/g++

#Using -Ofast instead of -O3 might result in faster code, but is supported only by newer GCC versions
CFLAGS = -pthread -O3 -Wall -fPIC -shared

#DEBUG_CFLAGS= -lm -pthread -O0 -Wall -g

LDFLAGS= -pthread -lm -fPIC

# for debug.
BIN_FLAGS= -pthread -O3 -Wall 

LIBS= -lm

INCLUDES= -I../include\
		  -I/home/users/gusimiu/.jumbo/include/python2.7/

OBJECTS=word2vec_dict.o term2query_dict.o

TARGETS= word2vec \
		word2phrase \
		distance \
		word-analogy \
		compute-accuracy \
		word2vec_gsm \
		word2vec_predict \
		dump_target_vector \
		syn0_distance \
		convert_word2vec_term2query \
		test_term2query \
		py_term2query_dict.so  \
		py_lsh.so

all: $(OBJECTS) $(TARGETS)
	@echo -e "\033[40;32;1m --> MAKE ALL OVER! <-- \033[0m"

$(OBJECTS): %.o:%.cc
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) -c $^ -o $@ $(CFLAGS) $(INCLUDES)

word2vec : word2vec.c
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) word2vec.c -o word2vec $(CFLAGS) $(INCLUDES)

word2vec_gsm : word2vec_gsm.cc
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) word2vec_gsm.cc -o word2vec_gsm $(CFLAGS) $(LIBS) $(INCLUDES)

word2vec_predict : word2vec_predict.cc $(OBJECTS)
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(INCLUDES)

convert_word2vec_term2query : convert_word2vec_term2query.cc $(OBJECTS)
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(INCLUDES)

test_term2query : test_term2query.cc $(OBJECTS)
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(INCLUDES)

syn0_distance : syn0_distance.cc $(OBJECTS)
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(INCLUDES)

dump_target_vector : dump_target_vector.cc $(OBJECTS)
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(INCLUDES)

py_term2query_dict.so: py_term2query_dict.cc $(OBJECTS) lsh.cc
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $@ $(filter %.cpp %.o %.c %.cc, $^)

py_lsh.so: py_lsh.cc lsh.cc $(OBJECTS)
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $@ $(filter %.cpp %.o %.c %.cc, $^)

word2phrase : word2phrase.c
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) word2phrase.c -o word2phrase $(CFLAGS) $(LIBS) $(INCLUDES)
distance : distance.c
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) distance.c -o distance $(CFLAGS) $(LIBS) $(INCLUDES)
word-analogy : word-analogy.c
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) word-analogy.c -o word-analogy $(CFLAGS) $(LIBS) $(INCLUDES)
compute-accuracy : compute-accuracy.c
	@echo -e "\033[40;32;1m --> make : [[" $@ " ]] <-- \033[0m"
	$(CC) compute-accuracy.c -o compute-accuracy $(CFLAGS) $(LIBS) $(INCLUDES)

clean:
	rm -rf $(OBJECTS) $(TARGETS) *~ .*.swp *.pyc ../include/*~ ../include/.*.swp ../include/*.pyc
