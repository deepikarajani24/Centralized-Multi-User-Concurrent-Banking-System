#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define MAXELEMENTSIZE 22000// defines maximum number of transactions supported by one client
using namespace std;

int main(int argc, char *argv[])
{
	int portno = atoi(argv[2]);//port number
	float timestep=atof(argv[3]);//timestep
	ifstream file(argv[4]);//Transactions.txt file path

	if (argc < 5) {//required 4 number of arguments
        	 fprintf(stderr,"ERROR, incorrect arguments provided\n");
        	 exit(1);
   	}
   	string str1; 
	char * pch;
	int i=0;
	string instruction[MAXELEMENTSIZE];
	int timestamp[MAXELEMENTSIZE];		
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	int noOfInstructions=0;
	int start_clock, stop_clock;
	float arr_for_time[MAXELEMENTSIZE];

	//each line is read from the file one by one
    	while (getline(file, str1))
    	{
		if(!str1.empty()){
			char str[MAXELEMENTSIZE];
			strcpy(str, str1.c_str());
			instruction[i]=str1;
			pch = strtok (str," ");
			timestamp[i]=atoi(pch);//line is tokenized to extract timestamp
			i++;
			noOfInstructions++;
		}
	}
	

	sockfd = socket(AF_INET, SOCK_STREAM, 0);//client socket is created
	if (sockfd < 0) 
		cout << "ERROR opening socket";
	server = gethostbyname(argv[1]);//server address
	if (server == NULL) {
	        perror("ERROR, no such host\n");
	        exit(0);
	}

	// Citiation: Socket communication code is created by refering to the code shared by Dr. Banerjee for single server and client communication
	memset((char *) &serv_addr,0, sizeof(&serv_addr));

	serv_addr.sin_family = AF_INET;

	bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

	serv_addr.sin_port = htons(portno);

	cout << "Connecting Socket "<< endl;

    	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //connect call
        	cout << "ERROR connecting";
		
	for(int i=0;i<noOfInstructions;i++){
		if(i==0){
			sleep(timestamp[i]*timestep);
		}else{
			sleep((timestamp[i] - timestamp[i-1])*timestep);//each transaction waits for the specified timestamp before it is written to the client
	
		}
		bzero(buffer,256);
		strcpy(buffer, instruction[i].c_str());//string is converted to char buffer
		start_clock=clock();//transaction execution started
		n = write(sockfd,buffer,strlen(buffer));//transaction is sent to the server
		if (n < 0) 
			cout << "Instruction cannot be written" << endl;
		bzero(buffer,256);
		n = read(sockfd,buffer,MAXELEMENTSIZE);//response is read from the server
		stop_clock=clock();//transaction execution stopped

		if (n < 0) 
			cout << "ERROR reading from socket" << endl;
		cout << "Client : response received from server :" << buffer << endl;
		arr_for_time[i]= ((stop_clock-start_clock)/double(CLOCKS_PER_SEC)*1000);//array stores the time taken by each transaction to execute
	}
	float timeperinstruction= 0;
	for(int i=0;i<noOfInstructions;i++){
		timeperinstruction+=arr_for_time[i];// used for calculating average time taken per instruction
	}
	
	file.close();
	//commented as the below code is used for testing scalability
	/*ofstream file1;
	file1.open("file.txt",ios::app);
	if(!file1.is_open()){
		cout << endl << "file could not be opened" <<endl;
		exit(0);
	}
	file1  << timeperinstruction << "\n";
	file1.close();*/
	close(sockfd);// closed the client socket
	return 0;
}

