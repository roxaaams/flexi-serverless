#!/bin/bash

export FLEXI_PROVIDER=AWS

g++ -DFLEXIUSE -c main.cpp -o main.o -std=c++2b -pthread

g++ main.o libmy_flexi.a -o main -std=c++2b
