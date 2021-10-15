#ifndef HELP_H_
#define HELP_H_

#include <iostream>
#include <string>

using namespace std;

/*----------------Hash-----------------------*/
unsigned long Newdjb2(string& str);
unsigned long Newsdbm(string& str);
unsigned long Newhash_i(string& str, unsigned int i);
/*--------------------------------------------*/

/*---------------------VirusList---------------------*/
struct request{
    string date;
    string country;
    int status; //1=Accepted, -1=rejected
    request *next;
};

struct virus{
    string virusName;
    int *bloomFilter;
    int size;
    request *record;
    virus *next;
};

class ParentVirusList{
private:
    virus *head, *tail;
    int totalViruses;
    int bfSize;

public:
    ParentVirusList(int BloomSize){
        head=NULL;
        tail=NULL;
        totalViruses=0;
        bfSize = BloomSize;
    }

    ~ParentVirusList(){
        virus* temp = head;
        virus* next = NULL;

        while (temp != NULL){
            next = temp->next;
            if(temp->bloomFilter!=NULL)
                delete[] temp->bloomFilter;
            delete temp;
            temp = next;
        }
        head = NULL;
        tail = NULL;
    }
    void insertVirus(const string& virusName);
    int checkIfVirusExists(const string& virusName);
    void insertBloomFilter(int fd, int bufferSize, const string& virusName);
    virus* getHead();

    int getVaccStatus(int citizenId, const string& virusName);   //1st Query-Check parent's Bloom Filter 
    void insertRequestRecord(const string& virusName, const string& country, const string& date, int status);

};
/*-----------------------------------------------------*/

struct Path{
    string path;
    string country;
    Path *next;
};

class SubDirectoryList{
private:
    Path *head;
    string inputDir;
    int totalPaths;
public:
    SubDirectoryList(const string& input_dir){
        inputDir = input_dir;
        head=NULL;
        totalPaths=0;
    }

    ~SubDirectoryList(){
        Path* temp = head;
        Path* next = NULL;

        while (temp != NULL){
            next = temp->next;
            delete temp;
            temp = next;
        }
        head = NULL;
    }
    int addPaths(const string& inputDir);
    int insertCountryPath(const string& path);
    int checkIfCountryExists(const string& path, int mode);
    int getTotalSubDirectories();
    Path* getNode(int nodeNumber);
    void printList();

    /*------------------------2nd--------------------------------*/
    
    Path* getHead();

};


struct monitorNode{
    int pid;
    int id;
    int totalSubDirectoriesTohandle;
    Path **path;
    monitorNode *next;
};

class MonitorList{
private:
    monitorNode *head, *tail;
    int totalMonitors;
    int totalDirectories;
public:
    MonitorList(int totalDirs){
        head=NULL;
        tail=NULL;
        totalMonitors=0;
        totalDirectories = totalDirs;
    }
    ~MonitorList(){
        monitorNode* temp = head;
        monitorNode* next = NULL;

        while (temp != NULL){
            next = temp->next;
            delete[] temp->path;
            delete temp;
            temp = next;
        }
        head = NULL;
        tail = NULL;
    }

    void addMonitor(int id);
    monitorNode* gethead();
    void addPath(int monitor, Path *path);
    void print();
    void insertPid(int monitorNumber, pid_t pid);
    string getTotalPaths(int monitorId);
    int getMonitor(const string& countryName);  //Given country, it returns the monitor number
    /*------------------------------------------------------------------------*/
    
    int getTotalDirectoriesPerMonitor(int monitorNumber);
    string getSpecificDirectory(int monitorNumber, int directoryNumber);
    
};

int getRandomPort();
int fileDistribution(MonitorList *monitorList, SubDirectoryList *subDirectoryList, int monitors);
void showOptions();
void messageDecryption(string message);
int checkIfDateIsBetween(const string& date1, const string& date2, const string& date3);

/*-----------------------Queries-----------------------*/
void travelStats(ParentVirusList *virusList, const string& virusName, const string& date1, const string& date2, const string& country, int mode);

#endif