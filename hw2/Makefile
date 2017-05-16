CC = gcc
TARGET = hw2.so

all: hw2.so

hw2.so: hw2.c
	$(CC) -fPIC -shared -o $@ $< -ldl

clean:
	rm $(TARGET)
