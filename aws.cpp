#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stack>
#include <iomanip>

using namespace std;

#define AWS_TCP_PORT  24022
#define AWS_UDP_PORT  23022
#define SERVER_A_PORT 21022
#define SERVER_B_PORT 22022

struct node
{	
	int nodeName;			//node name
	int sum;			//shortest distance from current node to the root node
	//friend bool operator<(node a, node b){return a.sum > b.sum;}; //Override operator to achieve min pile in increasing order
};



int main()
{
//-------------------------------------------------------TCP--to--server--A----------------------------------------------------------------
	//TCP connection and socket functions are referenced www.cnblogs.com/zkfopen/p/9441264.html
	//1.Create a TCP socket on AWS server side
	int awsSockDesc = socket(AF_INET, SOCK_STREAM, 0);
	if (awsSockDesc == -1){
			perror("Failed to create the socket on the AWS side");//print the error reason to the console window
			return 1;						//end the program
	}
	
	//2.Set address of AWS TCP server
	struct sockaddr_in awsAdd;
	memset(&awsAdd, 0, sizeof(awsAdd));
	awsAdd.sin_family = AF_INET;
	awsAdd.sin_port = htons(AWS_TCP_PORT);
	awsAdd.sin_addr.s_addr = inet_addr("127.0.0.1");

	//3.bind() TCP
	int bindRes = bind(awsSockDesc, (struct sockaddr*)&awsAdd, sizeof(awsAdd));
	if (bindRes == -1){
		perror("Failed to bind on the AWS side");
		return 1;						
	}
	
	//UDP connection and socket functions are referenced www.cnblogs.com/zkfopen/p/9382705.html
	//4.Create a UDP socket 
	int awsUDPsd = socket(AF_INET, SOCK_DGRAM, 0);
	if (awsUDPsd == -1){
		perror("Failed to create the socket on the AWS side");
		return 1;						
	}
	
	//5.set UDP addresses  
	//AWS UDP strcut
	struct sockaddr_in awsUdpAdd;
	memset(&awsAdd, 0, sizeof(awsUdpAdd));
	awsUdpAdd.sin_family = AF_INET;
	awsUdpAdd.sin_port = htons(AWS_UDP_PORT);
	awsUdpAdd.sin_addr.s_addr = inet_addr("127.0.0.1");
	//Server A
	struct sockaddr_in serverA;
	memset(&serverA, 0, sizeof(serverA));
	serverA.sin_family = AF_INET;
	serverA.sin_port = htons(SERVER_A_PORT);
	serverA.sin_addr.s_addr = inet_addr("127.0.0.1");
	int lengthA = sizeof(serverA);
	//Server B
	struct sockaddr_in serverB;
	memset(&serverB, 0, sizeof(serverB));
	serverB.sin_family = AF_INET;
	serverB.sin_port = htons(SERVER_B_PORT);
	serverB.sin_addr.s_addr = inet_addr("127.0.0.1");
	int lengthB = sizeof(serverB);
	
	

	//6.Create another UDP socket for B
	int awsUDPsdB = socket(AF_INET, SOCK_DGRAM, 0);
	if (awsUDPsdB == -1){
			perror("Failed to create the socket on the AWS side");
			return 1;						
	}

	while(1){
		//7.TCP listen() from client
		int lisRes = listen(awsSockDesc, 15);
		if (lisRes == -1){
			perror("Failed to listen on the AWS side");
			return 1;						
		}
		cout << "The AWS is up and running."<<endl;


		//8.accept() waiting for client's connection and return a child socket descriptor
		struct sockaddr_in clientAdd;
		socklen_t len = sizeof(clientAdd);
		int childDesc = accept(awsSockDesc,(struct sockaddr*)&clientAdd, &len);
		if (childDesc == -1){
			perror("Failed to accept on the AWS side");
			return 1;						
		}


		//9.TCP receive from client	
		vector<string> output1;
		char buff1[4096] = {};
		char copybuff1[4096] = {};
		int size = recv(childDesc, buff1, sizeof (buff1), 0);
		if (size < 0){
			perror("Failed to receive on the AWS side");
			return 1;						
		}
		strcpy(copybuff1, buff1);
		char *token = strtok(copybuff1," ");
		while (token != NULL){
			output1.push_back(token);
			token = strtok(NULL , " ");
		}
		cout << "The AWS has revieved map ID <" << output1[0] <<">, start vertex <"<< output1[1] << "> and file size <" << output1[2] << "> from the client using TCP over port <" << AWS_UDP_PORT << ">"<< endl ;



		//10. UDP Send to server A
		int sendResA = sendto(awsUDPsd,buff1,sizeof(buff1) , 0, (struct sockaddr *)&serverA, lengthA);
		if (sendResA < 0){
			perror("Failed to send on the Server A side");
			return 1;						
		}
		cout << "The AWS has sent map ID and starting bertex to server A using UDP over port <" << AWS_UDP_PORT << ">" << endl;


		//11.UDP receive from server A
		char buffA[4096] = {};
		char copybuffA[4096] = {};
		vector<string> outputA;
		int recvResA = recvfrom(awsUDPsd, buffA, sizeof (buffA), 0, (struct sockaddr *)&serverA, (socklen_t *)&lengthA);
		strcpy(copybuffA,buffA);
		char *token2 = strtok(copybuffA,",");
		int cnt = 0;
		while (token2 != NULL){
			outputA.push_back(token2);
			cnt ++;
			token2 = strtok(NULL , ",");
		}
		

		//12.print the result of A
		cout << "The AWS has received shortest path from server A" << endl;
		cout << "-----------------------------------------------" << endl;
		cout << "Destination   Min Length " << endl;
		cout << "--------------------------" << endl;
		for (int i = 2;i < cnt-3; i=i+2){
			cout << setiosflags(ios::left) << setw(14) << outputA[i] << setiosflags(ios::left) << setw(13) << outputA[i+1] << endl;
		}
		cout << "--------------------------" << endl;
	
		//13.UDP Send the result and file size to B
		string toB = output1[2] ;
		for (int i = 0;i < cnt; i ++){
			toB += 	"," + outputA[i];
		}

		
		//14.UDP Send to server B
		int sendResB = sendto(awsUDPsdB,toB.c_str(),2048 , 0, (struct sockaddr *)&serverB, lengthB);
		if (sendResB < 0){
			perror("Failed to send on the Server B side");
			return 1;						
		}
		cout << "The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port <" << AWS_UDP_PORT << ">" << endl;		


		//15.UDP receive from server B 
		char buffB[4096] = {};
		char copybuffB[4096] = {};
		vector<double> outputB;
		int recvResB = recvfrom(awsUDPsdB, buffB, sizeof (buffB), 0, (struct sockaddr *)&serverB, (socklen_t *)&lengthB);
		if (recvResB < 0){
			perror("Failed to receive on the Server B side");
			return 1;						
		}
		


		//16.UDP print the result of B
		strcpy(copybuffB,buffB);
		char *token3 = strtok(copybuffB,",");
		while (token3 != NULL){
			string h =token3;
			istringstream iss2(h);
			double value;
			iss2 >> value;
			outputB.push_back(value);
			token3 = strtok(NULL , ",");
		}	

		cout << "The AWS has received delays from server B: " << endl;
		cout << "-------------------------------------------" << endl;
		cout << "Destination        Tt        Tp       Delay " << endl;
		cout << "-------------------------------------------" << endl;
		for (int i = 3;i < outputB.size()-8; i= i+5){
			cout <<  setiosflags(ios::left) << setw(19) << setiosflags(ios::fixed)<< setprecision(0) << outputB[i]<< setw(10) << setiosflags(ios::fixed) << setprecision(2) << outputB[i+2] << setw(9)  << setiosflags(ios::fixed) << setprecision(2) << outputB[i+3]<< setw(9) << setiosflags(ios::fixed) << setprecision(2) << outputB[i+4] << endl;      
		}
		cout << "-------------------------------------------" << endl;
	

		
		//17.TCP Send to client
		int sendResC = send(childDesc,buffB, 2048, 0);
		if (sendResC == -1){
			cout <<"Failed to send to the Client." <<endl;
			return 1;
		}
		cout << "The AWS has sent calculated delay to client using TCP over port <"<< AWS_UDP_PORT << ">." << endl;
	}
	//18.Do not close anu socket
	//close(awsUDPsdB);
	//close(awsUDPsd);
	//close(childDesc);
	//close(awsSockDesc);
	return 0;

}
