#include <iostream>
#include "travelMonitorClientHelp.h"
#include "common.h"
#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <cstring>

using namespace std;

unsigned long Newdjb2(string& str) {
	unsigned long hash = 5381;
	int len = str.length();
	for (int i = 0; i < len; i++) {
		hash = ((hash << 5) + hash) + str[i]; /* hash * 33 + c */
	}
	return hash;
}

unsigned long Newsdbm(string& str) {
	unsigned long hash = 0;
	int len = str.length();

	for (int i = 0; i < len; i++) {
		hash = str[i] + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

unsigned long Newhash_i(string& str, unsigned int i) {
	return Newdjb2(str) + i * Newsdbm(str) + i * i;
}

void ParentVirusList::insertVirus(const string& virusName){
    virus *temp = new virus();
    temp->bloomFilter=NULL;
    temp->size=bfSize;
    temp->virusName = virusName;
    temp->record=NULL;
    temp->next=NULL;

    totalViruses++;
    if(head == NULL){
        head=temp;
        tail=temp;
        temp=NULL;
    }else{
        tail->next=temp;
        tail=temp;
    }
}

int ParentVirusList::checkIfVirusExists(const string& virusName){
    virus *temp = head;
    while(temp!=NULL){
        if(!temp->virusName.compare(virusName))
            return -1;  //Exists
        temp = temp->next;
    }
    return 0;
}

virus* ParentVirusList::getHead(){
    return this->head;
}

void ParentVirusList::insertBloomFilter(int fd, int bufferSize, const string& virusName){
    virus *temp = head;
    while(temp!=NULL){
        if(!temp->virusName.compare(virusName)){
            int arrayLength = bfSize/4;
            if(temp->bloomFilter==NULL){    //Add it for the first time
                temp->bloomFilter = new int[arrayLength];
                memset(temp->bloomFilter, 0, bfSize);
                customReadBF(fd, bufferSize, bfSize, temp->bloomFilter);
            }else{  //Final BF = (Old BF || Received BF)
                int *arr = new int[arrayLength];
                customReadBF(fd, bufferSize, bfSize, arr);
                for(int i=0;i<arrayLength;i++){
                    temp->bloomFilter[i] = temp->bloomFilter[i] | arr[i]; 
                }
                delete[] arr;
            }
            return;
        }
        temp = temp->next;
    }
}
int ParentVirusList::getVaccStatus(int citizenId, const string& virusName){
    virus *tempNode = head;
    while(tempNode!=NULL){
        if(!tempNode->virusName.compare(virusName)){
            int index = 0, innerIndex = 0;
            int value = 0;
            int temp = 0;
            string citId = to_string(citizenId);
            for (int i = 0; i < 16; i++) {
                index = Newhash_i(citId, i) % (bfSize/4);
                value = tempNode->bloomFilter[index];
                innerIndex = index % 32;	//Access bit array
                //temp = value << (32-innerIndex);
                //temp = temp >> 32;
                temp = (value >> innerIndex) & 1;
                if (temp == 0) {
                    return -1;	//TRUE NEGATIVE
                }
                temp = 0;
            }
            return 0;	//MAYBE
        }   
        tempNode = tempNode->next;
    }
    return -2;
}

void ParentVirusList::insertRequestRecord(const string& virusName, const string& country, const string& date, int status){
    request *rec = new request();
    rec->country = country;
    rec->date = date;
    rec->status = status;
    rec->next = NULL;

    virus *temp = head;
    while(temp!=NULL){
        if(!temp->virusName.compare(virusName)){    //Find the right node
            if(temp->record==NULL){ //Head
                temp->record = rec;
            }else{
                request *last = temp->record;   //Inner
                while (last->next != NULL)  
                    last = last->next;
                last->next = rec;
            }
            return;
        }
        temp = temp->next;
    }
}

int SubDirectoryList::addPaths(const string& inputDir){

    struct dirent *dir;
    DIR *dp = opendir(inputDir.c_str());
    if(dp){
        string unique_path;
        while((dir=readdir(dp)) != NULL){
            if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) //Reject . and ..
                continue;
            if(dir->d_type == DT_DIR){
                unique_path = inputDir + "/" + dir->d_name;
                insertCountryPath(unique_path);   //Add it alphabetically
            }
        }
        closedir(dp);
        return 0;
    }
    return -1;
}

int SubDirectoryList::checkIfCountryExists(const string& path, int mode){
    if(mode==1){//Country comparison
        Path* temp = head;
        while (temp != NULL) {
            if (!temp->country.compare(path))
                return 0;
            temp = temp->next;
        }
    return -1;
    }

    Path* temp = head;  //Path comparison
    while (temp != NULL) {
        if (!temp->path.compare(path))
            return 0;
        temp = temp->next;
    }
    return -1;
    
}

int SubDirectoryList::insertCountryPath(const string& path) {
    if (!checkIfCountryExists(path, 0)) {
        cout << "Path "<<path<<" already exists" << endl;
        return -1;
    }
    Path* newNode = new Path;
    newNode->path = path;
    newNode->country = path.substr(path.find("/") + 1);
    newNode->country = newNode->country.substr(newNode->country.find("/") + 1);
    newNode->next = NULL;
    totalPaths++;

    if (head == NULL || head->path >= newNode->path) {
        newNode->next = head;
        head = newNode;
    }
    else {
        Path* temp = head;
        while (temp->next!=NULL && temp->next->path < newNode->path) {
            temp = temp->next;
        }
        newNode->next = temp->next;
        temp->next = newNode;
    }
    return 0;
}

int SubDirectoryList::getTotalSubDirectories(){
    return this->totalPaths;
}

Path* SubDirectoryList::getNode(int nodeNumber){
    string empty;
    Path* temp=head;
    int counter=0;
    while(temp!=NULL){
        if(counter==nodeNumber)
            return temp;
        counter++;
        temp=temp->next;
    }
    return NULL;
}

Path* SubDirectoryList::getHead(){
    return this->head;
}

void SubDirectoryList::printList(){
    Path *temp = head;
    cout<<"Total paths:"<<totalPaths<<endl;
    while(temp!=NULL){
        cout<<temp->path<<endl;
        temp = temp->next;
    }
}

void MonitorList::addMonitor(int id){
    monitorNode *temp = new monitorNode;
    temp->id = id;
    temp->next = NULL;
    temp->totalSubDirectoriesTohandle=0;
    temp->path = new Path*[totalDirectories];
    for(int i=0;i<totalDirectories;i++){
        temp->path[i] = NULL;
    }
    totalMonitors++;

    if(head == NULL){
        head=temp;
        tail=temp;
        temp=NULL;
    }else{
        tail->next=temp;
        tail=temp;
    }
}

monitorNode* MonitorList::gethead(){
    return this->head;
}

void MonitorList::addPath(int monitor, Path *path){

    monitorNode *temp = head;
    int i=0;
    while(temp!=NULL){  //Go to spesific monitor
        if(i==monitor)
            break;
        i++;
        temp = temp->next;
    }

    for(int i=0;i<totalDirectories;i++){
        if(temp->path[i]==NULL){
            temp->path[i] = path;
            temp->totalSubDirectoriesTohandle++;
            return;
        }
    }
}

void MonitorList::print(){
    monitorNode *temp = head;
    while(temp!=NULL){
        cout<<"Monitor: "<<temp->id<<" has "<<temp->totalSubDirectoriesTohandle<<" subdirectories to handle: ";
        for(int i=0;i<totalDirectories;i++){
            if(temp->path[i]!=NULL){
                cout<<temp->path[i]->path<<" ";
            }
        }
        temp=temp->next;
        cout<<endl;
    }
}

void MonitorList::insertPid(int monitorNumber, pid_t pid){
    monitorNode *temp = head;
    while(temp!=NULL){
        if(temp->id == monitorNumber){
            temp->pid = pid;
        }
        temp=temp->next;
    }
}

string MonitorList::getTotalPaths(int monitorId){
    monitorNode *temp = head;
    string total;
    while(temp!=NULL){
        if(temp->id == monitorId){
            for(int i=0;i<totalDirectories;i++){
                if(temp->path[i]!=NULL){
                    total = total + temp->path[i]->path + "-";
                }
            }
            return total;
        }
        temp = temp->next;
    }
    return total;
}

int MonitorList::getMonitor(const string& countryName){
    monitorNode *temp = head;

    while(temp!=NULL){
        for(int i=0;i<totalDirectories;i++){
            if(temp->path[i]!=NULL && !temp->path[i]->country.compare(countryName)){
                return temp->id;    //Return monitor id
            }
        }
        temp = temp->next;
    }
    return -1;
}

int getRandomPort(){
    srand(time(0));
    return rand()%30000 + 50000;
}

int fileDistribution(MonitorList *monitorList, SubDirectoryList *subDirectoryList, int monitors){
    monitorNode *temp=monitorList->gethead();
    int i=0, j=0, totalSubDirectories=subDirectoryList->getTotalSubDirectories();
    while(totalSubDirectories){
        while(temp!=NULL){
            if(!totalSubDirectories)
                break;
            monitorList->addPath(i,subDirectoryList->getNode(j));
            totalSubDirectories--;
            i++;
            j++;
            temp=temp->next;
            
        }
        temp=monitorList->gethead();
        i=0;
        
    }
    return 0;
}

void showOptions(){

    cout << endl;
	cout << "Please select one of these options(1-5):" << endl;
	cout << "--------------------------------------------------------------------------------" << endl;
	cout << "/travelRequest <citizenID> <date> <countryFrom> <countryTo> <virusName>" << endl;
	cout << "/travelStats <virusName> <date1> <date2> <[country]>" << endl;
	cout << "/addVaccinationRecords <country>" << endl;
	cout << "/searchVaccinationStatus <citizenID>" << endl;
	cout << "/exit" << endl;
	cout << "--------------------------------------------------------------------------------" << endl;
	cout << endl;
}

void messageDecryption(string message) {
    string deliminter = "*";

    int pos = 0;
    string token;

    while ((pos = message.find(deliminter)) != (int)message.npos) {
        token = message.substr(0, pos);
        if (!token.compare("/")) {
            cout << endl;
        }
        else {
            cout <<token<<" ";
        }
        
        message.erase(0, pos + deliminter.length());
    }

}

int checkIfDateIsBetween(const string& date1, const string& date2, const string& date3){
	//date1
	int year1 = stoi(date1.substr(date1.find_last_of("-") + 1 ));
	int month1 = stoi(date1.substr(date1.find("-") + 1, 2));
	int day1 = stoi(date1.substr(0, 2));

	//date2
	int year2 = stoi(date2.substr(date2.find_last_of("-") + 1));
	int month2 = stoi(date2.substr(date2.find("-") + 1, 2));
	int day2 = stoi(date2.substr(0, 2));
	//date3
	int year3 = stoi(date3.substr(date3.find_last_of("-") + 1));
	int month3 = stoi(date3.substr(date3.find("-") + 1, 2));
	int day3 = stoi(date3.substr(0, 2));

	int toBeCompared = (year2 * 10000) + (month2 * 100) + day2;
	int startDate = (year1 * 10000) + (month1 * 100) + day1;
	int endDate = (year3 * 10000) + (month3 * 100) + day3;

	if (toBeCompared >= startDate && toBeCompared <= endDate) {
		return 1;
	}
	else {
		return -1;
	}
}

void travelStats(ParentVirusList *virusList, const string& virusName, const string& date1, const string& date2, const string& country, int mode){
    virus *temp = virusList->getHead();
    int accepted=0, rejected=0;
    while(temp!=NULL){
        if(!temp->virusName.compare(virusName)){
            request *record = temp->record;
            if(record!=NULL){
                while(record!=NULL){
                    if(checkIfDateIsBetween(date1, record->date, date2)==1){
                        if(!mode){  //Country given
                            if(!record->country.compare(country)){
                                 if(record->status==1){
                                    accepted++;
                                }else{
                                    rejected++;
                                }
                            }
                        }else{  //All countries
                            if(record->status==1){
                                accepted++;
                            }else{
                                rejected++;
                            }
                        }
                    } 
                    record = record->next;
                }
                cout<<"TOTAL REQUESTS "<<accepted+rejected<<endl;
                cout<<"ACCEPTED "<<accepted<<endl;
                cout<<"REJECTED "<<rejected<<endl;
            }else{
                cout<<"Nothing found!"<<endl;
            }
            return;
        }
        temp = temp->next;
    }
}