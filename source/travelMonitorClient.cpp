#include <iostream> 
#include <cstring>
#include <cstdlib>
#include <sys/types.h> /*fork()*/
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include "travelMonitorClientHelp.h"
#include "common.h"

using namespace std;

int main(int argc, char *argv[]){

    //Error checking
    if (argc < 11) {
		cout << "Bad option" << endl;
        cout << "Usage: ./travelMonitor –m <numMonitors> -b <socketBufferSize> -c <cyclicBufferSize> -s <sizeOfBloom> -i <input_dir> -t <numThreads>" << endl;
		return 0;
	}

    int status;
    //Fix arguments
    int index = 1, numMonitors, socketBufferSize, cyclicBufferSize, sizeOfBloom, numThreads;
    string input_dir;
    while (index < argc) {
		if (!strcmp(argv[index], "-m"))
			numMonitors = atoi(argv[index + 1]);
		else if (!strcmp(argv[index], "-b"))
			socketBufferSize = atoi(argv[index + 1]);
        else if (!strcmp(argv[index], "-c"))
			cyclicBufferSize = atoi(argv[index + 1]);
        else if (!strcmp(argv[index], "-s"))
			sizeOfBloom = atoi(argv[index + 1]);
        else if (!strcmp(argv[index], "-i"))
			input_dir = argv[index + 1];
        else if (!strcmp(argv[index], "-t"))
			numThreads = atoi(argv[index + 1]);
		index += 2;
	}

    //Check if input_dir exists
    struct stat buffer;
    if(stat(input_dir.c_str(), &buffer)){   
        perror(input_dir.c_str());
        exit(-1);
    }

    int newSock[numMonitors];   //Array of all fds
    pid_t pid[numMonitors];     //Array of pids

    SubDirectoryList *subDirectoryList = new SubDirectoryList(input_dir);    //List of subdirectories
    subDirectoryList->addPaths(input_dir);  //Insert alphabetically all the directories

    if(numMonitors > subDirectoryList->getTotalSubDirectories()){   //Check if monitors are more than directories
        numMonitors = subDirectoryList->getTotalSubDirectories();
        cout<<"Adjust monitors to "<<numMonitors<<endl;
    }

    int totalDirs = subDirectoryList->getTotalSubDirectories();
    MonitorList *monitorList = new MonitorList(totalDirs);   //List of processes
    for(int i=0;i<numMonitors;i++){
        monitorList->addMonitor(i); //Add new monitor node to list
    }

    fileDistribution(monitorList, subDirectoryList, numMonitors);

    subDirectoryList->printList();  //Debug
    monitorList->print();   //Debug

    //Create server
    int portAray[numMonitors];
    int port = getRandomPort();
    for(int i=0;i<numMonitors;i++){
        portAray[i] = port + i;
    }

    struct sockaddr_in server[numMonitors];
    struct sockaddr *serverptr[numMonitors];
    struct hostent *rem;
    rem = gethostbyname("localhost");

    for(int i=0;i<numMonitors;i++){
        serverptr[i] = (struct sockaddr *)&server[i];
    }

    for(int i=0;i<numMonitors;i++){
        //Create sockets
        if ((newSock[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("socket");
            exit(-1);
        }
    }

    for(int i=0;i<numMonitors;i++){
        server[i].sin_family = AF_INET;
        memcpy(&server[i].sin_addr, rem->h_addr, rem->h_length);
	    //server[i].sin_addr.s_addr = INADDR_ANY;
	    server[i].sin_port = htons(portAray[i]);
    }
	
    
    for(int i=0;i<numMonitors;i++){
        pid[i] = fork();
        monitorList ->insertPid(i, pid[i]);

        if(pid[i]==-1){     //Error
            perror("fork");
            exit(-1);
        }else if(pid[i]==0){    //Child
            char port[6], thread[5], sockBufSize[10], cyclicBufSize[10], bloomSize[10]; 
            snprintf(port, sizeof(port), "%d", portAray[i]);   //Port
            snprintf(thread, sizeof(thread), "%d", numThreads);  //Threads
            snprintf(sockBufSize, sizeof(sockBufSize), "%d", socketBufferSize);  //Socket Buffer Size
            snprintf(cyclicBufSize, sizeof(cyclicBufSize), "%d", cyclicBufferSize);    //Cyclic Buffer Size
            snprintf(bloomSize, sizeof(bloomSize), "%d", sizeOfBloom); //Bloom Filter Size

            string totalPaths = monitorList->getTotalPaths(i);  //Paths
            int len = totalPaths.length() + 1;
            char *path = new char[len];
            strcpy(path, totalPaths.c_str());
            cout<<path<<endl;

            char* args[]={(char*)"./monitorServer", (char*)"-p", port, (char*)"-t", thread, (char*)"-b", sockBufSize, (char*)"-c", cyclicBufSize, (char*)"-s", bloomSize, path, NULL};
            //cout<<args[0]<<args[1]<<args[2]<<args[3]<<args[4]<<args[5]<<args[6]<<args[7]<<args[8]<<args[9]<<args[10]<<args[11]<<endl;
            if(execv(args[0], args) == -1){
                perror("monitor");
                exit(-1);
            }
            delete[] path;
        }  
    }
    
    //Connect
    int res;
    for(int i=0;i<numMonitors;i++){
        do{
            res = connect(newSock[i] ,serverptr[i] ,sizeof(server[i]));
        }while(res<0);
       cout<<"Requested connection to server"<<endl;
    }
	

    ParentVirusList *virusList = new ParentVirusList(sizeOfBloom);

    int totalViruses, BFexists;
    string message;
    for(int i=0;i<numMonitors;i++){ //Receive Bloom Filters
        // Get total viruses per monitor
        read(newSock[i], &totalViruses, sizeof(int));
        cout<<totalViruses<<" viruses to read."<<endl;
         for(int j=0;j<totalViruses;j++){
            message = customRead(newSock[i], socketBufferSize);
            cout<<"Virus:"<<message<<". ";
            if(!virusList->checkIfVirusExists(message))
                virusList->insertVirus(message);
            read(newSock[i], &BFexists, sizeof(int));
            if(BFexists==1){    //Reading Bloom Filter
                cout<<"Bloom Filter Exists"<<endl;
                virusList->insertBloomFilter(newSock[i], socketBufferSize, message);
            }else{
                cout<<"Bloom Filter does not exist"<<endl;           //Go to next virus
            }
        }

        message = customRead(newSock[i], socketBufferSize);
        if(!message.compare("READY")){
            cout<<"Monitor Server "<<i<<" is ready!"<<endl;
        } 
    }

    /*------------------Debug------------------*/
    //Check the sum of each array cells
    virus *temp = virusList->getHead();
    int *arr;
    int sum=0;
    int j=0;
    while(temp!=NULL){
        cout<<temp->virusName;
        arr=temp->bloomFilter;
        if(arr!=NULL){ 
            for(j=0;j<sizeOfBloom/4;j++){
                sum+=arr[j];
            }
            cout<<" "<<sum;
        }else{
            cout<<" NULL";
        }
        cout<<endl;
        sum =0;
        temp = temp->next;
    }
    /*------------------Debug------------------*/

    string fullCommand, word;
    int accepted=0, rejected=0;
    while(1){
        showOptions();
        cin.clear();
        getline(cin, fullCommand);
		istringstream iss(fullCommand);

        if (fullCommand.substr(0, fullCommand.find(" ")) == "/exit") {
            break;
        }else if(fullCommand.substr(0, fullCommand.find(" ")) == "/travelRequest"){
            int citizenId, counter=0;
            string date, countryFrom, countryTo, virusName;
            while (iss >> word) {
					if (counter == 1)
						citizenId = stoi(word);
					if (counter == 2)
						date = word;
                    if (counter == 3)
						countryFrom = word;
                    if (counter == 4)
						countryTo = word;
                    if (counter == 5)
						virusName = word;
					counter++;
			}
            if(counter==6){ //Check if all the arguments have been given
                if(virusList->checkIfVirusExists(virusName)==-1 && !subDirectoryList->checkIfCountryExists(countryFrom, 1)){   //Check if the virus and the country exis
                    if(virusList->getVaccStatus(citizenId, virusName)==-1){ //Search Bloom Filter
                        cout<<"REQUEST REJECTED - YOU ARE NOT VACCINATED"<<endl;
                        rejected++;
                        virusList->insertRequestRecord(virusName, countryFrom, date, -1);
                    }else{
                        cout<<"I have to search the monitors"<<endl;
                        int monitor=monitorList->getMonitor(countryFrom);   //Get the right monitor
                        cout<<monitor<<endl;
                        customWrite(newSock[monitor], socketBufferSize, "Q1");
                        customWrite(newSock[monitor], socketBufferSize, to_string(citizenId));  //Send citizenId
                        customWrite(newSock[monitor], socketBufferSize, virusName); //Send virus Name
                        customWrite(newSock[monitor], socketBufferSize, date);  //Send Travel Date

                        //Response
                        message = customRead(newSock[monitor], socketBufferSize);
                        if(!message.compare("YES")){
                            string dateVaccinated = customRead(newSock[monitor], socketBufferSize);
                            int requestStatus;
                            if(checkIfDateIsMoreThan6Months(dateVaccinated, date)==1){  //Check the dates
                                cout<<"REQUEST ACCEPTED - HAPPY TRAVELS"<<endl;
                                accepted++;
                                requestStatus=1;
                                virusList->insertRequestRecord(virusName, countryFrom, date, 1);
                            }else{
                                cout<<"REQUEST REJECTED - YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE"<<endl;
                                rejected++;
                                requestStatus=-1;
                                virusList->insertRequestRecord(virusName, countryFrom, date, -1);
                            }
                            customWrite(newSock[monitor], socketBufferSize, to_string(requestStatus));
                        }else{
                            rejected++;
                        }
                    }
                }else{
                    cout<<"Virus or CountryTo do not exist!"<<endl;
                }
            }else{
                cout<<"Missing arguments!"<<endl;
            }
        }else if(fullCommand.substr(0, fullCommand.find(" ")) == "/travelStats"){
            int counter=0;
            string virusName, date1, date2, country;
            while (iss >> word) {
                if (counter == 1)
                    virusName = word;
                if (counter == 2)
                    date1 = word;
                if (counter == 3)
                    date2 = word;
                if (counter == 4)
                    country = word;
                counter++;
			}
            if(counter==5){ //For specific country
                travelStats(virusList, virusName, date1, date2, country, 0);
            }else if(counter==4){   //All countries
                travelStats(virusList, virusName, date1, date2, country, 1);
            }else{
                cout<<"Missing arguments!"<<endl;
            }
            usleep(800);
        }else if(fullCommand.substr(0, fullCommand.find(" ")) == "/addVaccinationRecords"){
            int counter=0;
            string word, country;
            while (iss >> word) {
                if (counter == 1)
					country = word;
				counter++;
            }

            if(!subDirectoryList->checkIfCountryExists(country, 1)){
                cout<<"Country exists!"<<endl;
                int monitor = monitorList->getMonitor(country);
                message = input_dir + "/" + country;
                customWrite(newSock[monitor], socketBufferSize, "Q3");
                customWrite(newSock[monitor], socketBufferSize, message); //Send input_dir/Country

               
                //Receive the new Bloom Filter
                read(newSock[monitor], &totalViruses, sizeof(int));
                cout<<totalViruses<<endl;
                for(int j=0;j<totalViruses;j++){
                    message = customRead(newSock[monitor], socketBufferSize);
                    cout<<"Message:"<<message<<endl;
                    if(!virusList->checkIfVirusExists(message))
                        virusList->insertVirus(message);
                    read(newSock[monitor], &BFexists, sizeof(int));
                    if(BFexists==1){    //Reading Bloom Filter
                        cout<<" Bloom Filter Exists"<<endl;
                        virusList->insertBloomFilter(newSock[monitor], socketBufferSize, message);
                    }else{
                        cout<<" Bloom Filter does not exist"<<endl;           //Go to next virus
                    }
                }
            //usleep(900);
            cout<<"Total Viruses "<<totalViruses<<endl;
            
            }else{
                cout<<"Country doesn't exist!"<<endl;
            }
        }else if(fullCommand.substr(0, fullCommand.find(" ")) == "/searchVaccinationStatus"){
            int counter=0;
            string citizenId;
            while (iss >> word) {
                if (counter == 1)
					citizenId = word;
				counter++;
            }

            int flag=0;
            for(int i=0;i<numMonitors;i++){ //Send to all monitors
                customWrite(newSock[i], socketBufferSize, "Q4");
                customWrite(newSock[i], socketBufferSize, citizenId);
                message = customRead(newSock[i], socketBufferSize);
                if(message.compare("-1")){
                    messageDecryption(message);
                    messageDecryption(customRead(newSock[i], socketBufferSize));
                    flag=1;
                }
            }

            if(!flag)
                cout<<"Sorry! I can not find the specific citizen in any monitor"<<endl;
            //usleep(800);
        }else{
            cout << "Wrong option. Please re-type your command." << endl;
        }

    }

    for(int i=0;i<numMonitors;i++){
        customWrite(newSock[i], socketBufferSize, "FIN");
    }

    //Create the log file
    string fileName = "./logs/log_file." + to_string(getpid());
    ofstream MyFile(fileName);
    Path *node = subDirectoryList->getHead();
    while(node!=NULL){
        MyFile<<node->country<<endl;   //Maybe i should convert them to countries
        node=node->next;
    }
    
    MyFile<<"TOTAL TRAVEL REQUESTS "<<accepted+rejected<<endl;;
    MyFile<<"ACCEPTED "<<accepted<<endl;
    MyFile<<"REJECTED "<<rejected<<endl;
    MyFile.close();

    
    //Wait children
    int returnedPid=0;
    for(int i=0;i<numMonitors;i++){
        returnedPid = wait(&status);
		if (returnedPid < 0){
			perror("Wait");
        }
    }

    //Close socket
    for(int i=0;i<numMonitors;i++){
        close(newSock[i]);
    }

    //Clear up
    delete virusList;
    delete monitorList;
    delete subDirectoryList;
    return 0;
}