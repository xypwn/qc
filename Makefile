READLINEFLAGS = -lreadline -DENABLE_READLINE
LDFLAGS =
CFLAGS  = -Ofast -march=native -Wall -pedantic -Werror -lm $(READLINEFLAGS)
#CFLAGS  = -ggdb -Wall -pedantic -Werror -lm $(READLINEFLAGS)
CC      = cc
EXE     = qc

all: $(EXE)

$(EXE): main.c expr.c expr.h expr_config.h
	$(CC) -o $@ main.c expr.c $(LDFLAGS) $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(EXE)
