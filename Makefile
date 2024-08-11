CC = gcc

CFLAGS = -Wall -std=c11 -pthread -pedantic -Wextra

LDFLAGS = -lwiringPi

TARGET = FiltrationManager

SRCS = main.c utils.c gpio_utils.c

HEADERS = utils.h gpio_utils.h

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean