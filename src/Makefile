CXX = clang++

CXXFLAGS = -std=c++17

TARGET = tinyraycaster

SRCS = tinyraycaster.cpp framebuffer.cpp utils.cpp map.cpp textures.cpp

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)