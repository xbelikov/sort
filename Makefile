all:
	g++ main.cpp extsort.cpp logger.cpp -o sort -std=c++14 -D_NDEBUG -O3 -lpthread
debug:
	g++ main.cpp extsort.cpp logger.cpp -o sort -std=c++14 -g
zip:
	zip code.zip extsort.cpp extsort.h logger.cpp logger.h chunk_value.h main.cpp
