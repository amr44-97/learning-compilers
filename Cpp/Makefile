# Compiler and flags (use g++ or gcc with -x c++)
CC = g++
Warnings=  -Wall -Wdeprecated-copy -Wimplicit-fallthrough -Wno-unused-variable -Wno-unused-parameter -Wextra 
STDLIB = -std=c++23
BuildDebug = $(Warnings) -O0 -g
BuildFast = -O2 -g -fno-rtti -fno-exceptions -Wno-unused-variable -Wno-unused-parameter -Wimplicit-fallthrough

CFLAGS = $(BuildFast) $(STDLIB) -MMD -MP

# Directories
SRCDIR = src
BUILDDIR = build

# Source files and headers
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)

# Output executable
EXEC = compiler

# Default rule
all: $(EXEC)

# Rule to link object files into the final executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Create build directory if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Pattern rule to compile .c files into .o files as C++
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -x c++ -c $< -o $@

# Include dependency files if they exist
-include $(DEPS)

# Clean rule to remove compiled files
clean:
	rm -rf $(BUILDDIR) $(EXEC)

# Rebuild the project
rebuild: clean all

# Declare targets that are not files
.PHONY: all clean rebuild
