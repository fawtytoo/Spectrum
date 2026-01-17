TARGET = spectrum

CC = gcc

CFLAGS = -g -Wall -MMD
LDFLAGS = -lSDL2

SRC = src
O = linux

OBJS = $(O)/main.o $(O)/spectrum.o $(O)/cpu.o $(O)/mem.o $(O)/ula.o $(O)/psg.o $(O)/tape.o $(O)/sinclair.o $(O)/cursor.o $(O)/kempston.o $(O)/fuller.o

all:	dir $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(O)/%.o:	$(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(O) $(TARGET)

dir:
	@mkdir -p $(O)

-include $(O)/*.d

