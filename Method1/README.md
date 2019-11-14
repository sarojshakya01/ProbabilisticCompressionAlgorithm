compile client.cpp using
g++ -pthread -o client client.cpp

compile server.cpp using
g++ server.cpp -o server

run server
./server 8080

run client
./client 127.0.0.1 8080 < /..path../input1.txt