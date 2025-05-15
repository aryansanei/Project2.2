# File: libuthread/Makefile

CC = gcc
AR = ar
CFLAGS = -Wall -Werror -g
ARFLAGS = rcs

LIBRARY = libuthread.a
OBJS = queue.o

# Default target
all: $(LIBRARY)

$(LIBRARY): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(LIBRARY)
