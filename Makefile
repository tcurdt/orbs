platform := $(shell sh -c 'uname -s 2>/dev/null || echo not')

PREFIX?=/usr/local
BINARY=cafka
INSTALL_BIN=$(PREFIX)/bin
INSTALL=cp -pf

SOURCES=$(wildcard src/main/*.c src/main/bstr/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES})

LIB_SRC=$(filter-out src/main/${BINARY}.c,${SOURCES})
LIB_OBJ=$(filter-out src/main/${BINARY}.o,${OBJECTS})

TEST_SOURCES=$(wildcard src/test/*.c)
TESTS=$(patsubst %.c,%,${TEST_SOURCES})

CFLAGS=-g -O2 -Wall -Wextra -Isrc/main -pthread -rdynamic -DNDEBUG $(OPTFLAGS) -D_FILE_OFFSET_BITS=64


all: bin/${BINARY} tests

bin/${BINARY}: ${LIB_OBJ} src/main/${BINARY}.o
	@mkdir -p bin
	@echo linking $@
	@$(CC) $(CFLAGS) src/main/${BINARY}.o -o $@ $(LIB_OBJ)

${TESTS}:
	@$(CC) $(CFLAGS) -o $@.o -c $@.c
	@$(CC) $(CFLAGS) -o $@ $@.o $(LIB_OBJ)

%.o: %.c
	@echo building $@
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf bin
	rm -f ${OBJECTS}
	rm -f ${TESTS}
	rm -f valgrind.log
	rm -f src/test/tests.log

tests: ${TESTS}
	@echo
	@echo Running tests:
	@sh ./src/test/run.sh

valgrind: all
	valgrind --leak-check=full --show-reachable=yes --log-file=valgrind.log --suppressions=valgrind.sup ./bin/${BINARY}
