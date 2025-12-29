CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11

TARGET = copy_my
SRC = my_copy.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
