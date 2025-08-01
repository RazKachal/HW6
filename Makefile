CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

SRCS = hw6-files/main.cpp hw6-files/NIC_sim.cpp hw6-files/L2.cpp hw6-files/L3.cpp hw6-files/L4.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET = nic_sim.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

hw6-files/%.o: hw6-files/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)