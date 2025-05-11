.POSIX:
.PHONY: clean

SRC = lex.c fpc.c
OBJ = $(SRC:.c=.o)
EXE = fpc

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $(EXE)

.SUFFIXES: .c .o
.c.o:
	$(CC) -c $<

clean:
	rm -rf $(OBJ) $(EXE)
