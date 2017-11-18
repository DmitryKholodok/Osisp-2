#pragma once

#include "windows.h"

class Mutex {

public: 
	Mutex();
	void lock();
	void unlock();
	~Mutex();

protected:
	CRITICAL_SECTION critSection;
};

