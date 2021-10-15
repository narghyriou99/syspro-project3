#ifndef MONITORSERVERHELP_H_
#define MONITORSERVERHELP_H_

#include <iostream>
#include <string>
#include <pthread.h>
#include "citizen.h"
#include "virus.h"
#include "common.h"


using namespace std;

extern pthread_mutex_t mtx;

extern pthread_cond_t cond_nonfull;
extern pthread_cond_t cond_nonempty;

extern pthread_mutex_t mtx_ready;
extern pthread_cond_t cond_ready;

extern pthread_mutex_t mtx_read;

/*------------------Main-List------------------*/
struct txtFile{
    string name;
    txtFile *next;
    int isRead; //0 No, 1 Yes
    int inBuffer;//0 No, 1 Yes
};

struct directoryNode{
    string dirName; //./test/Greece
    string country; //Greece
    int totalTxtFiles;
    txtFile *nextRecord;
    directoryNode* next;
};

class Directory{
    private:
    directoryNode *head, *tail;
    int totalNodes, txtFiles;

    public:
    Directory(){
        head = NULL;
        tail = NULL;
        totalNodes=0;
    }
    ~Directory(){
        directoryNode *temp = head;
        directoryNode *next = NULL;

        while (temp != NULL){
            next = temp->next;
            deleteRecord(temp);
            delete temp;
            temp = next;
        }
    }
    void insertDirectory(const string& path);
    txtFile* insertTxtFile(const string& path, const string& fileName);
    void deleteRecord(directoryNode *node);
    directoryNode* getHead();
    int checkIfTxtExist(const string& path, const string& fileName);
    void printList();
    int getTotalTxts();
    txtFile* checkIfTxtNotPlacedInBuf();
    txtFile* getTxt();  //Get a node who is not in buffer
    int checkIfAllReadInBuffer();
    txtFile *getNode();
};
/*---------------------------------------------*/

/*-----------------Cyclic Buffer ver2-----------------*/
class CB{
private:
    
public:
    txtFile **array;
    int start, end;
    int capacity;
    int count;

    CB(int size){
        capacity = size;
        count = 0;
        array = new txtFile*[capacity];
        for(int i=0;i<capacity;i++){
            array[i] = NULL;
        }
        // start = array[0];
        // end = array[capacity-1];
        start = 0;
        end = -1;
    }
    ~CB(){
        delete[] array;
    }

    void assignFilePaths(Directory *directoryList);
};





/*------------------Thread Args------------------*/
struct args{
    Directory *directoryList;
    CB *cyclicBuffer;
    List* virusList;
	CitizenList* citizenList;
	CountryList* countryList;
	VirusList* virusNameList;
	AgeList* ageList;
    int bloomSize;
};
/*------------------Thread Args------------------*/

void place(CB *cyclicBuffer, txtFile *data);
void insertTxtFiles(Directory *directoryList, const string &path, CB *cyclicBuffer);
void insertPaths(const string& paths, Directory *directoryList, CB *cyclicBuffer);
int openTxtFile(const string& txtName, int BloomSize, CitizenList *citList, CountryList *countryList, VirusList* virusNameList, AgeList *ageList, List  *virusList);
void sendBloomFilter(List *virusList, int fd, int bufferSize, int sizeOfBloom);

/*---------------Main----------------*/
void getCommand(List* virusList, CitizenList* citList, CountryList* countryList, VirusList *virusNameList, AgeList *ageList, int BloomSize);
int checkForErrors(int mode, CountryList* countryList, List* virusList, CitizenList* citList, int citizenId, const string& virusName, const string& country);//0 is ok, -1 tis poutanas
string todayDate();

/*-------------Functions-------------*/

void ResetData(int& citizenId, string& firstName, string& lastName, string& country, int& age, string& virusName, string& isVactinated, string& dateVaccinated);
void Entry(CitizenList *citList, CountryList *countryList, VirusList* virusNameList, AgeList *ageList, List  *virusList, int citizenId, const string& firstName, const string& lastName, const string& country, int age, const string& virusName, const string& isVactinated, const string& dateVaccinated, int bfsize);
int checkIfDateisBetween(const string& date1, const string& date2, const string& date3);

/*--------------Queries--------------*/
void vaccineStatusBloom(List* virusList, int citizenId, const string& virusName);	//1st
void vaccineStatus(List* virusList, CN* citizen, const string& virusName);	//2nd 
void vaccineStatusAll(List* virusList, CitizenList* citList, int citizenId); //3rd
void populationStatus(int mode, CountryList* countryList, CitizenList* citList, node* virus, const string& country, const string& date1, const string& date2); //4th
void popStatusByAge(int mode, CitizenList* citList, CountryList* countryList, node* virus, const string& country, const string& date1, const string& date2); //5th
void insertCitizen(CitizenList* citList, CountryList* countryList, VirusList* virusNameList, AgeList* ageList, List* virusList, int citizenId, const string& firstName, const string& lastName, const string& country, int age, const string& virusName, const string& isVactinated, const string& dateVaccinated, int bfsize); //6th
void listNonVaccinatedPersons(node* virus);	//8th
#endif