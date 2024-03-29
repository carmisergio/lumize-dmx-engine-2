SHELL := /bin/bash

# Compiler and linker
CC = g++ 

# compiler flags:
#  -g    adds debugging information to the executable file
#  -c compiles to object files
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -c -Wall -std=c++11 -pthread
LFLAGS  = -g -Wall -std=c++11 -pthread

# linking information:
#  --libs libftdi
PKG_CONFIG = `pkg-config --cflags --libs libftdi1`

# source and build folders
SRC = src
BUILD = build

# name of the program
EXECUTABLE = lumizedmxengine2

# install stuff
INSTALL_SCRIPT = installer/install.sh
UNINSTALL_SCRIPT = installer/uninstall.sh


# Main executable target
$(EXECUTABLE): $(BUILD)/main.o $(BUILD)/dmxsender.o $(BUILD)/tcpserver.o $(BUILD)/lightrenderer.o $(BUILD)/logger.o $(BUILD)/configreader.o $(BUILD)/persistency.o
	@ echo "Linking main executable..."
	@ mkdir -p $(BUILD)
	@ $(CC) $(LFLAGS) -o $(EXECUTABLE) $(BUILD)/main.o $(BUILD)/dmxsender.o $(BUILD)/tcpserver.o $(BUILD)/lightrenderer.o $(BUILD)/logger.o $(BUILD)/configreader.o $(BUILD)/persistency.o $(PKG_CONFIG)
	@ echo "Build complete!"

$(BUILD)/main.o: $(SRC)/main.cpp
	@ echo "Compiling main.cpp..."
	@ mkdir -p $(BUILD)
	@ $(CC) $(CFLAGS) -o $(BUILD)/main.o $(SRC)/main.cpp
	@ echo "Finished compilation for main.cpp"

$(BUILD)/dmxsender.o: $(SRC)/dmxsender.cpp $(SRC)/dmxsender.h
	@ echo "Compiling dmxsender.cpp..."
	@ mkdir -p $(BUILD)
	@ $(CC) $(CFLAGS) -o $(BUILD)/dmxsender.o $(SRC)/dmxsender.cpp
	@ echo "Finished compilation for dmxsender.cpp"

$(BUILD)/tcpserver.o: $(SRC)/tcpserver.cpp $(SRC)/tcpserver.h
	@ echo "Compiling tcpserver.cpp..."
	@ mkdir -p $(BUILD)
	@ $(CC) $(CFLAGS) -o $(BUILD)/tcpserver.o $(SRC)/tcpserver.cpp
	@ echo "Finished compilation for tcpserver.cpp"

$(BUILD)/lightrenderer.o: $(SRC)/lightrenderer.cpp $(SRC)/lightrenderer.h
	@ echo "Compiling lightrenderer.cpp..."
	@ mkdir -p $(BUILD)
	@ $(CC) $(CFLAGS) -o $(BUILD)/lightrenderer.o $(SRC)/lightrenderer.cpp
	@ echo "Finished compilation for lightrenderer.cpp"

$(BUILD)/logger.o: $(SRC)/logger.cpp $(SRC)/logger.h
	@ echo "Compiling logger.cpp..."
	@ mkdir -p $(BUILD)
	@ $(CC) $(CFLAGS) -o $(BUILD)/logger.o $(SRC)/logger.cpp
	@ echo "Finished compilation for logger.cpp"

$(BUILD)/configreader.o: $(SRC)/configreader.cpp $(SRC)/configreader.h
	@ echo "Compiling configreader.cpp..."
	@ mkdir -p $(BUILD)
	@ $(CC) $(CFLAGS) -o $(BUILD)/configreader.o $(SRC)/configreader.cpp
	@ echo "Finished compilation for configreader.cpp"

$(BUILD)/persistency.o: $(SRC)/persistency.cpp $(SRC)/persistency.h
	@ echo "Compiling persistency.cpp..."
	@ mkdir -p $(BUILD)
	@ $(CC) $(CFLAGS) -o $(BUILD)/persistency.o $(SRC)/persistency.cpp
	@ echo "Finished compilation for persistency.cpp"

# Clean all build files
clean:
	@ echo "Removing all build files..."
	@ rm -r -f $(BUILD) $(EXECUTABLE)
	@ echo "Build files deleted!"

install:
	@ bash $(INSTALL_SCRIPT)

uninstall:
	@ bash $(UNINSTALL_SCRIPT)
