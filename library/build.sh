#!/bin/bash

g++ -c flexi.cpp -o flexi.o -std=c++11

ar cr libmy_flexi.a flexi.o
