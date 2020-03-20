#				EE450 SOCKET PROGRAMMING PROJECT
#Full Name:Yelei Zhang
#USC    ID:1745298022
#Net    ID:yeleizha
#Section 3


*What I have done in the assignment.
I created 3 servers and a client via TCP and UDP. Stored a map.txt file at server A and constructed maps according to this file. Client side sent map id,file size and vertex to AWS, AWS then pass them to server A. Server A used Dijkstra algorithm to compute the shortest paths from all the other node to the vertex passed from AWS. Return the result to AWS, AWS forward the result and file size to server B. B then finished calculation of transmission delay,propagation delay and endtoend delay according to the file size, propagation speed and transmission speed , sent the result to AWS. AWS forward the final result to client. Each host works well with corresponding on screen messages showed of each steps.


*What your code files are and what each one of them does.
1)client.cpp---Create a TCP socket. Read map ID, vertex , and file size in which stored the map infomation from user input (in the format:./client mapid vertex filesize). Store these three parameter in a string and convert the string to char[], then send to AWS. Client wait for aws's response, after all the other 3 servers finish their functions, receive the final result from AWS and print it on the screen(Delay unit: milliseconds).

2)aws.cpp------Create a TCP and 2 UDP sockets(for server A&B). Show a boot up message when running this file. Recieved 3 parameters from client via TCP, store them in buffer. Print the on screen message. Forward the message to server A directly. Receive the result(shortest path) from serverA via UDP. Use delimiter',' to seperate what aws has recieved. Display it. Send result of serverA together with filesize to Server B via UDP. 

3)serverA.cpp--Create a UDP sockets(for aws). Show a boot up message when running this file. Use adjacency list to construct Maps. Use adjcency list to construct maps. Each map stored in the graph array (g[52]) with index be the distance between Map Id and 'A' or 'a'(ASCII).Record the number of vertexes and edges in each map and print on the screen. Use dijkstra algorithm to calculate the shortest paths from all the other vertexes in that map to the target vertex. Store the spanning tree in a stack. Convert the stack infomation into a string--->char[], data are seperated by ','. Send the result(char[]) back to AWS. Keep the socket open. 

4)serverB.cpp--Create a UDP sockets(for aws). Show a boot up message when running this file. Receive required message from AWS.Display it. Calculate transmission delay, propagation delay and endtoend delay (Delay unit: milliseconds) according to the file size, propagation speed and transmission speed. Display the result and send it to AWS.



*The format of all the messsages exchanged
client------->AWS:char[]
		  displayed in string:"<mapid>,<vertex>,<filesize>"

AWS------>serverA:char[]
		  displayed in string:"<mapid>,<vertex>,<filesize>"

serverA------>AWS:char[]
		  displayed in string:"<propagation speed>,<transmission speed>,<destination vertex name>,<length>,<destination vertex name>,<length>...,<destination vertex name>,<length>"

AWS------>serverB:char[]
		  displayed in string:"<filesize>,<propagation speed>,<transmission speed>,<destination vertex name>,<length>,<destination vertex name>,<length>...,<destination vertex name>,<length>"

serverB------>AWS:char[]
		  displayed in string:"<filesize>,<propagation speed>,<transmission speed>,<destination vertex name>,<length>,<Transmission Delay>,<Propagation Delay>,<EndtoEnd Delay>...<destination vertex name>,<length>,<Transmission Delay>,<Propagation Delay>,<EndtoEnd Delay>"


AWS------->Client:char[]
		  displayed  in string:"<filesize>,<propagation speed>,<transmission speed>,<destination vertex name>,<length>,<Transmission Delay>,<Propagation Delay>,<EndtoEnd Delay>...<destination vertex name>,<length>,<Transmission Delay>,<Propagation Delay>,<EndtoEnd Delay>"





*Condition that project will fail
1)The number of edges will be incorrect(printed on server A when there is emptyline or a line with while spaces in map.txt


*Note!!
1)I set Server A's on screen message will display the root node(target vertex) along with other vertexed while other hosts will not display the target vertex.  
2)The delay unit are 'millisecond', not 'second'.
3)On screen messages about length are in decreasing order by distance.




*Referenced code
All project was wrote by my self but referenced those websites contents
TCP connection and socket functions //referenced www.cnblogs.com/zkfopen/p/9441264.html	
UDP connection and socket functions //referenced www.cnblogs.com/zkfopen/p9382705.html
Dijkstra in serverA //code is completely rewrote by my self, main idea is referenced www.cnblogs.com/dzkang2011/p/3828965.html	







