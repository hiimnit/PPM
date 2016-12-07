GXX = g++
FLAGS =-O1 -Wall -Werror -lm -o main -std=c++11
LIBS = -fopenmp
INFILE = main.cpp
OUTFILE = main
CACHE = cachegrind.out
ARGS = vit_normal.ppm

all: conv

conv: $(INFILE)
	$(GXX) $(LIBS) $(FLAGS) $(INFILE) -o $(OUTFILE) -g
clean:
	rm $(OUTFILE) $(CACHE)
run:
	./$(OUTFILE) $(ARGS)
cache:
	valgrind --tool=cachegrind --I1=32768,8,64 --D1=32768,8,64 --LL=1048576,16,64 --cachegrind-out-file=$(CACHE) ./main $(ARGS)
cg:
	cg_annotate $(CACHE) $(PWD)/main.cpp