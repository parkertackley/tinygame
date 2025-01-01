CXX = clang++

CXXFLAGS = -std=c++17

TARGET = tinyraycaster

SRCS = tinyraycaster.cpp framebuffer.cpp utils.cpp map.cpp textures.cpp sprite.cpp

.PHONY:

$(TARGET): $(OBJS)
	rm -f $(TARGET)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)