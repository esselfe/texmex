
CC ?= gcc
#CC ?= clang
CFLAGS ?= -std=c17 -Wall -O2 -D_GNU_SOURCE
LDFLAGS ?= -lm
PROGNAME ?= texmex

.PHONY: default all clean

default: all

all: $(PROGNAME)
	@ls -li --color=auto $(PROGNAME) 2>/dev/null || true

$(PROGNAME): texmex.c
	$(CC) $(CFLAGS) $(LDFLAGS) texmex.c -o $(PROGNAME)

clean:
	@rm -v $(PROGNAME) 2>/dev/null || true

