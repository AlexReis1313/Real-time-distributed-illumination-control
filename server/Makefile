CXX = g++ # Or your preferred compiler
CXXFLAGS =-Wall -Wextra -std=c++17 -I./boost_1_84_0
LDFLAGS = -L./boost_1_84_0/stage/lib -lboost_system -pthread

#g++ -std=c++11 -o server your_code.cpp -lboost_system -pthread
# Define the target binaries
TARGETS = server client
#
# Default target
all: $(TARGETS)

server: server.o serial_port.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

serial_port.o: serial_port.cpp serial_port.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
client: client.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to clean up generated files
clean:
	rm -f $(TARGETS) *.o
