CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -g
LIBS =

TARGET = rrtar
SOURCES = main.o

all: $(TARGET)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

install: all
	cp -f $(TARGET) ${HOME}/bin/
	cp -f makerrtar ${HOME}/bin/

clean:
	rm -f $(TARGET) *.o
