#pragma once
#include "Windows.h"

class Monitor
{

public:

	Monitor();
	~Monitor();
	void Wait();
	void Pulse();
	void PulseAll();

private:

	CRITICAL_SECTION monitorCritSection;
	CONDITION_VARIABLE monitorCondVar;
};

