#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <sstream>

using namespace std;

//void readBlock(ifstream* f, int begin, int end, vector<int>* buffer);
//void sortBlock(vector<int>* buffer);
//void writeBlock(ofstream* f, vector<int>* buffer);
//void protoSort();         

void externalMergeSort();
int getFileSize(ifstream* f);
int splitFile(ifstream* f, char* buff, int filesize, int bufsize);
int comp(const void* p1, const void* p2);
void saveBuff(int chunk, char* buff, int buffsize);
int merge(ofstream* of, char* buff, int bufsize, int chunks, int chunkOffset);
void viewFile(string filename);
void viewBuff(char* buff, int bufsize);

int main(int argc, char* argv[])
{
	cout << "Programm start" << endl;
	time_t startTime = time(NULL);
  	time_t endTime;
  	double seconds;

  	//viewFile("bigfile.data");
  	externalMergeSort();
  	
  	// viewFile("0");
  	// viewFile("1");
  	// viewFile("2");
  	// viewFile("3");
  	// viewFile("4");
  	// viewFile("5");
  	// viewFile("6");
  	// viewFile("output");
	
	endTime = time(NULL);
	seconds = difftime(endTime, startTime);

	cout << "Programm stop, time: " << seconds << " sec." << endl; 

	return 0;
}

void viewFile(string filename)
{
	ifstream f(filename);

	cout << "---------------------------" << endl;
	cout << "File: " << filename << endl; 
	while (f.tellg() > -1) {
		int number;
		char bytes[4];

		f.read(bytes, sizeof(number));

		if (f.tellg() < 0)
			break;

		number = *reinterpret_cast<int*> (bytes);
		cout << number << " ";
	}

	f.close();
	cout << endl << "----------------------" << endl;;
}

void viewBuff(char* buff, int bufsize)
{
	for (auto i = 0; i < bufsize; i+=4) {
		int number = *reinterpret_cast<int*>(buff + i); 
		cout << number << " ";
	}

	cout << endl;
}


void externalMergeSort()
{	
	cout << "External Merge Sort" << endl;

	auto filename = "bigfile.data";

	ifstream inputFile(filename, ios::binary);
  	auto filesize = getFileSize(&inputFile);

  	auto bufsize = 13000000 * 4;

  	if (filesize < bufsize) {
  		bufsize = filesize;
  	}

  	char* buff = new char[bufsize];

	auto chunkOffset = 0;
  	auto chunks = splitFile(&inputFile, buff, filesize, bufsize);
  	
  	inputFile.close();

	auto outputFilename = "output";
	ofstream of(outputFilename, ios::binary);
	
	auto n = bufsize / chunks;
  	while (n % 4) {
  		n++;
  	}

	while (chunkOffset < bufsize)
	{
  		merge(&of, buff, bufsize, chunks, chunkOffset);
  		chunkOffset += n;
  	}

	of.close();

  	delete[] buff;
}

int getFileSize(ifstream* f)
{
	int filesize = 0;

	if (f->is_open()) {
		f->seekg(0, ios::end);
		filesize = f->tellg();
		f->seekg(0);
	}

	return filesize;
}

int splitFile(ifstream* f, char* buff, int filesize, int bufsize)
{
	auto n = bufsize / sizeof(int);

	auto chunks = filesize / bufsize;

	if (filesize % bufsize) {
		chunks++;	
	}
	
	//cout << "Split File " << bufsize << " " << filesize << " " << chunks << endl;

	for (auto i = 0; i < chunks; i++) {
		f->read(buff, bufsize);

		auto nread = f->gcount();
		//cout << "Read " << nread << " bytes (" << nread / sizeof(int) << " ints)" << endl;

		qsort(buff, nread / sizeof(int), sizeof(int), comp);
		saveBuff(i, buff, nread);
	}

	return chunks;
}

int comp(const void* p1, const void* p2)
{
	int* i1;
	int* i2;

	i1 = (int*) p1;
	i2 = (int*) p2;

	return *i1 - *i2;
}

void saveBuff(int chunk, char* buff, int bufsize)
{
	stringstream ss;
	ss << chunk;
	string fn(ss.str());

	//cout << "Save buff to " << fn << endl;
	ofstream of(fn, ios::binary);

	of.write(buff, bufsize);
	
	of.close();
}

int merge(ofstream* of, char* buff, int bufsize, int chunks, int chunkOffset)
{
	//cout << "Merge: " << chunks << ", current: " << chunkOffset << endl;

	auto slice = bufsize / chunks;

	while (slice % 4) {
		slice++;
	}

	auto bufOffset = 0;

	//cout << "Slice: " << slice << " (" << slice / 4 << ")" << endl;

	for (auto c = 0; c < bufsize; c++) {
		buff[c] = 0;
	}

	for (auto i = 0; i < chunks; i++) {
		stringstream ss;
		ss << i;
		string filename(ss.str());
		ifstream f(filename);

		f.seekg(chunkOffset);

		if (f.tellg() < 0) {
			continue;
		}

		auto nread = 0;

		for (auto j = 0; j < slice / 4; j++) {
			f.read(buff + nread + bufOffset, 4);

			nread += f.gcount();
		}

		bufOffset += nread;
		
		f.close();

		//viewBuff(buff, bufOffset);

		//cout << "File #" << i << "Chunk #" << chunkOffset << ":Read: " << bufOffset << endl;
	}

	auto n = bufOffset / sizeof(int);
	//cout << "Sorting " << n << " elements" << endl;
	qsort(buff, n, sizeof(int), comp);
	of->write(buff, bufOffset);

	return bufOffset;
}