#include "extsort.h"

using namespace std;

Extsort::Extsort(string inputFilename, string outputFilename, int bufferSize, bool useLogger): bufferSize(bufferSize), logger(useLogger)
{
	this->inputFile.open(inputFilename, ios::in | ios::binary);
	this->outputFile.open(outputFilename, ios::out | ios::binary);
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

	int partBufferSize = this->bufferSize / 2;
	this->chunks = this->getNumberOfChunks(partBufferSize, inputSize);

	vector<future<void>> futures;

	for (auto i = 0; i < this->chunks; i++) {
		auto nChunks = this->chunks;

		futures.push_back(
			async([&, i, nChunks, partBufferSize] {
				this->processPart(i, nChunks, partBufferSize);
			})
		);
	}

	return this->chunks;
}

void Extsort::processPart(int chunk, int nChunks, int bufferSize)
{
	char* buffer = new char [bufferSize];
	auto partFileName = to_string(chunk);

	ifstream ifs("input", ios::in | ios::binary);

	ifs.seekg(chunk * bufferSize, ios::cur);
	ifs.read(buffer, bufferSize);
	auto nread = ifs.gcount();

	qsort(buffer, nread / sizeof(int), sizeof(int), Extsort::comp);
	this->savePart(chunk, partFileName, buffer, nread);
	delete[] buffer;
}

void Extsort::merge()
{
	auto chunksSize = this->getChunksSize(this->chunks);
	auto chunkValueSize = sizeof(ChunkValue);
	auto nChunkValues = this->bufferSize / chunkValueSize;

	auto elementSize = sizeof(int);
	int nSlices = this->elements;

	vector<int> chunksPointers(this->chunks);
	vector<bool> chunksForRead(this->chunks);

	ChunkValue currentValue;
	priority_queue<ChunkValue, vector<ChunkValue>, ChunkValueCpm> pqBuffer;

	int nElementsInBuffer = this->bufferSize / elementSize;
	int* outputBuffer = new int[nElementsInBuffer];
	auto outputBufferOffset = 0;

	this->parts.reserve(this->chunks);

	for (int chunk = 0; chunk < this->chunks; chunk++) {
		auto partFileName = to_string(chunk);
		this->parts.emplace_back(partFileName, ios::in | ios::binary);
	}

	//Пройдемся по всем числам во временных файлах
	for (auto i = 0; i < nSlices; i++) {
		auto currentChunk = 0;
		
		//Пройдемся по временным файлам согласно текущего указателя 
		//и соберем подвыборки для дальнейшей сортировки и определения наименьшего
		while (currentChunk < this->chunks) {
			auto currentPointer = parts[currentChunk].tellg();

			if (i == 0 || chunksForRead[currentChunk] && !parts[currentChunk].eof()) {
				chunksForRead[currentChunk] = false;

				//Храним не только значение, но и позицию, чтобы потом можно было легко передвинуть указатель
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

		//Перемещаем указатель на следующий элемент в выбранном блоке
		pqBuffer.pop();
		chunksPointers[currentValue.chunk] += elementSize;
		chunksForRead[currentValue.chunk] = true;

		outputBuffer[outputBufferOffset] = currentValue.value;
		outputBufferOffset++;

		if (
			outputBufferOffset == nElementsInBuffer ||
			i == nSlices - 1
		) {
			this->outputFile.write(reinterpret_cast<char*>(outputBuffer), outputBufferOffset * elementSize);
			outputBufferOffset = 0;
		}
	}

	delete[] outputBuffer;
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

int Extsort::getChunkLength(int bufferSize, int chunksNumber)
{
	auto length = bufferSize / chunksNumber;

  	while (length % sizeof(int)) {
  		length++;
  	}

  	return length;
}

int Extsort::getChunksSize(int chunksNumber)
{
	return chunksNumber * sizeof(int);
}

int Extsort::getNumberOfChunksInPass(int numberOfChunks, int numberOfPasses)
{
	int result = numberOfChunks / numberOfPasses;

	if (numberOfChunks % numberOfPasses) {
		result++;
	}

	return result;
}

int Extsort::getNumberOfPasses(int nChunkValues, int nChunks)
{
	auto result = nChunks / nChunkValues;

	if (!result) {
		result = 1;
	}

	return result;
}

void Extsort::savePart(int index, string fileName, char* data, int length)
{
	ofstream part(fileName, ios::out | ios::binary | ios::trunc);
	cout << "write " << length << endl;
	part.write(data, length);
	//this->parts[index].write(data, length);
	//this->parts[index].seekg(0);

	//this->parts[index].flush();
	//this->parts[index].clear();
}

void Extsort::savePart(int index, string fileName, priority_queue<int, vector<int>, std::greater<int>>& pqBuffer)
{
	while(pqBuffer.size()) {
		auto top = pqBuffer.top();
		this->parts[index].write(reinterpret_cast<char*>(&top), sizeof(int));
		pqBuffer.pop();
	}

	this->parts[index].seekg(0);
	this->parts[index].flush();
	this->parts[index].clear();
}

int Extsort::comp(const void* p1, const void* p2)
{
	int* i1;
	int* i2;

	i1 = (int*) p1;
	i2 = (int*) p2;

	return *i1 - *i2;
}