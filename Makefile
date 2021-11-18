PREFIX ?= /usr/local
WFLAGS ?= -Wall -Wextra -Wmissing-prototypes -Wdiv-by-zero -Wbad-function-cast -Wcast-align -Wcast-qual -Wfloat-equal -Wmissing-declarations -Wnested-externs -Wno-unknown-pragmas -Wpointer-arith -Wredundant-decls -Wstrict-prototypes -Wswitch-enum -Wno-type-limits
CFLAGS ?= -O3 -march=native -fno-exceptions -flto $(WFLAGS)
CPPFLAGS += -I. -Iext/libhydrogen
STRIP ?= strip

OBJ = ext/libhydrogen/hydrogen.o src/encpipe.o src/safe_rw.o

SRC = \
	ext/libhydrogen/hydrogen.c \
	src/common.h \
	src/encpipe.c \
	src/encpipe_p.h \
	src/safe_rw.c \
	src/safe_rw.h

all: bin test

bin: encpipe

$(OBJ): $(SRC)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

encpipe: $(OBJ)
	$(CC) $(CFLAGS) -o encpipe $(LDFLAGS) $(OBJ)

ext/libhydrogen/hydrogen.c:
	git submodule update --init || echo "** Make sure you cloned the repository **" >&2

install: all
	-$(STRIP) --strip-all encpipe 2> /dev/null || $(STRIP) encpipe 2> /dev/null
	mkdir -p $(PREFIX)/bin
	install -o 0 -g 0 -m 0755 encpipe $(PREFIX)/bin 2> /dev/null || install -m 0755 encpipe $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/encpipe

test: bin
	@echo test | ./encpipe -e -p password | ./encpipe -d -p password -o /dev/null

.PHONY: clean

clean:
	rm -f encpipe $(OBJ)

distclean: clean

.SUFFIXES: .c .o

check: test

distclean: clean
