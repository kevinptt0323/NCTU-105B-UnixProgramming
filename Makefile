CXX = /usr/bin/g++
CXXFLAGS = -std=c++11 -O2 -lm -Wall -Wshadow
TARGET = hw1
all: main.cpp
	$(CXX) $< $(CXXFLAGS) -o $(TARGET)
