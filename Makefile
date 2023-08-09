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

CPPSRC=media_list.cpp common.cpp qualify.cpp ext_lookup.cpp file_fmts.cpp

CXXSRC=MediaInfoDll.cxx

LIBS=-lshlwapi

OBJS = $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)  $(CXXSRC:.cxx=.o) 

#**************************************************************************
%.o: %.cpp
	$(TOOLS)\g++ $(CFLAGS) $<

%.o: %.cxx
	$(TOOLS)\g++ $(CxxFLAGS) $<

ifeq ($(USE_64BIT),NO)
BIN = MediaList.exe
else
BIN = media_list64.exe
endif

all: $(BIN)

clean:
	rm -f *.o *.exe *~ *.zip

dist:
	rm -f media_list.zip
	zip media_list.zip $(BIN) Readme.md MediaInfo.dll

wc:
	wc -l *.cpp

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp $(CPPSRC)"

depend: 
	makedepend $(CSRC) $(CPPSRC) $(CXXSRC)

$(BIN): $(OBJS)
	$(TOOLS)\g++ $(OBJS) $(LFLAGS) -o $(BIN) $(LIBS) 

# DO NOT DELETE

media_list.o: common.h media_list.h qualify.h
common.o: common.h
qualify.o: qualify.h
ext_lookup.o: common.h media_list.h file_fmts.h
file_fmts.o: common.h media_list.h file_fmts.h
MediaInfoDll.o: MediaInfoDLL.h common.h media_list.h file_fmts.h
