
CFLAGS = -std=c11 -Wall -Werror -O2
LDFLAGS = 
PROGNAME = texmex

.PHONY: default all clean

default: all

all: $(PROGNAME)
	@ls -li --color=auto $(PROGNAME) 2>/dev/null || true

$(PROGNAME): texmex.c
	gcc $(CFLAGS) $(LDFLAGS) texmex.c -o $(PROGNAME)

clean:
	@rm -v $(PROGNAME) 2>/dev/null || true

