#ifndef LOGGER_H
#define LOGGER_H

#include <string>

using namespace std;

class Logger
{
public:
	Logger(bool enable);
	Logger* l(string val);
	Logger* l(int val);
	Logger* l(double val);
	Logger* l(float val);
	Logger* l(long val);
	Logger* end();
private:
	bool enable;
};

#endif //LOGGER_H