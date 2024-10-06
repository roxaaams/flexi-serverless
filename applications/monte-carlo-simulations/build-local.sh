#!/bin/bash

g++ -c main.cpp -o mainlocal.o -std=c++2b -pthread

g++ mainlocal.o libmy_flexi.a -o mainlocal -std=c++2b
