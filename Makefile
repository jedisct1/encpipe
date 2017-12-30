PREFIX ?= /usr/local
WFLAGS ?= -Wall -Wextra -Wmissing-prototypes -Wdiv-by-zero -Wbad-function-cast -Wcast-align -Wcast-qual -Wfloat-equal -Wmissing-declarations -Wnested-externs -Wno-unknown-pragmas -Wpointer-arith -Wredundant-decls -Wstrict-prototypes -Wswitch-enum -Wno-type-limits
CFLAGS ?= -Os -march=native -fno-exceptions $(WFLAGS)
CFLAGS += -I.
OBJ = encpipe.o safe_rw.o
AR ?= ar
RANLIB ?= ranlib
STRIP ?= strip

SRC = \
	common.h \
	encpipe.c \
	encpipe_p.h \
	safe_rw.c \
	safe_rw.h

all: bin

bin: encpipe

encpipe: $(OBJ)
	$(CC) $(CFLAGS) -o encpipe $(OBJ) -lhydrogen

install: bin
	-$(STRIP) encpipe 2> /dev/null
	mkdir -p $(PREFIX)/bin
	install -o 0 -g 0 -m 0755 encpipe $(PREFIX)/bin 2> /dev/null || install -m 0755 encpipe $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/encpipe

$(OBJ): $(SRC)

.PHONY: clean

clean:
	rm -f encpipe $(OBJ)
