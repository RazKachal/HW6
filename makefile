CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

SRCS = main.cpp NIC_sim.cpp L2.cpp L3.cpp L4.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET = nic_sim.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)