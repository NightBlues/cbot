.PHONY: build test

CC=gcc -std=gnu11
LIBCHECK=$(shell pkg-config --cflags --libs check)
LIBGC=$(shell pkg-config --cflags --libs bdw-gc)
LIBMSGPACK=$(shell pkg-config --cflags --libs msgpack)
LIBEV=$(shell pkg-config --cflags --libs libev)
LIB_messaging_FILES=src/messaging/message.c src/messaging/peer.c
LIB_modules_FILES=src/modules/repairer.c src/modules/tcp_repairer.c

build: src/main.o libmessaging.a libmodules.a
	$(CC) $^ ${LIBGC} ${LIBEV} ${LIBMSGPACK} -o cbot

test: test-messaging

test-messaging: tests/check_messaging.o libmessaging.a
	$(CC) $^ ${LIBCHECK} ${LIBGC} ${LIBEV} ${LIBMSGPACK} -o $@
	./$@

clean:
	rm -rf *.o *.a cbot test-*
	find -name *.o -exec rm -rf {} \;

define libgen
lib$(1).a: ${LIB_$(1)_FILES:.c=.o}
	ar rcs lib$(1).a ${LIB_$(1)_FILES:.c=.o}
endef

$(foreach lib,messaging modules,$(eval $(call libgen,$(lib))))

%.o: %.c
	$(CC) ${LIBGC} ${LIBEV} ${LIBMSGPACK} -c $< -o $@
