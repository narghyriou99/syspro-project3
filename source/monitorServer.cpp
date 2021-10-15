#include <iostream> 
#include <cstring>
#include <cstdlib>
#include <sys/types.h> /*fork()*/
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include "monitorServerHelp.h"

using namespace std;

pthread_mutex_t mtx;

pthread_cond_t cond_nonfull;
pthread_cond_t cond_nonempty;

pthread_mutex_t mtx_ready;
pthread_cond_t cond_ready;

pthread_mutex_t mtx_read;

txtFile* obtain(CB *cyclicBuffer){
	pthread_mutex_lock(&mtx);
	while (cyclicBuffer->count <= 0) {
		cout<<"Buffer Empty"<<endl;
		pthread_cond_wait(&cond_nonempty, &mtx);
	}
	txtFile *temp = cyclicBuffer->array[cyclicBuffer->start];
	temp->inBuffer = 0;
	cyclicBuffer->start = (cyclicBuffer->start + 1) % cyclicBuffer->capacity;
	cyclicBuffer->count--;
	pthread_mutex_unlock(&mtx);
	return temp;
}

void *consume(void *input){
	struct args *arguments = (struct args *)input;
	while(1){
		txtFile* filename = obtain(arguments->cyclicBuffer);
		openTxtFile(filename->name, arguments->bloomSize, arguments->citizenList, arguments->countryList, arguments->virusNameList, arguments->ageList, arguments->virusList);
		filename->isRead = 1;
		pthread_cond_signal(&cond_nonfull);

		pthread_mutex_lock(&mtx_ready);
		if(!arguments->directoryList->checkIfAllReadInBuffer()){	//Check if reading process is done
			pthread_cond_signal(&cond_ready);
		}
		pthread_mutex_unlock(&mtx_ready);
	}
	pthread_exit(NULL);
}


int main(int argc, char *argv[]){
    cout<<"Started monitor with PID "<<getpid()<<endl;
    
    int index = 1, port, numThreads, socketBufferSize, cyclicBufferSize, sizeOfBloom;
    string paths;
    while (index < argc) {
		if (!strcmp(argv[index], "-p"))
			port = atoi(argv[index + 1]);
		else if (!strcmp(argv[index], "-t"))
			numThreads = atoi(argv[index + 1]);
        else if (!strcmp(argv[index], "-b"))
			socketBufferSize = atoi(argv[index + 1]);
        else if (!strcmp(argv[index], "-c"))
			cyclicBufferSize = atoi(argv[index + 1]);
        else if (!strcmp(argv[index], "-s"))
			sizeOfBloom = atoi(argv[index + 1]);
		index += 2;
	}
	paths = argv[11];

	cout<<"Port: "<<port<<endl;
	cout<<"Threads: "<<numThreads<<endl;
	cout<<"SocketBufSize: "<<socketBufferSize<<endl;
	cout<<"cyclicBufferSize: "<<cyclicBufferSize<<endl;
	cout<<"sizeOfBloom: "<<sizeOfBloom<<endl;
	cout <<"Paths: "<<paths<<endl;

	int sock, newSock;
	struct sockaddr_in server, client;
	socklen_t clientLen;

	struct sockaddr *serverPtr=(struct sockaddr *)&server;
	struct sockaddr *clientPtr=(struct sockaddr *)&client;
	struct hostent *rem;

	rem = gethostbyname("localhost");

	//Create socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(-1);
	}

	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
	//server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Bind socket to address
	if (bind(sock, serverPtr, sizeof(server))<0){
		perror("bind");
		exit(-1);
	}

	//Listening
	if (listen(sock, 5) < 0){
		perror("listen");
		exit(-1);
	}
	cout<<"Listening for connections to port "<<port<<endl;

	//Accept
	do{
		newSock = accept(sock, clientPtr, &clientLen);
		// sleep(1);
	}while(newSock < 0);

	// if((newSock = accept(sock, clientPtr, &clientLen))<0){
	// 	perror("accept");
	// 	exit(-1);
	// }
	cout<<"Accepted connection"<<endl;


	/*-----------------Data Structures-----------------------*/
	Directory *directoryList = new Directory();
	CB *cb = new CB(cyclicBufferSize);

    List* virusList = new List();
	CitizenList* citizenList = new CitizenList();
	CountryList* countryList = new CountryList();
	VirusList* virusNameList = new VirusList();
	AgeList* ageList = new AgeList();
    /*-------------------------------------------------------*/

	//Init mtxs
	pthread_mutex_init(&mtx, NULL);
	pthread_mutex_init(&mtx_ready, NULL);
	pthread_mutex_init(&mtx_read, NULL);

	//Init cond vars
	pthread_cond_init(&cond_nonfull, NULL);
	pthread_cond_init(&cond_nonempty, NULL);
	pthread_cond_init(&cond_ready, NULL);


	//Threads
	pthread_t *tids;
	tids = new pthread_t[numThreads];

	//Fix arguments
	struct args *arguments = new args();
	arguments->ageList = ageList;
	arguments->citizenList = citizenList;
	arguments->countryList = countryList;
	arguments->cyclicBuffer = cb;
	arguments->directoryList = directoryList;
	arguments->virusList = virusList;
	arguments->virusNameList = virusNameList;
	arguments->bloomSize = sizeOfBloom;


	
	cout<<"Original thread: "<<pthread_self()<<endl;
	for(int i=0;i<numThreads;i++){
		pthread_create(&tids[i], NULL, consume,(void *)arguments);	//Create
		cout<<"New thread: "<<tids[i]<<endl;
	}

	insertPaths(paths, directoryList, cb);	//Insert directories and txts to directoryList
	
	directoryList->printList();	//Debug

	
	//Wait, until file-reading process is done
	pthread_mutex_lock(&mtx_ready);
	while(directoryList->checkIfAllReadInBuffer()!=0){
		pthread_cond_wait(&cond_ready, &mtx_ready);
	}	
	pthread_mutex_unlock(&mtx_ready);

	citizenList->printList();	//Debug	
	virusList->printList();	//Debug

	/*------------------Debug------------------*/
    //Check the sum of each array cells
    node *temp = virusList->getHead();
    int *arr;
    int sum=0;
    int j=0;
    while(temp!=NULL){
        cout<<temp->virusName->virus;
        if(temp->bloomFilter!=NULL){
            arr=temp->bloomFilter->getBloomFilter();
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
	sendBloomFilter(virusList, newSock, socketBufferSize, sizeOfBloom);	//Send all BloomFilters
	customWrite(newSock, socketBufferSize, "READY");	//Indicate that monitor is ready


	int accepted=0, rejected=0;
	string message;
	while(1){
		
		message = customRead(newSock, socketBufferSize);
		
		if(!message.compare("FIN")){
			break;
		}else if(!message.compare("Q1")){
			int citizenId;
			string virusName, travelDate;
			citizenId = stoi(customRead(newSock, socketBufferSize));
			virusName = customRead(newSock, socketBufferSize);
			travelDate = customRead(newSock, socketBufferSize);
			cout<<citizenId<<" "<<virusName<<" "<<travelDate<<endl;

			string status, vaccinatedDate;
			status = citizenList->getVaccinationStatus(citizenId, virusName);
			if(!status.compare("YES")){
				customWrite(newSock, socketBufferSize, "YES");
				vaccinatedDate = citizenList->getVactinationDate(citizenId, virusName);
				customWrite(newSock, socketBufferSize, vaccinatedDate);
				
				if(stoi(customRead(newSock, socketBufferSize))==1){
					accepted++;
				}else{
					rejected++;
				}
			}else{
				customWrite(newSock, socketBufferSize, "NO");
				rejected++;
			}


		}else if(!message.compare("Q3")){
			message = customRead(newSock, socketBufferSize);
			cout<<"Finding new files to: "<<message<<endl;
			insertTxtFiles(directoryList, message, cb);

			//Wait, until file-reading process is done
			pthread_mutex_lock(&mtx_ready);
			while(directoryList->checkIfAllReadInBuffer()!=0){
				pthread_cond_wait(&cond_ready, &mtx_ready);
			}	
			pthread_mutex_unlock(&mtx_ready);

			sendBloomFilter(virusList, newSock, socketBufferSize, sizeOfBloom);	//Send all BloomFilters
			// citizenList->printList();	//Debug	
			// virusList->printList();	//Debug


		}else if(!message.compare("Q4")){
			int citizenId = stoi(customRead(newSock, socketBufferSize));
			if(!citizenList->checkIfCitizenExists(citizenId)){
				CN *temp = citizenList->getCitizen(citizenId);
				message = to_string(temp->citizenId) + "*" + temp->firstName + "*" + temp->lastName + "*" + temp->country->country + "*" + "/" + "*" + "AGE"+ "*" + to_string(temp->age->age) + "*" + "/" + "*";
				customWrite(newSock, socketBufferSize, message);
				string record;
				CI *innerTemp = temp->citizenInfo;
				while(innerTemp!=NULL){
					if(!innerTemp->isVactinated.compare("YES")){
						record = record + innerTemp->virusName->virus + "*" + "VACCINATED ON" + "*" + innerTemp->dateVaccinated + "*" + "/" + "*";
					}else{
						record = record + innerTemp->virusName->virus + "*" + "NOT YET VACCINATED" + "*" + "/" + "*";
					}
					innerTemp=innerTemp->next;
				}
				customWrite(newSock, socketBufferSize, record);
			}else{
				customWrite(newSock, socketBufferSize, "-1");	//Indicate that monitor couldn't find the citizen
			}
		}
	}

	for(int i=0;i<numThreads;i++){		//Kill
		pthread_kill(tids[i], 0);
	}

	// for(int i=0;i<numThreads;i++){
	// 	if(pthread_join(tids[i], NULL)){	//Join
	// 		perror("pthread_join");
	// 	}		
	// }
	

	//Create the log file
	string fileName = "./logs/log_file." + to_string(getpid());
	ofstream MyFile(fileName);
	directoryNode *node = directoryList->getHead();
	while(node!=NULL){
		MyFile<<node->country<<endl;
		node=node->next;
	}

	MyFile<<"TOTAL TRAVEL REQUESTS "<<accepted+rejected<<endl;;
	MyFile<<"ACCEPTED "<<accepted<<endl;
	MyFile<<"REJECTED "<<rejected<<endl;
	MyFile.close();


	//Clear up
	
	//Destroy mtxs
	pthread_mutex_destroy(&mtx);
	pthread_mutex_destroy(&mtx_ready);
	pthread_mutex_destroy(&mtx_read);

	//Destroy cond vars
	// pthread_cond_destroy(&cond_nonfull);
	// pthread_cond_destroy(&cond_nonempty);
	// pthread_cond_destroy(&cond_ready);
	close(newSock);	//Close socket
	
	delete virusList;
	delete citizenList;
	delete countryList;
	delete virusNameList;
	delete ageList;

	delete arguments;
	delete[] tids;
	delete directoryList;
	delete cb;
	cout<<"All done"<<endl;
    return 0;
}