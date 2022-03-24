EXE    := bin/fmcw
SRCS   := src/main.c src/ui-fake.c
SRCS   += src/fmcw.c src/daq.c src/util.c src/window.c src/thread.c src/dds.c
HDRS   := src/fmcw.h src/daq.h src/util.h src/window.h src/thread.h src/dds.c

CFLAGS := -Iinclude/ -Isrc/ -Llib/ -std=c11 -Wall -Wextra -pedantic
CFLAGS += -Wno-unused-parameter -Wno-unused-command-line-argument

LIBS   := -lfftw3

debug  := yes
netcdf := no
daq    := no

ifeq ($(daq),yes)
	LIBS   += -lWD-Dask64
else
	CFLAGS += -DFAKE_DAQ
endif

ifeq ($(debug),yes)
	CFLAGS += -DLOG_LEVEL=TRACE -g
else
	CFLAGS += -DLOG_LEVEL=FATAL -O3
endif

ifeq ($(netcdf),yes)
	CFLAGS += -DUSE_NETCDF
	SRCS   += src/export.c
	HDRS   += src/export.h

	ifeq ($(OS),Windows_NT)
		CFLAGS += -Iinclude/netcdf
		CFLAGS += -Llib/netcdf/
	endif

	LIBS += -lnetcdf
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

$(info OS:          $(OS))
$(info Compiler:    $(CC))
$(info Executable:  $(EXE))

$(info Debug build: $(debug))
$(info Use NetCDF:  $(netcdf))
$(info WD-Dask:     $(daq))

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
