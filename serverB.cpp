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

#define SERVER_B_PORT 22022
#define AWS_UDP_PORT 23022

//UDP connection and socket functions are referenced www.cnblogs.com/zkfopen/p/9382705.html  
int main(int argc, char *argv[])
{
	//1.Create a socket on Server B side
	int bSockDesc = socket(AF_INET, SOCK_DGRAM, 0);
	if (bSockDesc == -1){
			perror("Failed to create the socket on the Server B side!");
			return 1;						
	}


	//2.set address
	struct sockaddr_in serverB;
	memset(&serverB, 0, sizeof(serverB));
	serverB.sin_family = AF_INET;
	serverB.sin_port = htons(SERVER_B_PORT);
	serverB.sin_addr.s_addr = inet_addr("127.0.0.1");
	int length = sizeof(serverB);
	
	//3.bind
	int bindRes = bind(bSockDesc, (struct sockaddr*)&serverB, sizeof(serverB));
	if (bindRes == -1){
		perror("Failed to bind on the ServerB side!");
		return 1;						
	}
	cout << "The Server B is up and running using UDP on port <" << SERVER_B_PORT << ">." <<endl;


	while(1){
		//4.receive the map id and source node from AWS 
		char buff[2048] = {};
		char copybuff[2048] = {};
		vector<string> output;

		struct sockaddr_in awsAdd;
		int recvRes = recvfrom(bSockDesc, buff, 2048, 0, (struct sockaddr *)&awsAdd,(socklen_t *)&length);
		if (recvRes < 0){
			perror("Failed to receive on the Server B side");
			return 1;						
		}


		strcpy(copybuff,buff);
		char *token = strtok(copybuff,",");
		while (token != NULL){
			output.push_back(token);
			token = strtok(NULL , ",");
		}
	
		string fileSize1 = output[0];
		string propSpeed1 = output[1];
		string tranSpeed1 = output[2];

		double fileSize;
		stringstream(output[0]) >> fileSize;
		double propSpeed;
		stringstream(output[1]) >> propSpeed;
		double tranSpeed;
		stringstream(output[2]) >> tranSpeed;


		//5.Display the infomation that received from AWS
		cout << "The Server B has received data for calculation:" << endl;
		cout <<"* Propagation speed: <" << propSpeed << "> km/s;" << endl;
		cout <<"* Transmission speed: <" << tranSpeed << "> Bytes/s;" << endl;	

		vector<int> data;
		for(int j = 3 ;j < output.size(); j++){
			char *token2 = strtok(const_cast<char*>(output[j].c_str())," ");
			while (token2 != NULL){
				int a;
				istringstream(token2) >> a;
				data.push_back(a);
				token2 = strtok(NULL , " ");
			}
		}
		for(int j = 0 ;j < data.size()-1 ; j=j+2){
			cout <<"* Path length for destination <" << data[j] << ">: <" <<  data[j + 1] << ">;" <<endl;
		}

		//5.calculate delays (unit: second)
		vector<double> tp;
		vector<double> end2end;
		double tt = (fileSize *1000) / (tranSpeed * 8.0);

	
		//cout << "tt:" << setiosflags(ios::fixed) << setprecision(2) << tt << endl;

		for(int j = 1 ;j < data.size() ; j=j+2){
			double caltp = (data[j]*1000) / propSpeed;
			tp.push_back(caltp);
			tp.push_back(0);

			double calend = caltp + tt;
			end2end.push_back(calend);
			end2end.push_back(0);
		}

		cout << "The Server B has finished the calculation of the delays:" << endl;
		cout << "-------------------" << endl;	
		cout << "Destination   Delay " << endl;
		cout << "-------------------" << endl;
		for(int j = 0 ;j < data.size()-3 ; j=j+2){
			cout << setiosflags(ios::left) << setw(14) << data[j] << setiosflags(ios::left) << setw(10) << setiosflags(ios::fixed) << setprecision(2)<<end2end[j] << endl;
		}
		cout << "-------------------" << endl;


		//7.Send to AWS
		ostringstream ttTostring;
		ttTostring << tt;
		string trans = ttTostring.str();
		string toAWS = fileSize1 + "," + propSpeed1 + "," + tranSpeed1 + ",";
		for(int j = 0 ;j < data.size()-1 ; j= j+2){
			//node name
			ostringstream data1Tostring;
			data1Tostring << data[j];
			string des = data1Tostring.str();
		
			//distance
			ostringstream data2Tostring;
			data2Tostring << data[j+1];
			string d = data2Tostring.str();
		
			ostringstream tpTostring;
			tpTostring << tp[j];
			string propa = tpTostring.str();
	
			ostringstream endTostring;
			endTostring << end2end[j];
			string endend = endTostring.str();	
		
			toAWS = toAWS + des + "," + d + "," + trans +","+ propa + "," + endend + ",";
		}

		int sendRes = sendto(bSockDesc, toAWS.c_str(), 4096, 0, (struct sockaddr *)&awsAdd, length);
		if (sendRes < 0){
			perror("Failed to send on the Server B side");
			return 1;						
		}
		cout << "The Server B has finished sending the output to AWS" << endl;
	}
	//8.close the socket
	//close(bSockDesc);





}
