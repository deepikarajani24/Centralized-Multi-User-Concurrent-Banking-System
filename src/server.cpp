#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<iostream>
#include <fstream>
#include <sstream>
#include <csignal>
#include <ctime>
#define MAXELEMENTSIZE 22000
using namespace std;

int accountNoArray[MAXELEMENTSIZE], noOfrecords=0;//accountNoArray contains all account numbers from server records file
float balance[MAXELEMENTSIZE];//balance array contains all the balances from server records file
string name[MAXELEMENTSIZE];// name array contains all the names from server records file
fstream file;
pthread_mutex_t mutex[MAXELEMENTSIZE];//array of mutex is created
int arrayofindex[MAXELEMENTSIZE];
int noOfConcurrentLocks=-1;

float time_arr[MAXELEMENTSIZE];
int no_of_clients=0;

string performtransaction(string instruction);//this function performs withdraw and deposit operation
void * readwrite(void *arg);//this is the thread function

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

string performtransaction(string instruction){
		
		int size;
		int i=0,fileindex=0;
    		char operation[2],tempchar[256],op;
		string response="";
		
		strcpy(tempchar, instruction.c_str());//tokenize instruction which is of format =(timestamp accountno operation amount)
		strtok (tempchar," ");
		int accno=atoi(strtok (NULL, " "));// accno has the accountno from client transaction
		strcpy(operation,strtok (NULL, " "));//operation has the operation from client transaction
		float amount=std::atof(strtok (NULL, " "));//amount has the amount from client transaction
		//cout << "save for "<<endl;
		for(int i=0;i<noOfrecords;i++){
			
			if(accountNoArray[i]==accno){

				break;
			}		
			fileindex++;//fileindex is the index in records.txt of the accountno sent by client
		}		
		if(fileindex == noOfrecords){
			response="Account number not found ";
			return response;
		}
		pthread_mutex_lock(&mutex[fileindex]);//lock acquired for an account accountNoArray[fileindex]
		
		if(!strcmp(operation, "d")){//deposit operation
						
			cout << "Account Number\t: " << accountNoArray[fileindex] << endl;
			cout << "Old Balance\t: " << balance[fileindex] << endl;
			cout << "Deposit\t\t: " << amount << endl;
			cout << "New Balance \t: " << balance[fileindex]+amount << endl;
			balance[fileindex]=balance[fileindex]+amount;								
			response="balance deposited";
		
		}else if(!strcmp(operation,"w")){//withdraw operation
			if(balance[fileindex] <= 0 || balance[fileindex]-amount <0){//check for insufficient balance
				cout << "Account Number\t: " << accountNoArray[fileindex] << endl;
				cout << "Old Balance\t: " << balance[fileindex] << endl;
				cout << "Withdraw\t: " << amount << endl;			
				pthread_mutex_unlock(&mutex[fileindex]);
				return "insufficient balance";
			}
			
			cout << "Account Number\t: " << accountNoArray[fileindex] << endl;
			cout << "Old Balance\t: " << balance[fileindex] << endl;
			cout << "Withdraw\t: " << amount << endl;
			cout << "New Balance \t: " << balance[fileindex]-amount << endl;	
			balance[fileindex]=balance[fileindex]-amount;		
			response="balance withdrawn";
		}
		else{
			response= "invalid operation";
		}	
				
		pthread_mutex_unlock(&mutex[fileindex]);// lock released for account accountNoArray[fileindex]
		
		return response;
		
}

void * addInterest(void *arg){
	
    while(1){
		sleep(600);
		for(int i=0;i<noOfrecords;i++){
		
			pthread_mutex_lock(&mutex[i]);//acquires the lock when updating each record
			cout << endl << "lock acquired by interest method" << endl;
			
			balance[i]=balance[i]*(1 + .0001);//every 10 min, interest is added at the rate of 0.0001%
			
			pthread_mutex_unlock(&mutex[i]);//releases lock when updated the record
			cout << "lock released by interest method " << endl <<endl;
		
		}
	}
}

/**
* readwrite thread method is called for each client
*/
void * readwrite(void *arg){

	int newsockfd=*((int *)arg);
	float avg=0;
	char buffer[256];
	int n=0;
    	bzero(buffer,256);
	int accountNo[MAXELEMENTSIZE];
    	char *name;
    	float balance[MAXELEMENTSIZE];
	string instruction[MAXELEMENTSIZE];
	string response;
	int no_of_client_instructions=0;
	
    	while(n = read(newsockfd,buffer,256)){//keeps reading data from client continuously
		if(n==0){
			cout << "cannot read from the client";
			exit(0);
		}
		string s;
		
     		if (n < 0){
			perror("ERROR reading from socket");
		}
		s.append(buffer, buffer+n);
     		cout << endl << "Server : data received from client => \t" <<s << endl;
		cout.flush();
		bzero(buffer,256);
		
		response=performtransaction(s);//transaction received from client is sent to performtransaction method
		cout << "Server : sent response to client" << endl;
		no_of_client_instructions++;
		strcpy(buffer,response.c_str());
     		n = write(newsockfd,buffer,strlen(buffer));//writes the response back to the client
		if (n < 0){ 
			perror("ERROR writing to client");
		}

		
	}
	
	
}


int main(int argc, char *argv[])
{
	
    	int sockfd, newsockfd, portno,n,i=0,no_of_thread=0;
    	socklen_t clilen;
    	char buffer[MAXELEMENTSIZE], *p,tempchar[256];			
	string line;
    	pthread_t t2[MAXELEMENTSIZE];
	
    	struct sockaddr_in serv_addr, cli_addr;
	for(int i=0;i<MAXELEMENTSIZE;i++){
		pthread_mutex_init(&mutex[i],0);//mutex array elements are initialized to 0
	}

	if (argc < 3) {
        	 fprintf(stderr,"ERROR, no port or file name provided\n");
        	 exit(1);
   	}

    	file.open(argv[2], ios::in);//file is opened in read mode
    	if(!file.is_open()){
		cout << "Server records file could not be found";
		return 0;
     	}     	

     	sockfd = socket(AF_INET, SOCK_STREAM, 0);
     	if (sockfd < 0) 
        	error("ERROR opening socket");

     	bzero((char *) &serv_addr, sizeof(serv_addr));
     	portno = atoi(argv[1]);
     	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = INADDR_ANY;
     	serv_addr.sin_port = htons(portno);
     	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //bind the socket
	        error("ERROR on binding");
     	listen(sockfd,5);
     	clilen = sizeof(cli_addr);
     
     	while (getline(file, line))//reads server file data into arrays
    	{	
		if(!line.empty()){
			//tokenizes information from the transaction
		    	strcpy(tempchar, line.c_str());
			accountNoArray[i]=atoi(strtok (tempchar," "));
			p=strtok (NULL, " ");			
			strcpy(tempchar,p);
			name[i]=tempchar;
			balance[i]=atoi(strtok (NULL, " "));
			i++;
			noOfrecords++;	
		}
		
    	}     
    	file.close();

	pthread_t interest_calculator_Thread;// thread for calculating interest
	pthread_create(&interest_calculator_Thread, NULL, &addInterest, &newsockfd);	

	while(newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)){
	        no_of_thread++;
	        pthread_create(&t2[no_of_thread], NULL, &readwrite, &newsockfd);//pthread created for each client that is connected
    	}     
	close(newsockfd);
	close(sockfd);
	return 0; 
}
