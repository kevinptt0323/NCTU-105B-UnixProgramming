CXX = g++
CXXFLAGS = -O2 -std=c++11 -Wall -Wextra
TARGET = hw3sh

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $< $(CXXFLAGS) -o $@

clean:
	rm $(TARGET)
