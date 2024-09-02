# Define program names
TARGET1 = proc_info
TARGET2 = build

# Define the compiler
CC = gcc  # Assuming you're using gcc

# Define compiler flags (optional)
CFLAGS = -Wall 

# Rule to build proc_info
$(TARGET1): $(TARGET1).c
	$(CC) $(CFLAGS) -o $(TARGET1) $(TARGET1).c

# Rule to build build
$(TARGET2): $(TARGET2).c
	$(CC) $(CFLAGS) -o $(TARGET2) $(TARGET2).c

# Default target builds both programs
all: $(TARGET1) $(TARGET2)

# Clean target (optional)
clean:
	rm -f $(TARGET1) $(TARGET2)  # Remove both compiled programs
