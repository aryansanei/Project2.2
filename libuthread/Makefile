CC = gcc
CFLAGS = -Wall -Wextra -Werror -MMD
CFLAGS += -g  # Add debugging info

# List object files
OBJS = queue.o uthread.o sem.o preempt.o

# Include dependencies
-include $(OBJS:.o=.d)

# Default rule
all: libuthread.a

# Rule to make the library
libuthread.a: $(OBJS)
	ar rcs $@ $^

# Generic rule for object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $

# Clean up
clean:
	rm -f $(OBJS) $(OBJS:.o=.d) libuthread.a

.PHONY: clean all