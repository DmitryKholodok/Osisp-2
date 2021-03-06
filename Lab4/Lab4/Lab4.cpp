// Lab4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "StringSorter.h"

#define INPUT_FILE "input.txt"
#define OUTPUT_FILE "output.txt"

int main()
{
	int threadCount;
	printf("Enter count of threads:\n");
	scanf("%d", &threadCount);
	StringSorter sorter(threadCount);
	sorter.sort(INPUT_FILE, OUTPUT_FILE);
	printf("File was succesfully sorted\n");
	getchar();
	getchar();
	return 0;
}

