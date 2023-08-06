#!/bin/bash

g++ -c flexi.cpp -o flexi.o -std=c++11

ar cr libmy_flexi.a flexi.o

g++ -DFLEXIUSE -c base64.cpp  -o base64.o  -std=c++2b


g++ -DFLEXIUSE -c main.cpp  -o main.o  -std=c++2b -pthread

g++ main.o base64.o libmy_flexi.a -o main.out -std=c++2b


#g++ -std=c++11 -o main main.cpp base64.cpp -pthread
