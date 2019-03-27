#include <iostream>
#include <ctime>

#include "extsort.h"

using namespace std;

void viewFile(string filename, int logEnabled);
void viewBuff(char* buff, int bufsize, int logEnabled);

int main(int argc, char* argv[])
{
	string params;
	bool logEnabled = false;
	auto bufferSize = 6500000 * sizeof(int); //13000000 * sizeof(int);

	for (auto i = 1; i < argc; i++) {
		params = argv[i];

		if (params == "-l") {
			logEnabled = true;
		}

		if (params == "-b" && argc > i + 1) {
			string value = argv[i + 1];
			bufferSize = stoi(value);
		}
	}

	Extsort extsort("input", "output", bufferSize, logEnabled);

	cout << "Programm start" << endl;
	time_t startTime = time(NULL);
	time_t endTime;
	double seconds;

	//viewFile("input", logEnabled);

	extsort.run();

	//viewFile("output", logEnabled);

	endTime = time(NULL);
	seconds = difftime(endTime, startTime);

	cout << "Programm stop, time: " << seconds << " sec." << endl; 

	return 0;
}

void viewFile(string filename, int logEnabled)
{
	if (!logEnabled) {
		return;
	}

	ifstream f(filename);

	cout << "---------------------------" << endl;
	cout << "File: " << filename << endl; 
	int pos = (int) f.tellg();

	while (pos > -1) {
		int number;
		char bytes[4];

		cout << " (" << pos << ") ";
		f.read(bytes, sizeof(number));
		pos = (int) f.tellg();

		if (pos < 0)
			break;

		number = *reinterpret_cast<int*> (bytes);
		cout << number << " ";
	}

	f.close();
	cout << endl << "----------------------" << endl;;
}

void viewBuff(char* buff, int bufsize, int logEnabled)
{
	if (!logEnabled) {
		return;
	}

	for (auto i = 0; i < bufsize; i+=4) {
		int number = *reinterpret_cast<int*>(buff + i); 
		cout << number << " ";
	}

	cout << endl;
}