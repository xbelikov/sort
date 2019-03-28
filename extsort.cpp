#include "extsort.h"

using namespace std;

Extsort::Extsort(string inputFilename, string outputFilename, int bufferSize, int threads, bool useLogger): bufferSize(bufferSize), logger(useLogger)
{
	this->inputFile.open(inputFilename, ios::in | ios::binary);
	this->outputFile.open(outputFilename, ios::out | ios::binary);
	this->threads = threads;
}

void Extsort::run()
{
	this->split();
	inputFile.close();

	this->merge();

	outputFile.close();
}

int Extsort::split()
{
	auto elementSize = sizeof(int);
	auto inputSize = this->getFileSize(this->inputFile);
	int nElementsInBuffer = this->bufferSize / elementSize;
	this->elements = inputSize / elementSize;
	this->chunks = this->getNumberOfChunks(this->bufferSize, inputSize);

	vector<future<void>> futures;

	auto nChunks = this->chunks;
	auto partBufferSize = this->bufferSize;

	for (auto i = 0; i < this->chunks; i++) {
		futures.push_back(
			async([&, i, nChunks, partBufferSize, inputSize] {
				this->processPart(i, nChunks, partBufferSize, inputSize);
			})
		);

		if (i > 0 && i % this->threads == 0) {
			for (auto j = 0; j < this->threads; j++) {
				futures[i - j].wait();
			}
		} 
	}

	return this->chunks;
}

void Extsort::processPart(int chunk, int nChunks, int bufferSize, int inputLength)
{
	ifstream ifs("input", ios::in | ios::binary);

	int elementSize = sizeof(int);
	int leftBytes = inputLength - (int) ifs.tellg();
	bufferSize = (leftBytes < bufferSize)? leftBytes : bufferSize;
	int nElements = bufferSize / elementSize;

	vector<int> buffer(nElements);
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
	auto elementSize = sizeof(int);
	int nSlices = this->elements;
	int nElementsInBuffer = this->bufferSize / elementSize;

	vector<int> chunksPointers(this->chunks);
	vector<bool> chunksForRead(this->chunks);
	vector<int> outputBuffer;

	ChunkValue currentValue;
	priority_queue<ChunkValue, vector<ChunkValue>, ChunkValueCpm> pqBuffer;

	this->parts.reserve(this->chunks);

	//Откроем временные файлы на чтение
	for (int chunk = 0; chunk < this->chunks; chunk++) {
		auto partFileName = to_string(chunk);
		this->parts.emplace_back(partFileName, ios::in | ios::binary);
	}

	//Пройдемся по всем числам во временных файлах
	for (auto i = 0; i < nSlices; i++) {
		auto currentChunk = 0;
		
		//Добавим в очередь, если необходимо и сравним
		while (currentChunk < this->chunks) {
			auto currentPointer = parts[currentChunk].tellg();

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

int Extsort::getFileSize(fstream& file)
{
	int filesize = 0;

	if (file.is_open()) {
		file.seekg(0, ios::end);
		filesize = file.tellg();
		file.seekg(0);
	}

	return filesize;
}

int Extsort::getNumberOfChunks(int bufferSize, int inputFileSize)
{
	int chunks = inputFileSize / bufferSize;

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