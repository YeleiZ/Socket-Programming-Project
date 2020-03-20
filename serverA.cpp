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
#include <fstream>
#include <map>
#include <ctype.h>
#include <stack>
#include <cstdio>
#include <iomanip>


using namespace std;

#define SERVER_A_PORT 21022
#define AWS_UDP_PORT  23022
#define VERNUMBER     11 		//The maximun number of vertex in a map.
#define INF	      0xfffff		//present infinite distance in Dijkstra


int  findLoc(map<int,int> m, int key);	
int  isNewNode(map<int,int> m, int key);
void constructMap(string fileName);
void addAdj(int index, int target, int adj, int distance);
void printMap(int index);
void Dijkstra(int root,int index);	//find the shortest paths to target vertex


typedef struct Vertex			//target Vertex
{	
	int node;			//vertex name
	struct AdjVertex *firstEdge;	//point to the first vertex that adjcent to it
}AdjList[VERNUMBER];

struct AdjVertex			//adjcent vertexes
{	
	int nodeInd;			//node index in g[]
	int distance;			//the edge(distance) from this adj node to target vertex
	struct AdjVertex *next;		//point to the next vertex that adjcent to the target vertex 
};

struct Graph				//maps in the file
{	
	string tp;			//propagation speed of current map
	string ttran;			//transmission speed of current map
	string mapId;
	int vnum;
	int numEdges;
	Vertex v[VERNUMBER];		//the max number of vertexes in the current map
	map<int,int> loc;		//location of a vertex in v[]
};

struct node
{	
	int ind;			//location of the node in v[]
	int nodeName;			//node name
	int dis;
	int sum;			//shortest distance from current node to the root node
	friend bool operator<(node a, node b){return a.sum > b.sum;}; //Override operator to achieve min pile in increasing order
};

		
Graph g[52];				//26 graphs from A~Z
int previous[VERNUMBER];		//the previous parent node, used to recover SPT
bool visited[VERNUMBER];		//show the state that whether the node is in SPT
node d[VERNUMBER];			//distance from root to every other vertexes. Final result is the shortest one.
stack<node> spt;			//spanning tree  priority_queue
stack<node> path;			//the spanning tree sent to AWS
int numMaps = 0;

//UDP connection and socket functions are referenced www.cnblogs.com/zkfopen/p/9382705.html  
int main(int argc, char *argv[])
{
	//1.Create a socket on Server A side
	int aSockDesc = socket(AF_INET, SOCK_DGRAM, 0);
	if (aSockDesc == -1){
			perror("Failed to create the socket on the Server A side!");
			return 1;						
	}


	//2.set address
	struct sockaddr_in serverA;
	memset(&serverA, 0, sizeof(serverA));
	serverA.sin_family = AF_INET;
	serverA.sin_port = htons(SERVER_A_PORT);
	serverA.sin_addr.s_addr = inet_addr("127.0.0.1");

	
	//3.bind 
	int bindRes = bind(aSockDesc, (struct sockaddr*)&serverA, sizeof(serverA));
	if (bindRes == -1){
		perror("Failed to bind on the ServerA side!");
		return 1;						
	}
	cout << "The Server A is up and running using UDP on port <" << SERVER_A_PORT << ">." <<endl;


	//4.Map construction
	string fileName = "./map.txt";
	constructMap(fileName);
	cout << "The server A has constructed a list of <" << numMaps << "> maps:" << endl;
	cout << "-------------------------------------------------"<<endl;
	cout << "Map ID    Num Vertices    Num Edges"<<endl;
	cout << "-------------------------------------------------"<<endl;
	for(int i = 0; i < 52;i++){
		if (g[i].vnum > 0){
			cout << setiosflags(ios::left) << setw(10) << g[i].mapId << setiosflags(ios::left) << setw(16) << g[i].vnum << setiosflags(ios::left) << setw(10) << g[i].numEdges <<endl;
		}	
	}
	cout << "-------------------------------------------------"<<endl;

	while (1){
		//5.receive the map id and source node from AWS 
		struct sockaddr_in awsAdd;
		memset(&awsAdd, 0, sizeof(awsAdd));
		socklen_t lengthAWS = sizeof(awsAdd);
		char *token;
		char buff[4096] = {};
		vector<string> output;
		int recvRes = recvfrom(aSockDesc,buff, sizeof (buff), 0, (struct sockaddr *)&awsAdd,&lengthAWS);
		if (recvRes < 0){
			perror("Failed to receive on the Server A side");
			return 1;						
		}
		token = strtok(buff," ");
		while (token != NULL){
			output.push_back(token);
			token = strtok(NULL , " ");
		}	
		cout << "The server A has received input for finding shortest path: starting vertex <" << output[1] << "> of map <" << output[0] << ">." << endl;


		//6.find the shortest path using Dijkstra and print in a required format
		int index ;	
		if(output[0][0] >= 'A' && output[0][0] <= 'Z'){	index = output[0][0] - 'A';}
		if(output[0][0] >= 'a' && output[0][0] <= 'z'){	index = output[0][0] - 'a' + 26;}

		int root = findLoc(g[index].loc, atoi(output[1].c_str()));
		int sum = 0;
		string result,p,t;
		istringstream word1(g[index].tp);
		word1 >> p;
		istringstream word2(g[index].ttran);
		word2 >> t;
		result = p + "," + t;
	
		Dijkstra(root, index);

		cout << "The server A has identified the following shortest paths:" <<endl;
		cout << "-----------------------------"<<endl;
		cout << "Destination   Min Length"<<endl;
		cout << "-----------------------------"<<endl;
		while (!spt.empty()){
			int total = spt.top().sum;
			cout << setiosflags(ios::left) << setw(14)<< spt.top().nodeName << setiosflags(ios::left) << setw(10) << total << endl;

			ostringstream convert1,convert2;
			convert1 << spt.top().nodeName;
			string name = convert1.str(); 

			convert2 << spt.top().sum;
			string value = convert2.str();
		
			result += "," + name +  "," + value  ;	
			spt.pop();
		}
		cout << "-----------------------------"<<endl;
	
	

		//7.Send to AWS
		int sendVnum = sendto(aSockDesc, result.c_str(), 2048, 0, (struct sockaddr *)&awsAdd, sizeof(awsAdd));	
		if (sendVnum == -1 ){
			perror("Failed to send on the Server A side");
			return 1;						
		}
		cout << "The Server A has sent shortest paths to AWS."<< endl;

	}
	//8.not close the socket
	//close(aSockDesc);

}

//construct maps in the file
void constructMap(string fileName){
	string delay;
	for (int i = 0;i < 52;i++){
		g[i].vnum = 0;
		g[i].numEdges = 0;
		g[i].mapId = "no";
	}
	AdjVertex *p;
	fstream file;
	file.open(fileName.c_str(), ios::in);
	if(!file){
		cout << "Fail to open the file"<< endl;
	}
	string line;
	getline(file, line);
	while(!line.empty() && !file.eof() && isalpha(line[0]) != 0){ 
		int index;
		if(line[0] >= 'A' && line[0] <= 'Z'){   index = line[0] - 'A';}
		if(line[0] >= 'a' && line[0] <= 'z'){	index = line[0] - 'a' + 26;}
			
		g[index].vnum = 0;
 		g[index].mapId = line[0];
		numMaps ++;
		//Store propagation delay
		getline(file,line);
		g[index].tp = line;
		//Store transmission delay
		getline(file,line);
		g[index].ttran = line;
		
		while(!file.eof() && getline(file,line) && !line.empty() && !isalpha(line[0]) ){	
			istringstream iss(line);
			int target;
			int adj;
			int distance;
			//create 2 new vertexed from the first 2 integer in the line
			iss >> target;
			if(isNewNode(g[index].loc, target) < 0){
				g[index].v[g[index].vnum].node = target;
				g[index].v[g[index].vnum].firstEdge = NULL;
				g[index].loc.insert(pair<int,int>(target,g[index].vnum));
				g[index].vnum = g[index].vnum + 1;
			}
			iss >> adj;
			if(isNewNode(g[index].loc,adj) < 0){
				g[index].v[g[index].vnum].node = adj;
				g[index].v[g[index].vnum].firstEdge = NULL;
				g[index].loc.insert(pair<int,int>(adj,g[index].vnum));
				g[index].vnum = g[index].vnum + 1;
			}
			iss >> distance;
		
			//add an adjcent node of the Vertex
			addAdj(index,target,adj,distance);
			addAdj(index,adj,target,distance);
			g[index].numEdges ++;
		}			
	}
	
	file.close();
}

//To judge whether it is a new node.If yes, return it index in g[], otherwise return -1.
int isNewNode(map<int,int> m, int key){
	if(m.count(key) > 0){return m[key];}	
	return -1;	
}

int findLoc(map<int,int> m, int key){
	map<int,int>::iterator iter = m.find(key);
	if(iter != m.end()){
		return iter->second;
	}
	return -1;
}

//For testing. Print a map's info.
void printMap(int index){
	AdjVertex *p;
	cout <<"Vertex   distance "<< g[index].vnum<< endl;
	
	for(int i = 0; i < g[index].vnum; i++){	
		p = g[index].v[i].firstEdge;
		while(p != NULL){
			cout << g[index].v[i].node <<"->"<< p->nodeInd << "   "<< p->distance <<endl;	
			p = p->next;
		}
	}

}

//Add an adjcent vertex of target vertex into adjcency list 
void addAdj(int index, int target, int adj, int distance){
	AdjVertex *p;
	p = new AdjVertex;
			
	int value1 = findLoc(g[index].loc,target);
	int value2 = findLoc(g[index].loc,adj);
	if(value1 < 0 || value2 < 0){cout<<"Not found !! "<< target << endl;}
	else{
		p->distance = distance;
		p->nodeInd = value2;
		p->next = g[index].v[value1].firstEdge;
		g[index].v[value1].firstEdge = p;
	}
}


//Excute dijkstra algorithm to calculate the shortest paths.
//Partially referenced www.cnblogs.com/dzkang2011/p/3828965.html
void Dijkstra(int root, int index){  				//root: the location of root in v[]
	for(int i = 0; i < g[index].vnum; i++){			//Initialization
		d[i].ind = i;		
		d[i].nodeName = g[index].v[i].node;		//i is the location in the Vertex[]
		d[i].sum = INF; 
		d[i].dis = INF;   				//All the node to root is infinite				
		visited[i] = false;				//has not found the shortest route	
	}
	d[root].dis = 0;
	d[root].sum = 0;					//the distance from distance to distance is 0			
	
	spt.push(d[root]);					//store in the SPT
	visited[root] = true;
	
	  					
	int n = -1;						//corresponding node index of the minimum distance
	do{	
		int min = INF;					//the minimum distance to root among all the current distances 
		int now = spt.top().ind;
		int total = spt.top().sum;		

		for(AdjVertex *a = g[index].v[now].firstEdge; a!=NULL ;a = a->next){
			if( (visited[a->nodeInd] == false )&& ((d[spt.top().ind].sum + a->distance) < d[a->nodeInd].sum)){
				d[a->nodeInd].dis = a->distance;
				d[a->nodeInd].sum = d[spt.top().ind].sum + a->distance;	
			}

		}
		for (int i = 0; i < g[index].vnum ; i++){     //find the smallest one and add the node to spanning tree
			if((visited[i] == false) && (d[i].sum < min)){
				min = d[i].sum;
				n = i;
			}	
		}
		spt.push(d[n]);
		visited[n] = true;

	}while(spt.size() != g[index].vnum);

}
	






