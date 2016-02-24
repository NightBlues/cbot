.PHONY: build

CC=gcc -std=gnu11
LIB_messaging_FILES=src/messaging/message.c
LIB_modules_FILES=src/modules/repairer.c src/modules/tcp_repairer.c

build: libmessaging.a libmodules.a
	$(CC) src/main.c -lev $^ -o cbot

clean:
	rm -rf *.o *.a cbot
	find -name *.o -exec rm -rf {} \;

define libgen
lib$(1).a: ${LIB_$(1)_FILES:.c=.o}
	ar rcs lib$(1).a ${LIB_$(1)_FILES:.c=.o}
endef

$(foreach lib,messaging modules,$(eval $(call libgen,$(lib))))

%.o: %.c
	$(CC) -c $< -o $@
