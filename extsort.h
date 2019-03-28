#ifndef EXTSORT_H
#define EXTSORT_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <queue>
#include <future>
#include <algorithm>

#include "chunk_value.h"
#include "logger.h"

using namespace std;

class Extsort
{
public:
	Extsort(string inputFilename, string outputFilename, int bufferSize, int threads, bool useLogger);
	void run();

private:
	Logger logger;
	fstream inputFile;
	fstream outputFile;
	int bufferSize;
	int threads;

	vector<fstream> parts;

	int chunks;
	int elements;

	int split();
	void merge();
	int getFileSize(fstream& file);
	int getNumberOfChunks(int bufferSize, int inputFileSize);
	void processPart(int chunk, int nChunks, int bufferSize, int inputLength);
	static int comp(const void* p1, const void* p2);
};

#endif //EXTSORT_H