#include "stdafx.h"
#include "Monitor.h"


Monitor::Monitor()
{
	InitializeCriticalSectionAndSpinCount(&monitorCritSection, 4000);
	InitializeConditionVariable(&monitorCondVar);
}

Monitor::~Monitor(){}

void Monitor::Wait()
{
	SleepConditionVariableCS(&monitorCondVar, &monitorCritSection, INFINITE);
}

void Monitor::Pulse()
{
	WakeConditionVariable(&monitorCondVar);
}

void Monitor::PulseAll()
{
	WakeAllConditionVariable(&monitorCondVar);
}

