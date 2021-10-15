#ifndef COMMON_H_
#define COMMON_H_
#include <iostream>
#include <string>

using namespace std;

int customWrite(int fd, int bufferSize, const string& message);
string customRead(int fd, int bufferSize);

int customWriteBF(int fd, int bufferSize, int bloomFilterSize, int *array);
int customReadBF(int fd, int bufferSize, int bloomFilterSize, int arr[]);

int checkIfDateIsMoreThan6Months(const string& dateVaccinated, const string& dateTravel); //Calculate if date Of Vaccination is between [TravelDate - 6months, TravelDate]
int checkBloomFilter(int *array, int size);

#endif