#include "stdafx.h"
#include "StringSorter.h"

StringSorter::StringSorter(int threadCount)
{
	this->threadCount = threadCount;
}

void StringSorter::sort(LPSTR inputFile, LPSTR outputFile)
{
	std::vector<std::vector<std::string>*>* stringVectors = receiveStringVectors(inputFile);
	
	ThreadPool threadPool(this->threadCount);
	
	TaskStruct* tasks = new TaskStruct[stringVectors->size()];
	ThreadParamStruct* params = new ThreadParamStruct[stringVectors->size()];
	workThreadCount = 0;
	
	for (int i = 0; i < stringVectors->size(); i++)
	{
		params[i].sorter = this;
		params[i].stringVector = stringVectors->at(i);
		tasks[i].func = threadSortVector;
		tasks[i].param = &params[i];
		
		sorterMutex.lock();
		workThreadCount++;
		threadPool.enqueueTask(&tasks[i]);
		sorterMutex.unlock();
	}

	while (true)
	{
		sorterMutex.lock();
		if (!workThreadCount)
		{
			break;
		}
		sorterMutex.unlock();
		monitor.Wait();
	}

	threadPool.close();
	
	delete[] params;
	delete[] tasks;
	
	std::vector<std::string>* result = mergeSortVectors(stringVectors);
	delete stringVectors;
	
	writeStringsToFile(outputFile, result);
	delete result;
}

StringSorter::~StringSorter()
{
}

std::vector<std::vector<std::string>*>* StringSorter::receiveStringVectors(LPSTR inputFile)
{
	std::vector<std::string>* stringVector = receiveStringsFromFile(inputFile);
	std::vector<std::vector<std::string>*>* result = divideVector(stringVector, this->threadCount);
	delete stringVector;
	return result;
}
std::vector<std::string>* StringSorter::receiveStringsFromFile(LPSTR inputFile)
{
	std::vector<std::string>* vectorRes = new std::vector<std::string>();
	std::ifstream stream(inputFile);
	while (!stream.eof())
	{
		std::string str;
		getline(stream, str, DIVIDER);
		str += DIVIDER;
		vectorRes->push_back(str);
	}
	stream.close();
	return vectorRes;
}
std::vector<std::vector<std::string>*>* StringSorter::divideVector(std::vector<std::string>* stringVectorSource, int threadCount)
{
	std::vector<std::vector<std::string>*>* result = new std::vector<std::vector<std::string>*>();
	int j = 0;
	int stringInVectorCount;
	if (stringVectorSource->size() % threadCount == 0) 
	{
		stringInVectorCount = stringVectorSource->size() / threadCount;
	}
	else
	{
		stringInVectorCount = stringVectorSource->size() / threadCount + 1;
	}
	for (int i = 0; i < threadCount; i++)
	{
		std::vector<std::string>* stringVector = new std::vector<std::string>();
		for (int j = 0; j < stringInVectorCount; j++)
		{
			if (stringVectorSource->size() == 0) 
			{
				break;
			}
			std::string str = stringVectorSource->at(0);
			std::size_t index = str.find("\n");
			if (index != std::string::npos)
			{
				str = str.replace(index, 1, "");
			}
			stringVectorSource->erase(stringVectorSource->begin());
			stringVector->push_back(str);
		}
		result->push_back(stringVector);
	}
	return result;
}
bool StringSorter::writeStringsToFile(LPSTR outputFile, std::vector<std::string>* stringVector)
{
	std::ofstream stream(outputFile);
	for (std::string string : *stringVector)
	{
		stream.write(string.c_str(), strlen(string.c_str()));
	}
	stream.close();
	return true;
}

std::vector<std::string>* StringSorter::mergeSortVectors(std::vector<std::vector<std::string>*>* vectors)
{
	std::vector<std::string>* firstVector = vectors->at(0);
	if (vectors->size() != 1) 
	{
		for (int i = 1; i < vectors->size(); i++) 
		{
			std::vector<std::string>* tempVector = new std::vector<std::string>();
			std::vector<std::string>* workVector = vectors->at(i);
			while(workVector->size() != 0 && firstVector->size() != 0) 
			{
				std::string str1  = firstVector->at(0);
				std::string str2 = vectors->at(i)->at(0);
				if (str1 <= str2)
				{
					tempVector->push_back(str1);
					firstVector->erase(firstVector->begin());
				}
				else 
				{
					tempVector->push_back(str2);
					vectors->at(i)->erase(vectors->at(i)->begin());
				}
			}

			while(vectors->at(i)->size() != 0) 
			{
				tempVector->push_back(vectors->at(i)->at(0));
				firstVector->erase(vectors->at(i)->begin());
			}
		
			while (firstVector->size() != 0) 
			{
				tempVector->push_back(firstVector->at(0));
				firstVector->erase(firstVector->begin());
			}

			firstVector = tempVector;
		}
	}
	return firstVector;
}

DWORD _stdcall StringSorter::threadSortVector(LPVOID param)
{
	ThreadParamStruct* threadParamStruct = (ThreadParamStruct*)param;
	StringSorter* sorter = threadParamStruct->sorter;
	std::vector<std::string>* pVector = threadParamStruct->stringVector;
	sortStringVector(pVector);
	sorter->sorterMutex.lock();
	sorter->workThreadCount--;
	sorter->monitor.Pulse();
	sorter->sorterMutex.unlock();
	return 0;
}

void StringSorter::sortStringVector(std::vector<std::string>* stringVector)
{
	std::sort(stringVector->begin(), stringVector->end());
}


