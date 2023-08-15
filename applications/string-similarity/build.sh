#!/bin/bash

export FLEXI_PROVIDER=AWS

g++ -c flexi.cpp -o flexi.o -std=c++11

ar cr libmy_flexi.a flexi.o


g++ -DFLEXIUSE -c main.cpp -o main.o -std=c++2b -pthread

g++ main.o libmy_flexi.a -o main.out -std=c++2b
