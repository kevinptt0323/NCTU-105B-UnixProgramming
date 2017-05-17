CXX = g++
CXXFLAGS = -O2 -std=c++11 -Wall -Wextra
TARGET = hw3sh
OBJS = command.o builtin.o signal_handler.o jobs.o

all: $(TARGET)

$(TARGET): main.cpp $(OBJS)
	$(CXX) $^ $(CXXFLAGS) -o $@

%.o: %.cpp %.h
	$(CXX) $< $(CXXFLAGS) -o $@ -c

clean:
	rm $(TARGET) $(OBJS)
