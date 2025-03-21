TARGET = bt_test_server
BUILD_DIR = build
$(CXX) ?= g++

# Log level for the server
# 0 - Disabled
# 1 - Error
# 2 - Info
# 3 - Debug
LOG_LEVEL ?= 3

INCLUDES = -I/usr/include/ -Isrc/ -Iexternal/

DEFINES = -D_LOG_LEVEL=$(LOG_LEVEL) -DLOG_COLOR

CXXFLAGS = -O3 $(DEFINES) $(INCLUDES) -Wall -std=c++20

LDFLAGS =

SOURCES = $(shell find src/ -iname "*.cpp")
OBJS = $(SOURCES:%.cpp=$(BUILD_DIR)/obj/%.o)

all: make_server

make_server: $(OBJS)
	$(CXX) -o $(BUILD_DIR)/$(TARGET) $(OBJS) $(LDFLAGS)
	chmod +x $(BUILD_DIR)/$(TARGET)

$(OBJS): $(BUILD_DIR)/obj/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)