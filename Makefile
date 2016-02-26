.PHONY: build

CC=gcc -std=gnu11
LIBCHECK=$(shell pkg-config --cflags --libs check)
LIB_messaging_FILES=src/messaging/message.c
LIB_modules_FILES=src/modules/repairer.c src/modules/tcp_repairer.c

build: libmessaging.a libmodules.a src/main.o
	$(CC) $^ -lev -o cbot

test-messaging: tests/check_messaging.o libmessaging.a
	$(CC) $^ ${LIBCHECK} -o $@
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
	$(CC) -c $< -o $@
