#pragma once
#include "Windows.h"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include "Monitor.h"
#include "Mutex.h"
#include "ThreadPool.h"

#define DIVIDER (' ')

class StringSorter
{

public:

	StringSorter(int threadCount);
	void sort(LPSTR inputFile, LPSTR outputFile);
	~StringSorter();

private:

	int threadCount;
	int workThreadCount;
	Mutex sorterMutex; // for threadCount
	Monitor monitor;

	struct ThreadParamStruct
	{
		StringSorter* sorter;
		std::vector<std::string>* stringVector;
	};

	static DWORD _stdcall threadSortVector(LPVOID paramStruct);
	static void sortStringVector(std::vector<std::string>* stringVector);

	std::vector<std::vector<std::string>*>* receiveStringVectors(LPSTR inputFile);
	std::vector<std::string>* receiveStringsFromFile(LPSTR inputFile);
	std::vector<std::vector<std::string>*>* divideVector(std::vector<std::string>* sourceVector, int count);
	bool writeStringsToFile(LPSTR outputFile, std::vector<std::string>* stringVector);
	std::vector<std::string>* mergeSortVectors(std::vector<std::vector<std::string>*>* vectors);

protected:

	CRITICAL_SECTION critSection;
};

