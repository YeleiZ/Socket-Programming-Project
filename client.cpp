#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <iomanip>

using namespace std;

const int  AWS_PORT = 24022;		//AWS's port number with client
const string IP_ADDR  = "127.0.0.1";	//local host 's IP address

//TCP connection and socket functions are referenced www.cnblogs.com/zkfopen/p/9441264.html
int main(int argc, char *argv[])
{	
	//1.create a socket(NOS returns a non-negative integer called Socket Descriptor or -1 if errors)
	int sockDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (sockDescriptor == -1){
		perror("Failed to create the socket on the client side");//print the error reason to the console window
		return 1;						//end the program
	}
	

	//2.create a structure for the AWS server. Set address.
	struct sockaddr_in info;
	memset(&info, 0 , sizeof(info));
	info.sin_family = AF_INET;      				//IPv4
	info.sin_port = htons(AWS_PORT);				//convert the awsPort(host byte order) to Network Byte Order
	if (info.sin_port == -1){
		perror("Failed to input Server's port number");
		return 1;	
	}
	int addCheck = inet_pton(AF_INET, IP_ADDR.c_str(), &info.sin_addr); //converts an numbers-and-dots IP into a corresponding type for struct in_addr(binary)
	if (addCheck == -1){
		perror("Address is on error");
		return 1;
	}
	if (addCheck == 0){
		perror("Address is messed up");
		return 1;
	} 


	//3.Establish a connection to the server socket
	int conRes = connect(sockDescriptor,(struct sockaddr*)&info, sizeof(info));
	if (conRes == -1){
		perror("Client side connection failed.");
		return 1;
	}
	cout << "The client is up and running." << endl;


	//4.Read map name from command line  ./client <Map ID> <Source Index> <File Size>   
	if (argc != 4){
		perror ("Invalid input!");
		return 1;	
	}

	//5.Send:
	string sendtoaws = "";
	for(int i = 1; i < argc; i++){	
		sendtoaws = sendtoaws + " " + argv[i];
	}
	int sendRes1 = send(sockDescriptor, sendtoaws.c_str(), sendtoaws.size() + 1, 0);
	if (sendRes1 == -1){
		cout <<"Failed to send to AWS server." <<endl;
		return 1;
	}
		
	string mapId = argv[1];
	string fileSize = argv[2];
	string vertexIndex = argv[3];
	cout << "The client has sent query to AWS using TCP : start vertex <"<< vertexIndex << ">;" <<"map <" << mapId <<">; file size <" << fileSize << ">." << endl ;

	//6.Wait for response
	vector<double> recvData;	
	char buff[4096] = {};
	char copybuff[4096] = {};
	int response = recv(sockDescriptor, buff, sizeof(buff), 0);

	strcpy(copybuff,buff);
	char *token = strtok(copybuff,",");
	while (token != NULL){
		string h =token;
		istringstream iss(h);
		double value;
		iss >> value;
		recvData.push_back(value);
		token = strtok(NULL , ",");
	}	


	//7.Display response
	cout << "The client has received results from AWS" << endl;
	cout << "----------------------------------------------------" << endl;
	cout << "Destination    Min Length    Tt       Tp       Delay" << endl;
	cout << "----------------------------------------------------" << endl;	
	for (int i = 3;i < recvData.size()-8; i= i+5){
		int round = recvData[i+1];
		cout << setiosflags(ios::left) << setw(15) << setiosflags(ios::fixed) << setprecision(0) << recvData[i]  << setw(14)   <<  round << setw(9) <<  setiosflags(ios::fixed) << setprecision(2) << recvData[i+2] <<  setw(9)  << setiosflags(ios::fixed) << setprecision(2) << recvData[i+3] <<  setw(10) <<   setiosflags(ios::fixed) << setprecision(2) << recvData[i+4] << endl;   
	
	}
	cout << "-----------------------------------------------" << endl;
	
	
	//8.Close the socket
	close(sockDescriptor);

	return 0;


}
