#!/bin/sh

echo 'Debug Objects'
c++ -g -c toolkit.cpp -DDEBUG hash.cpp container.cpp filename.cpp printhardlinked.cpp write2disk.cpp stats.cpp storage.cpp filegroup.cpp matchmatrix.cpp sizegroup.cpp holder.cpp templates.cpp main.cpp

echo 'Debug Samefile'
c++ -g -o samefile -DDEBUG samefile.cpp toolkit.o hash.o container.o filename.o printhardlinked.o write2disk.o stats.o storage.o filegroup.o matchmatrix.o sizegroup.o holder.o templates.o main.o

echo 'Debug Samelink'
c++ -g -o samelink -DDEBUG samelink.cpp toolkit.o hash.o filename.o

echo 'Debug Samearchive'
c++ -g -o samearchive samearchive.cpp toolkit.o hash.o container.o filename.o printhardlinked.o write2disk.o stats.o storage.o filegroup.o matchmatrix.o sizegroup.o holder.o templates.o main.o

echo 'Debug Samearchive'
c++ -g -o samearchive-lite -DDEBUG samearchive-lite.cpp toolkit.o hash.o
