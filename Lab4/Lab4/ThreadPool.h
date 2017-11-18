#pragma once
#include <queue>
#include "TaskStruct.h"
#include "Monitor.h"

class ThreadPool
{

public:

	ThreadPool(int threadCount);
	void enqueueTask(TaskStruct* taskStruct);
	void close();
	~ThreadPool();

private:

	bool closed;
	int threadCount;
	std::queue<TaskStruct*> tasks;
	HANDLE* threads;
	CRITICAL_SECTION threadPoolCritSection;
	CONDITION_VARIABLE threadPoolCondVar;

	static DWORD _stdcall threadProcessing(LPVOID param);
	TaskStruct* dequeueTask();
};

