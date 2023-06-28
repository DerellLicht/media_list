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
CxxFLAGS = -Wall -g -c
LFLAGS = -g
else
CFLAGS = -Wall -s -O3 -c
CxxFLAGS = -Wall -s -O3 -c
LFLAGS = -s -O3
endif
CFLAGS += -Weffc++
CFLAGS += -Wno-write-strings
ifeq ($(USE_64BIT),YES)
CFLAGS += -DUSE_64BIT
CxxFLAGS += -DUSE_64BIT
endif

CPPSRC=media_list.cpp qualify.cpp ext_lookup.cpp file_fmts.cpp

CXXSRC=MediaInfoDll.cxx

LIBS=-lshlwapi

OBJS = $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)  $(CXXSRC:.cxx=.o) 

#**************************************************************************
%.o: %.cpp
	$(TOOLS)\g++ $(CFLAGS) $<

%.o: %.cxx
	$(TOOLS)\g++ $(CxxFLAGS) $<

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
	zip media_list.zip media_list.exe Readme.md MediaInfo.dll

wc:
	wc -l *.cpp

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp $(CPPSRC)"

depend: 
	makedepend $(CSRC) $(CPPSRC) $(CXXSRC)

$(BIN): $(OBJS)
	$(TOOLS)\g++ $(OBJS) $(LFLAGS) -o $(BIN) $(LIBS) 

# DO NOT DELETE

media_list.o: media_list.h qualify.h
qualify.o: qualify.h
zplay_audio.o: media_list.h file_fmts.h
ext_lookup.o: media_list.h file_fmts.h
file_fmts.o: media_list.h file_fmts.h
mp3.parser.o: media_list.h
MediaInfoDll.o: MediaInfoDLL.h media_list.h file_fmts.h
