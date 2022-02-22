CC     := clang
EXE    := fmcw.exe
SRCS   := src/main.c
SRCS   += src/fmcw.c src/daq.c src/util.c src/window.c
HDRS   := src/fmcw.h src/daq.h src/util.h src/window.h
LIBS   := -lfftw3 -lpthread

CFLAGS := -Iinclude/ -Isrc/ -std=c11 -Wall -Wextra -pedantic -Llib/
CFLAGS += -Wno-unused-parameter -Wno-unused-command-line-argument
CFLAGS += -Wno-deprecated-declarations

CFLAGS += -DFAKE_DAQ

CFLAGS += -g
#CFLAGS += -O3 -flto

ifeq ($(OS),Windows_NT)
	DEFS   += -DWIN32_LEAN_AND_MEAN
else
	LIBS   += -lm
	CFLAGS += -pthread
endif

OBJS   := $(addsuffix .o,$(basename $(SRCS)))
DEPS   := $(OBJS:.o=.d)
LDLIBS := $(LIBS)

src/%.o: src/%.c

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEFS) $(LIBS)

$(EXE): $(OBJS) $(HDRS)
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(DEFS) $(LIBS) $(LDFLAGS)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(EXE)
