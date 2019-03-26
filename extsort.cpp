#include "extsort.h"

using namespace std;

Extsort::Extsort(string inputFilename, string outputFilename, int bufferSize, bool useLogger): bufferSize(bufferSize), logger(useLogger)
{
	this->inputFile.open(inputFilename, ios::in | ios::binary);
	this->outputFile.open(outputFilename, ios::out | ios::binary);
}

void Extsort::run()
{
	this->chunks = this->split();
	inputFile.close();

	this->merge();

	outputFile.close();
}

int Extsort::split()
{
	priority_queue<int, vector<int>, std::greater<int>> pqBuffer;

	auto elementSize = sizeof(int);
	auto inputSize = this->getFileSize(this->inputFile);
	int nElementsInBuffer = this->bufferSize / elementSize;
	int chunksNumber = this->getNumberOfChunks(this->bufferSize, inputSize);

	this->elements = inputSize / elementSize;
	this->parts.reserve(chunksNumber);

	this->logger.l("Split File ")->l(this->bufferSize)->l(" ")->l(inputSize)->l(" ")->l(chunksNumber)->end();

	for (auto i = 0; i < chunksNumber; i++) {
		auto partFileName = to_string(i);

		for (auto j = 0; j < nElementsInBuffer; j++) {
			int number;
			this->inputFile.read(reinterpret_cast<char*>(&number), elementSize);
			
			if (this->inputFile.eof()) {
				break;
			}

			pqBuffer.push(number);
		}

		this->parts.emplace_back(partFileName, ios::out | ios::in | ios::binary | ios::trunc);
		this->savePart(i, partFileName, pqBuffer);
	}

	return chunksNumber;
}

struct ChunkValue
{
	int value;
	int chunk;
};

class ChunkValueCpm
{
public:
	bool operator()(const ChunkValue& lhs, const ChunkValue& rhs) const
	{
		return lhs.value > rhs.value;
	}
};

int chunkValueComp(const void* p1, const void* p2)
{
	ChunkValue* cv1 = nullptr;
	ChunkValue* cv2 = nullptr;

	cv1 = (ChunkValue*) p1;
	cv2 = (ChunkValue*) p2;

	return cv1->value - cv2->value;
}

void Extsort::merge()
{
	this->logger.l("Merge: ")->l(this->chunks)->end();
	
	auto chunksSize = this->getChunksSize(this->chunks);
	auto chunkValueSize = sizeof(ChunkValue);
	auto nChunkValues = this->bufferSize / chunkValueSize;

	auto elementSize = sizeof(int);
	int nSlices = this->elements;

	auto nMergePasses = this->getNumberOfPasses(nChunkValues, this->chunks);

	vector<int> chunksPointers(this->chunks);
	vector<bool> chunksForRead(this->chunks);

	ChunkValue currentValue;
	priority_queue<ChunkValue, vector<ChunkValue>, ChunkValueCpm> pqBuffer;

	//Пройдемся по всем числам во временных файлах
	for (auto i = 0; i < nSlices; i++) {
		this->logger.l("----------------------------------------")->end();
		this->logger.l("Slice: ")->l(i + 1)->l(" of ")->l(nSlices)->end();

		auto nChunksInPass = this->getNumberOfChunksInPass(this->chunks, nMergePasses);
		auto currentMergePass = 0;
		auto currentChunk = 0;

		//Количество временных файлов может быть больше, чем может вместить в себя
		//буфер, поэтому может понадобиться сделать несколько проходов
		while (currentMergePass < nMergePasses) {
			this->logger.l("Pass: ")->l(currentMergePass + 1)->l(" of ")->l(nMergePasses)->end();
		
			//Пройдемся по временным файлам согласно текущего указателя 
			//и соберем подвыборки для дальнейшей сортировки и определения наименьшего
			while (currentChunk < this->chunks) {
				auto currentPointer = parts[currentChunk].tellg();

				if (i == 0 || chunksForRead[currentChunk] && !parts[currentChunk].eof()) {
					chunksForRead[currentChunk] = false;

					this->logger.l("Chunk: ")->l(currentChunk);
					this->logger.l(" tellg: ")->l((int) currentPointer);

					//Храним не только значение, но и позицию, чтобы потом можно было легко передвинуть указатель
					ChunkValue chv;
					chv.chunk = currentChunk;

					parts[currentChunk].read(reinterpret_cast<char*>(&chv.value), elementSize);

					if (parts[currentChunk].eof()) {
						this->logger.l(" EOF")->end();
						currentChunk++;
						continue;
					}

					pqBuffer.push(chv);
					this->logger.l(", value: ")->l(chv.value)->end();
				}

				currentChunk++;

				//Выходим, если обработали последний элемент прохода, когда слияние многопроходное
				if (currentChunk - 1 > 0 && nMergePasses > 1 && ((currentChunk - 1) % nChunksInPass) == 0) {
					this->logger.l("break")->end();
					break;
				}
			}

			ChunkValue minChunkValue = pqBuffer.top(); 
			
			if (
				(currentMergePass == 0) ||
				((nMergePasses - currentMergePass == 1) && (minChunkValue.value < currentValue.value)) ||
				(nMergePasses - currentMergePass == 1) && nSlices - i == 1
			) {
				this->logger.l("Merge pass: ")->l(currentMergePass)->end();
				this->logger.l("Merge passes - merge pass: ")->l(nMergePasses - currentMergePass)->end();
				
				this->logger.l("Values: ")
							->l("chunk ")->l(minChunkValue.chunk)
							->l(", min ")->l(minChunkValue.value)
							->l(", chunk ")->l(currentValue.chunk)
							->l(" cur ")->l(currentValue.value)->end();
				
				currentValue = minChunkValue;
			}

			this->logger.l("Reset buffer offset, set next merge pass...")->end();
			currentMergePass++;
		}

		this->logger.l("Move chunk pointer forward: ")->l(currentValue.chunk)->end();

		//Перемещаем указатель на следующий элемент в выбранном блоке
		pqBuffer.pop();
		//chunksPointers[currentValue.chunk] += elementSize;
		chunksForRead[currentValue.chunk] = true;

		this->outputFile.write(reinterpret_cast<char*>(&currentValue.value), elementSize);
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
	this->parts[index].write(data, length);
	this->parts[index].seekg(0);

	this->parts[index].flush();
	this->parts[index].clear();
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