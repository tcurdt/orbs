platform := $(shell sh -c 'uname -s 2>/dev/null || echo not')

PREFIX?=/usr/local
INSTALL_BIN=$(PREFIX)/bin
INSTALL=cp -pf

SOURCES=$(wildcard src/main/*.c src/main/bstr/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES})

LIB_SRC=$(filter-out src/main/orbs-pipe.c,${SOURCES})
LIB_OBJ=$(filter-out src/main/orbs-pipe.o,${OBJECTS})

TEST_SOURCES=$(wildcard src/test/*.c)
TESTS=$(patsubst %.c,%,${TEST_SOURCES})

CFLAGS=-g -O2 -Wall -Wextra -Isrc/main -pthread -rdynamic -DNDEBUG $(OPTFLAGS) -D_FILE_OFFSET_BITS=64

all: build tests

dirs:
	@mkdir -p build

build: dirs build/orbs-pipe build/orbs-server ${TESTS}

build/orbs-server: build/liborbs.a
	@echo linking $@

build/orbs-pipe: build/liborbs.a src/main/orbs-pipe.o
	@echo linking $@
	@$(CC) $(CFLAGS) src/main/orbs-pipe.o -o $@ $< $(LIBS)

build/liborbs.a: CFLAGS += -fPIC
build/liborbs.a: ${LIB_OBJ}
	@echo linking $@
	@ar rcs $@ ${LIB_OBJ}
	@ranlib $@

${TESTS}:
	@$(CC) $(CFLAGS) -o $@.o -c $@.c
	@$(CC) $(CFLAGS) -o $@ $@.o $(LIB_OBJ)

%.o: %.c
	@echo building $@
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo cleaning
	@rm -rf build
	@rm -f ${OBJECTS}
	@rm -f ${TESTS}
	@rm -f src/test/tests.log

tests: build ${TESTS}
	@echo
	@echo Running tests:
	@sh ./src/test/run.sh



# valgrind: all
# 	valgrind --leak-check=full --show-reachable=yes --log-file=valgrind.log --suppressions=valgrind.sup ./bin/${BINARY}
