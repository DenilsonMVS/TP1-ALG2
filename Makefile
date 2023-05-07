
CXX = g++ -std=c++17
FLAGS = -Wall


all: bin bin/text_compressor

release: FLAGS += -O3 -DNDEBUG
release: all

bin:
	mkdir bin

bin/text_compressor: obj obj/main.o obj/argReader.o obj/compression.o obj/bits.o
	$(CXX) $(FLAGS) -Wall obj/main.o obj/argReader.o obj/compression.o obj/bits.o -o $@

obj:
	mkdir obj

obj/main.o: src/main.cpp
	$(CXX) $(FLAGS) -c $< -o $@

obj/argReader.o: src/argReader.cpp src/argReader.hpp
	$(CXX) $(FLAGS) -c $< -o $@

obj/compression.o: src/compression.cpp src/compression.hpp
	$(CXX) $(FLAGS) -c $< -o $@

obj/bits.o: src/bits.cpp src/bits.hpp
	$(CXX) $(FLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf bin
	rm -rf obj
