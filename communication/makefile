CXX = g++
CXXFLAGS = `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4`
TARGET = server
OBJECTS = server.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

server.o: server.cpp
	$(CXX) $(CXXFLAGS) -c server.cpp -o server.o

clean:
	rm -f $(OBJECTS) $(TARGET)
