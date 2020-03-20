

.PHONY:serverB
serverB:serverB
	./serverB


.PHONY:serverA
serverA:serverA
	./serverA


.PHONY: aws
aws: aws
	./aws


all: aws.cpp serverA.cpp serverB.cpp client.cpp
	g++ -o aws aws.cpp 
	g++ -o serverA serverA.cpp 
	g++ -o serverB serverB.cpp 
	g++ -o client client.cpp 




