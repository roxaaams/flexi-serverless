#!/bin/bash

g++ -c base64.cpp  -o base64.o  -std=c++2b


g++ -c main.cpp  -o main2.o  -std=c++2b -pthread

g++ main2.o base64.o libmy_flexi.a -o mainlocal -std=c++2b
