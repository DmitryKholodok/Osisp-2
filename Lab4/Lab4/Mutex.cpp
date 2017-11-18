#include "stdafx.h"
#include "Mutex.h"

Mutex::Mutex() {
	InitializeCriticalSection(&critSection);
}

void Mutex::lock() {
	EnterCriticalSection(&critSection);
}

void Mutex::unlock() {
	LeaveCriticalSection(&critSection);
}

Mutex::~Mutex() {
	DeleteCriticalSection(&critSection);
}



