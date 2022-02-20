CC      := clang
SRCS    := src/main.c src/fmcw.c src/util.c

CFLAGS  := -Iinclude/ -Isrc/ -g -std=c99 -Wall -Wextra -pedantic -Llib/ -pthread

ifeq ($(OS),Windows_NT)
	LIBS    := -lfftw3 -lpthread
	DEFS    := -DWIN32_LEAN_AND_MEAN
else
	LIBS    := -lfftw3 -lpthread
endif

OBJS    := $(addsuffix .o,$(basename $(SRCS)))
DEPS    := $(OBJS:.o=.d)
LDLIBS  := $(LIBS)

src/%.o: src/%.c

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEFS) $(LIBS)

fmcw.exe: $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(DEFS) $(LIBS) $(LDFLAGS)
