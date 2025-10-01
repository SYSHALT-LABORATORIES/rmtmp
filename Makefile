CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c99
TARGET = rmtmp
SRCS = trash.c
OBJS = $(SRCS:.c=.o)
all:$(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) $(TARGET)
.PHONY: all clean
