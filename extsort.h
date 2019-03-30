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

using namespace std;

class Extsort
{
public:
	Extsort(string inputFileName, string outputFileName, int bufferSize, int threads);
	void run();

private:
	string inputFileName;
	string outputFileName;
	fstream inputFile;
	fstream outputFile;
	int bufferSize;
	int threads;

	vector<fstream> parts;

	unsigned int chunks;
	unsigned long elements;

	unsigned int split();
	void merge();
	unsigned long getFileSize(fstream& file);
	unsigned long getNumberOfChunks(int bufferSize, unsigned long inputFileSize);
	void processPart(string& fileName, unsigned int chunk, unsigned int nChunks, int bufferSize, unsigned long inputLength);
	static int comp(const void* p1, const void* p2);
};

#endif //EXTSORT_H