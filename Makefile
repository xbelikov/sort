all:
	g++ main.cpp extsort.cpp logger.cpp -o sort -std=c++14 -D_NDEBUG -O3 -lpthread
debug:
	g++ main.cpp extsort.cpp logger.cpp -o sort -std=c++14 -g
