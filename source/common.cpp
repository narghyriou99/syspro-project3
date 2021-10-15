#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>

#include "common.h"

using namespace std;

int customWrite(int fd, int bufferSize, const string& message){
    int length = message.length() + 1;
    char *buffer = new char[length];
    memset(buffer, '\0', sizeof(char)*length);
    strcpy(buffer, message.c_str());

    write(fd, &length, sizeof(int));    //Send Message size

    int sent, n;
    for(sent = 0; sent<length; sent+=n){
        if((length-sent)>bufferSize){
            if((n = write(fd, buffer+sent, bufferSize))==-1){
                perror("write");
                exit(-1);
            }    
        }else{
            if((n = write(fd, buffer+sent, length-sent))==-1){
                perror("write");
                exit(-1);
            }
        }  
    }
    delete[] buffer;
    //cout<<"S:"<<sent<<endl;
    return sent;
}

string customRead(int fd, int bufferSize){
    int bytesToRead=0;
    
    read(fd, &bytesToRead, sizeof(int));
    //cout<<"size:"<<bytesToRead<<endl;

    int received, n;
    char buffer[bytesToRead+1];
    for(received = 0; received<bytesToRead; received+=n){
        if((bytesToRead-received) > bufferSize){
            if((n = read(fd, buffer+received, bufferSize))==-1){
                return NULL;
            }
        }else{
            if((n = read(fd, buffer+received, bytesToRead-received))==-1){
                return NULL;
            }
        }
        usleep(800);
    }
    //cout<<"R:"<<received<<endl;
    return string(buffer);
}

//Definitely right
int customWriteBF(int fd, int bufferSize, int bloomFilterSize, int *array){
    int sent=0;
    for(int i=0;i<(bloomFilterSize/4);i++){
        sent+=write(fd, &array[i], sizeof(int));
    }
    cout<<"Sent: "<<sent<<endl;   //Debug
    return sent;
}

int customReadBF(int fd, int bufferSize, int bloomFilterSize, int arr[]){
    int received=0;
    for(int i=0;i<(bloomFilterSize/4);i++){
        received+=recv(fd, &arr[i], sizeof(int), MSG_WAITALL);
        //received+=read(fd, &arr[i], sizeof(int));
    }
    cout<<"Received: "<<received<<endl; //Debug
    return received;
}

// //Proper way
// int customWriteBF(int fd, int bufferSize, int bloomFilterSize, int *array){

//     int arr[bloomFilterSize];
//     memset(arr, 0, bloomFilterSize);
//     memcpy(arr, array, bloomFilterSize);
//     int sent, n;
//     for(sent = 0; sent<bloomFilterSize; sent+=n){
//         if((bloomFilterSize-sent)>bufferSize){
//             if((n = write(fd, arr+sent, bufferSize))==-1){
//                 perror("write");
//                 exit(-1);
//             }    
//         }else{
//             if((n = write(fd, arr+sent, bloomFilterSize-sent))==-1){
//                 perror("write");
//                 exit(-1);
//             }
//         }  
//     }
//     cout<<"Sent "<<sent<<endl;
//     return sent;
// }

// int customReadBF(int fd, int bufferSize, int bloomFilterSize, int arr[]){

//     int array[bloomFilterSize];
//     memset(array, 0, bloomFilterSize);
//     int received, n;
//     for(received = 0; received<bloomFilterSize; received+=n){
//         if((bloomFilterSize-received) > bufferSize){
//             if((n = recv(fd, array+received, bufferSize, MSG_WAITALL))==-1){
//                 perror("recv");
//                 exit(-1);
//             }
//         }else{
//             if((n = recv(fd, array+received, bloomFilterSize-received, MSG_WAITALL))==-1){
//                 perror("recv");
//                 exit(-1);
//             }
//         }
//     }
//     memcpy(arr, array, bloomFilterSize);
//     cout<<"Received:"<<received<<endl;
//     return received;
// }

int checkIfDateIsMoreThan6Months(const string& dateVaccinated, const string& dateTravel) {
    //dateVaccinated
    int year1 = stoi(dateVaccinated.substr(dateVaccinated.find_last_of("-") + 1));
    int month1 = stoi(dateVaccinated.substr(dateVaccinated.find("-") + 1, 2));
    int day1 = stoi(dateVaccinated.substr(0, 2));

    //dateTravel
    int year2 = stoi(dateTravel.substr(dateTravel.find_last_of("-") + 1));
    int month2 = stoi(dateTravel.substr(dateTravel.find("-") + 1, 2));
    int day2 = stoi(dateTravel.substr(0, 2));

    //MinDate
    int year3;
    int month3;
    int day3=day2;
    if (month2 - 6 <= 0){
        int temp = month2 - 6;
        month3 = 12 + temp;
        year3 = year2 - 1;
    }else {
        month3 = month2 - 6;
        year3 = year2;
    }
     
    int toBeCompared = (year1 * 10000) + (month1 * 100) + day1;
    int startDate = (year3 * 10000) + (month3* 100) + day3;
    int endDate = (year2 * 10000) + (month2 * 100) + day2;

    if (toBeCompared >= startDate && toBeCompared <= endDate) {
        return 1;	
    }
    else {
        return -1;
    }
}

int checkBloomFilter(int *array, int size){
    int sum=0;
    for(int i=0;i<(size/4);i++){
        sum+=array[i];
    }
    return sum;
}