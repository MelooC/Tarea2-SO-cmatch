CC      = gcc
CFLAGS  = -Wall -Wextra -std=c17
LDFLAGS = -lpthread
SRC     = main.c matchmaking.c gato.c
OBJ     = $(SRC:.c=.o)
TARGET  = cmatch

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c tipos.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean