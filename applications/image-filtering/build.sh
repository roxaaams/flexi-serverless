#!/bin/bash

g++ -DFLEXIUSE -c base64.cpp  -o base64.o  -std=c++2b


g++ -DFLEXIUSE -c main.cpp  -o main.o  -std=c++2b -pthread

g++ main.o base64.o libmy_flexi.a -o main -std=c++2b
