#ifndef EXTSORT_H
#define EXTSORT_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>

#include "chunk_value.h"
#include "logger.h"

using namespace std;

class Extsort
{
public:
	Extsort(string inputFilename, string outputFilename, int bufferSize, bool useLogger);
	void run();

private:
	Logger logger;
	fstream inputFile;
	fstream outputFile;
	int bufferSize;

	vector<fstream> parts;

	int chunks;
	int elements;

	int split();
	void merge();
	int getFileSize(fstream& file);
	int getNumberOfChunks(int bufferSize, int inputFileSize);
	int getNumberOfChunksInPass(int numberOfChunks, int numberOfPasses);
	int getNumberOfPasses(int nChunkValues, int nChunks);
	int getChunkLength(int bufferSize, int chunksNumber);
	int getChunksSize(int chunksNumber);
	void savePart(int index, string fileName, char* data, int length);
	void savePart(int index, string fileName, priority_queue<int, vector<int>, std::greater<int>>& pqBuffer);
	static int comp(const void* p1, const void* p2);
};

#endif //EXTSORT_H