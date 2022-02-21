CC      := clang
SRCS    := src/main.c src/fmcw.c src/util.c src/window.c
HDRS    := src/fmcw.h src/util.h src/window.h

CFLAGS  := -Iinclude/ -Isrc/ -std=c11 -Wall -Wextra -pedantic -Llib/ -pthread
#CFLAGS += -g
CFLAGS  += -O3 -flto

ifeq ($(OS),Windows_NT)
	LIBS    := -lfftw3 -lpthread
	DEFS    := -DWIN32_LEAN_AND_MEAN
else
	LIBS    := -lm -lfftw3 -lpthread
endif

OBJS    := $(addsuffix .o,$(basename $(SRCS)))
DEPS    := $(OBJS:.o=.d)
LDLIBS  := $(LIBS)

src/%.o: src/%.c

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEFS) $(LIBS)

fmcw.exe: $(OBJS) $(HDRS)
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(DEFS) $(LIBS) $(LDFLAGS)
