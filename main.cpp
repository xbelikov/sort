#include <iostream>
#include <ctime>

#include "extsort.h"

using namespace std;

int main(int argc, char* argv[])
{
	string params;
	bool logEnabled = false;
	auto bufferSize = 6500000 * sizeof(int);
	auto threads = 3;

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

	Extsort extsort("input", "output", bufferSize, threads);
	extsort.run();

	return 0;
}