#!/bin/bash


g++ -c main.cpp -o main2.o -std=c++2b -pthread

g++ main2.o libmy_flexi.a -o mainlocal -std=c++2b
