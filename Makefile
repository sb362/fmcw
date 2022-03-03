EXE    := fmcw
SRCS   := src/main.c
SRCS   += src/fmcw.c src/daq.c src/util.c src/window.c
HDRS   := src/fmcw.h src/daq.h src/util.h src/window.h

CFLAGS := -Iinclude/ -Isrc/ -Llib/ -std=c11 -Wall -Wextra -pedantic
CFLAGS += -Wno-unused-parameter -Wno-unused-command-line-argument

LIBS   := -lfftw3

debug  := yes
daq    := no

ifeq ($(daq),no)
	CFLAGS += -DFAKE_DAQ
	LIBS   += -lWD-Dask64
endif

ifeq ($(debug),yes)
	CFLAGS += -DLOG_LEVEL=DEBUG -g
else
	CFLAGS += -DLOG_LEVEL=FATAL -O3
endif

ifeq ($(OS),Windows_NT)
	CFLAGS += -Iinclude/pthreads4w/
	LIBS   += -lpthreadVC3

	CFLAGS += -Wno-deprecated-declarations
	CFLAGS += -DWIN32_LEAN_AND_MEAN
	EXE    := $(EXE).exe
else
	CFLAGS += -pthread
	LIBS   += -lm -lpthread
endif

$(info OS:         $(OS))
$(info Compiler:   $(CC))
$(info Executable: $(EXE))

OBJS   := $(addsuffix .o,$(basename $(SRCS)))
DEPS   := $(OBJS:.o=.d)
LDLIBS := $(LIBS)

src/%.o: src/%.c

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS) $(LIBS)

$(EXE): $(OBJS) $(HDRS)
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(LIBS) $(LDFLAGS)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(EXE)
