#include "logger.h"
#include <iostream>

using namespace std;

Logger::Logger(bool enable)
{
	if (!enable) {
		cout.rdbuf(NULL);
	}
}

Logger* Logger::l(string val)
{
	cout << val;
	return this;
}

Logger* Logger::l(int val)
{
	cout << val;
	return this;
}

Logger* Logger::l(double val)
{
	cout << val;
	return this;
}

Logger* Logger::l(float val)
{
	cout << val;
	return this;
}

Logger* Logger::l(long val)
{
	cout << val;
	return this;
}


Logger* Logger::end()
{
	cout << endl;
	return this;
}