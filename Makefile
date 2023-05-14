# makefile for media_list app
SHELL=cmd.exe
USE_DEBUG = NO
USE_64BIT = NO

ifeq ($(USE_64BIT),YES)
TOOLS=d:\tdm64\bin
else
TOOLS=c:\mingw\bin
endif

ifeq ($(USE_DEBUG),YES)
CFLAGS = -Wall -g -c
LFLAGS = -g
else
CFLAGS = -Wall -s -O3 -c
LFLAGS = -s -O3
endif
CFLAGS += -Weffc++
CFLAGS += -Wno-write-strings
ifeq ($(USE_64BIT),YES)
CFLAGS += -DUSE_64BIT
endif

CPPSRC=media_list.cpp qualify.cpp zplay_audio.cpp ext_lookup.cpp file_fmts.cpp mp3.parser.cpp

LIBS=-lzplay -lshlwapi

OBJS = $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)

#*************************************************************************
%.o: %.cpp
	$(TOOLS)\g++ $(CFLAGS) $<

ifeq ($(USE_64BIT),NO)
BIN = media_list.exe
else
BIN = media_list64.exe
endif

all: $(BIN)

clean:
	rm -f *.o ndir*.exe *~ *.zip

dist:
	rm -f media_list.zip
	zip media_list.zip media_list.exe Readme.md libzplay.dll

wc:
	wc -l *.cpp

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp $(CPPSRC)"

depend: 
	makedepend $(CSRC) $(CPPSRC)

$(BIN): $(OBJS)
	$(TOOLS)\g++ $(OBJS) $(LFLAGS) -o $(BIN) $(LIBS) 

# DO NOT DELETE

media_list.o: media_list.h qualify.h
qualify.o: qualify.h
zplay_audio.o: media_list.h file_fmts.h
ext_lookup.o: media_list.h file_fmts.h
file_fmts.o: media_list.h file_fmts.h
mp3.parser.o: media_list.h
