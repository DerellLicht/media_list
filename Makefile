# makefile for media_list app
# SHELL=cmd.exe
USE_DEBUG = NO
USE_64BIT = NO
USE_UNICODE = YES
USE_CLANG = NO
# sadly, cygwin mingw does not support gdiplus...
USE_CYGWIN = NO

include ..\tool_select.mak 

ifeq ($(USE_DEBUG),YES)
CFLAGS = -Wall -g -c
CxxFLAGS = -Wall -g -c
LFLAGS = -g
else
CFLAGS = -Wall -O3 -c
CxxFLAGS = -Wall -O3 -c
LFLAGS = -s -O3
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

ifeq ($(USE_CLANG),YES)
CFLAGS += -DUSING_CLANG
endif
LiFLAGS += -Ider_libs
CFLAGS += -Ider_libs
CxxFLAGS += -Ider_libs
IFLAGS += -Ider_libs

ifeq ($(USE_STATIC),YES)
LFLAGS += -static
endif
# This is required for *some* versions of makedepend
IFLAGS += -DNOMAKEDEPEND

CPPSRC=media_list.cpp ext_lookup.cpp file_fmts.cpp \
der_libs/conio_min.cpp \
der_libs/common_funcs.cpp \
der_libs/common_win.cpp \
der_libs/qualify.cpp 

CXXSRC=MediaInfoDll.cxx

LINTFILES=lintdefs.cpp lintdefs.ref.h 

LIBS=-lshlwapi -lgdi32 -lcomdlg32

OBJS = $(CPPSRC:.cpp=.o)  $(CXXSRC:.cxx=.o) 

GPP_NAME=g++
#GPP_NAME=clang++

#**************************************************************************
%.o: %.cpp
	$(TOOLS)/$(GPP_NAME) $(CFLAGS) $< -o $@

%.o: %.cxx
	$(TOOLS)/$(GPP_NAME) $(CxxFLAGS) $< -o $@

ifeq ($(USE_64BIT),NO)
BIN = MediaList.exe
else
BIN = media_list64.exe
endif

all: $(BIN)

clean:
	rm -f $(OBJS) *.exe *~ *.zip

dist:
	rm -f media_list.zip
	zip media_list.zip $(BIN) Readme.md MediaInfo.dll

wc:
	wc -l $(CPPSRC)

cppc:
	cmd /C "cppcheck --project=compile_commands.json --check-level=exhaustive --enable=all --std=c++14 --suppressions-list=./.suppress.cppcheck"

check:
	cmd /C "d:\llvm\bin\clang-tidy.exe $(CPPSRC)"

clint:
	cmd /C "python ..\ClaudeLint.py --exclude der_libs"

cstale:
	cmd /C "python ..\check_compile_commands_stale.py"

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) $(LINTFILES) $(CPPSRC)"

depend: 
	makedepend $(IFLAGS) $(CPPSRC) $(CXXSRC)

$(BIN): $(OBJS)
	$(TOOLS)/$(GPP_NAME) $(OBJS) $(LFLAGS) -o $(BIN) $(LIBS) 

# DO NOT DELETE

media_list.o: der_libs/common.h der_libs/conio_min.h media_list.h
media_list.o: der_libs/qualify.h
ext_lookup.o: der_libs/common.h der_libs/commonw.h der_libs/conio_min.h
ext_lookup.o: media_list.h file_fmts.h
file_fmts.o: der_libs/common.h der_libs/commonw.h der_libs/conio_min.h
file_fmts.o: media_list.h file_fmts.h
der_libs/conio_min.o: der_libs/common.h der_libs/conio_min.h
der_libs/common_funcs.o: der_libs/common.h
der_libs/common_win.o: der_libs/common.h der_libs/commonw.h
der_libs/qualify.o: der_libs/common.h der_libs/qualify.h
MediaInfoDll.o: MediaInfoDLL.h der_libs/common.h der_libs/commonw.h
MediaInfoDll.o: der_libs/conio_min.h media_list.h file_fmts.h
