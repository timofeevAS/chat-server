# Compiler and flags
CXX = g++
CXXFLAGS = -fsanitize=address -g -Wall -Wextra -std=c++17

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Executable name
TARGET = $(BIN_DIR)/chat_server

# Source files
SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/server/server.cpp \
       $(SRC_DIR)/logger/logger.cpp \
       $(SRC_DIR)/daemon/daemon.cpp

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Include directories
INCLUDES = -Iinclude

# Build rules
all: $(TARGET)

# Link executable
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Run the server
run: $(TARGET)
	$(TARGET)

# Phony targets
.PHONY: all clean run
