# Compiler and linker
CC = g++ 
LD = c++ 

# compiler flags:
#  -g    adds debugging information to the executable file
#  -c compiles to object files
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -c -Wall

# linking information:
#  --libs libftdi
PKG_CONFIG = `pkg-config --cflags --libs libftdi1`

# source and build folders
SRC = src
BUILD = build

# name of the program
EXECUTABLE = LumizeDmxEngine


# Main executable target
$(EXECUTABLE): $(BUILD)/main.o
	@ echo "Linking main executable..."
	@ mkdir -p $(BUILD)
	@ $(LD) -o LumizeDmxEngine $(PKG_CONFIG) $(BUILD)/main.o
	@ echo "Build complete!"

$(BUILD)/main.o: $(SRC)/main.cpp
	@ echo "Compiling main.cpp..."
	@ mkdir -p $(BUILD)
	@ $(CC) $(CFLAGS) -o $(BUILD)/main.o $(SRC)/main.cpp
	@ echo "Finished compilation for main.cpp"

# Clean all build files
clean:
	@ echo "Removing all build files..."
	@ rm -r -f $(BUILD) $(EXECUTABLE)
	@ echo "Build files deleted!"

