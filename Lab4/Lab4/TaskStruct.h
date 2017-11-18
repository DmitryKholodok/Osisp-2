#pragma once
#include "windows.h"

struct TaskStruct
{
	PTHREAD_START_ROUTINE func;
	LPVOID param;
};