#!/bin/bash

g++ -c flexi.cpp -o flexi.o -std=c++2b

ar cr libmy_flexi.a flexi.o
