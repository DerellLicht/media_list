# makefile for media_list app
# SHELL=cmd.exe
USE_DEBUG = NO
USE_64BIT = NO
USE_UNICODE = YES

ifeq ($(USE_64BIT),YES)
TOOLS=d:\tdm64\bin
else
TOOLS=c:\tdm32\bin
#TOOLS=c:\mingw32\bin
endif

ifeq ($(USE_DEBUG),YES)
CFLAGS = -Wall -g -c
CxxFLAGS = -Wall -g -c
LFLAGS = -g
else
CFLAGS = -Wall -s -O3 -c
CxxFLAGS = -Wall -s -O3 -c
LFLAGS = -s -O3 -m32
endif
CFLAGS += -Weffc++
CFLAGS += -Wno-write-strings
ifeq ($(USE_64BIT),YES)
CFLAGS += -DUSE_64BIT
CxxFLAGS += -DUSE_64BIT
endif

ifeq ($(USE_UNICODE),YES)
CFLAGS += -DUNICODE -D_UNICODE
CxxFLAGS += -DUNICODE -D_UNICODE
LiFLAGS += -dUNICODE -d_UNICODE
LFLAGS += -dUNICODE -d_UNICODE
endif
# neither of these fix __gxx_personality_v0 runtime error
# it is caused by the Dwarf version of the MinGW toolchain
#LFLAGS += -lstdc++
#LFLAGS += -fno-exceptions -fno-rtti 

LiFLAGS += -Ider_libs
CFLAGS += -Ider_libs
CxxFLAGS += -Ider_libs

CPPSRC=media_list.cpp ext_lookup.cpp file_fmts.cpp conio_min.cpp \
der_libs\common_funcs.cpp \
der_libs\common_win.cpp \
der_libs\qualify.cpp 

CXXSRC=MediaInfoDll.cxx

#  clang-tidy options
CHFLAGS = -header-filter=.*
CHTAIL = --
CHTAIL += -Ider_libs
ifeq ($(USE_64BIT),YES)
CHTAIL += -DUSE_64BIT
endif
ifeq ($(USE_UNICODE),YES)
CHTAIL += -DUNICODE -D_UNICODE
endif

LIBS=-lshlwapi -lgdi32 -lcomdlg32

OBJS = $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)  $(CXXSRC:.cxx=.o) 

#**************************************************************************
%.o: %.cpp
	$(TOOLS)\g++ $(CFLAGS) -c $< -o $@

%.o: %.cxx
	$(TOOLS)\g++ $(CxxFLAGS) -c $< -o $@

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

check:
	cmd /C "d:\clang\bin\clang-tidy.exe $(CHFLAGS) $(CPPSRC) $(CHTAIL)"

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp $(CPPSRC)"

depend: 
	makedepend $(CSRC) $(CPPSRC) $(CXXSRC)

$(BIN): $(OBJS)
	$(TOOLS)\g++ $(OBJS) $(LFLAGS) -o $(BIN) $(LIBS) 

# DO NOT DELETE

media_list.o: conio_min.h media_list.h
ext_lookup.o: conio_min.h media_list.h file_fmts.h
file_fmts.o: conio_min.h media_list.h file_fmts.h
conio_min.o: conio_min.h
MediaInfoDll.o: MediaInfoDLL.h media_list.h file_fmts.h
