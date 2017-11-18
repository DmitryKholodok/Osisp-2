#include "stdafx.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool(int threadCount)
{
	InitializeCriticalSectionAndSpinCount(&threadPoolCritSection, 4000);
	InitializeConditionVariable(&threadPoolCondVar);
	this->threadCount = threadCount;
	this->closed = false;
	threads = new HANDLE[threadCount];
	for (int i = 0; i < threadCount; i++)
	{
		threads[i] = CreateThread(NULL, 0, threadProcessing, this, 0, NULL);
	}
}

DWORD _stdcall ThreadPool::threadProcessing(LPVOID param)
{
	ThreadPool* threadPool = (ThreadPool*)param;
	while (true)
	{
		TaskStruct* taskStruct = threadPool->dequeueTask();
		if (!taskStruct)
		{
			break;
		}
		else
		{
			taskStruct->func(taskStruct->param);
		}
	}
	return 0;
}


void ThreadPool::enqueueTask(TaskStruct* taskStruct)
{
	EnterCriticalSection(&threadPoolCritSection);
	tasks.push(taskStruct);
	WakeConditionVariable(&threadPoolCondVar);
	LeaveCriticalSection(&threadPoolCritSection);
}

TaskStruct* ThreadPool::dequeueTask()
{
	EnterCriticalSection(&threadPoolCritSection);
	while (tasks.size() == 0 && !closed)
	{
		SleepConditionVariableCS(&threadPoolCondVar, &threadPoolCritSection, INFINITE);
	}
	TaskStruct* taskStruct = NULL;
	if (tasks.size() != 0)
	{
		taskStruct = tasks.front();
		tasks.pop();
	}
	LeaveCriticalSection(&threadPoolCritSection);
	return taskStruct;
}

void ThreadPool::close()
{
	if (!closed)
	{
		EnterCriticalSection(&threadPoolCritSection);
		closed = true;
		WakeAllConditionVariable(&threadPoolCondVar);
		LeaveCriticalSection(&threadPoolCritSection);
	}
	for (int i = 0; i < threadCount; i++)
	{
		WaitForSingleObject(threads[i], INFINITE);
		CloseHandle(threads[i]);
	}
	delete[] threads;
}

ThreadPool::~ThreadPool()
{
	if (!closed)
	{
		close();
	}
}


