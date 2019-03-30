#include "extsort.h"

using namespace std;

Extsort::Extsort(string inputFileName, string outputFileName, int bufferSize, int threads): 
	inputFileName(inputFileName), outputFileName(outputFileName), bufferSize(bufferSize), threads(threads)
{}

void Extsort::run()
{
	this->inputFile.open(this->inputFileName, ios::in | ios::binary);
	this->outputFile.open(this->outputFileName, ios::out | ios::binary);

	this->split();
	this->merge();
}

int Extsort::split()
{
	auto elementSize = sizeof(unsigned int);
	auto inputSize = this->getFileSize(this->inputFile);
	int nElementsInBuffer = this->bufferSize / elementSize;
	this->elements = inputSize / elementSize;
	this->chunks = this->getNumberOfChunks(this->bufferSize, inputSize);

	vector<future<void>> futures;

	//auto nChunks = this->chunks;
	//auto partBufferSize = this->bufferSize;
	//auto filaName = this->inputFileName;

	for (auto i = 0; i < this->chunks; i++) {
		futures.push_back(
			async([this, i, inputSize] {
				this->processPart(this->inputFileName, i, this->chunks, this->bufferSize, inputSize);
			})
		);

		if ((i + 1) % this->threads == 0) {
			for (auto j = 0; j < this->threads; j++) {
				futures[i - j].wait();
			}
		} 
	}

	return this->chunks;
}

void Extsort::processPart(string& fileName, unsigned long chunk, unsigned long nChunks, int bufferSize, unsigned long inputLength)
{
	ifstream ifs(fileName, ios::in | ios::binary);

	unsigned int elementSize = sizeof(unsigned int);
	unsigned long leftBytes = inputLength - (unsigned long) ifs.tellg();
	bufferSize = (leftBytes < bufferSize)? leftBytes : bufferSize;
	int nElements = bufferSize / elementSize;

	vector<unsigned int> buffer(nElements);
	auto partFileName = to_string(chunk);
	auto nread = 0;

	ofstream part(partFileName, ios::out | ios::binary | ios::trunc);

	ifs.seekg(chunk * bufferSize, ios::cur);
	ifs.read(reinterpret_cast<char*>(buffer.data()), bufferSize);
	nread = ifs.gcount();

	int lastElement = nread / elementSize;
	sort(begin(buffer), begin(buffer) + lastElement);
	part.write(reinterpret_cast<char*>(buffer.data()), nread);
}

void Extsort::merge()
{
	auto elementSize = sizeof(unsigned int);
	unsigned long nSlices = this->elements;
	unsigned int nElementsInBuffer = this->bufferSize / elementSize;

	vector<unsigned long> chunksPointers(this->chunks);
	vector<bool> chunksForRead(this->chunks, true);
	vector<unsigned int> outputBuffer;

	ChunkValue currentValue;
	priority_queue<ChunkValue, vector<ChunkValue>, ChunkValueCpm> pqBuffer;

	this->parts.reserve(this->chunks);

	for (unsigned int chunk = 0; chunk < this->chunks; chunk++) {
		auto partFileName = to_string(chunk);
		this->parts.emplace_back(partFileName, ios::in | ios::binary);
	}

	for (unsigned long i = 0; i < nSlices; i++) {
		unsigned int currentChunk = 0;

		while (currentChunk < this->chunks) {
			if (i == 0 || chunksForRead[currentChunk] && !parts[currentChunk].eof()) {
				chunksForRead[currentChunk] = false;

				ChunkValue chv;
				chv.chunk = currentChunk;

				parts[currentChunk].read(reinterpret_cast<char*>(&chv.value), elementSize);

				if (parts[currentChunk].eof()) {
					currentChunk++;
					continue;
				}

				pqBuffer.push(chv);
			}
			
			currentChunk++;
		}

		ChunkValue minChunkValue = pqBuffer.top();
		currentValue = minChunkValue;

		pqBuffer.pop();

		chunksPointers[currentValue.chunk] += elementSize;
		chunksForRead[currentValue.chunk] = true;

		outputBuffer.push_back(currentValue.value);

		if (
			outputBuffer.size() == nElementsInBuffer ||
			i == nSlices - 1
		) {
			this->outputFile.write(reinterpret_cast<char*>(outputBuffer.data()), outputBuffer.size() * elementSize);
			outputBuffer.clear();
		}
	}
}

unsigned long Extsort::getFileSize(fstream& file)
{
	unsigned long filesize = 0;

	if (file.is_open()) {
		file.seekg(0, ios::end);
		filesize = file.tellg();
		file.seekg(0);
	}

	return filesize;
}

unsigned long Extsort::getNumberOfChunks(int bufferSize, unsigned long inputFileSize)
{
	unsigned int chunks = inputFileSize / bufferSize;

	if (inputFileSize % bufferSize) {
		chunks++;
	}

	return chunks;
}

int Extsort::comp(const void* p1, const void* p2)
{
	int* i1;
	int* i2;

	i1 = (int*) p1;
	i2 = (int*) p2;

	return *i1 - *i2;
}