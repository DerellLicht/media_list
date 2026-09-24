# makefile for media_list app
# SHELL=cmd.exe
USE_DEBUG = NO
USE_64BIT = NO
USE_UNICODE = YES
USE_CLANG = NO
# sadly, cygwin mingw does not support gdiplus...
USE_CYGWIN = NO

include der_libs\tool_select.mak
include der_libs\release.mak

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

LIBS=-lshlwapi -lgdi32 -lcomdlg32

OBJS = $(CPPSRC:.cpp=.o)  $(CXXSRC:.cxx=.o) 

BASE := MediaList
BIN = $(BASE).exe

DIST_ZIP := $(BASE)V$(VERSION).zip

# Force these action-only targets to always run
.PHONY: dist

#**************************************************************************
%.o: %.cpp
	$(TOOLS)/$(GNAME) $(CFLAGS) $< -o $@

%.o: %.cxx
	$(TOOLS)/$(GNAME) $(CxxFLAGS) $< -o $@

all: $(BIN)

clean:
	rm -f $(OBJS) *.exe *~ *.zip

dist:
	rm -f *.zip
	zip $(DIST_ZIP) $(BIN) Readme.md MediaInfo.dll CHANGELOG.md

wc:
	wc -l $(CPPSRC) $(CXXSRC)

cppc:
	cmd /C "cppcheck --project=compile_commands.json --check-level=exhaustive --enable=all --std=c++14 --suppressions-list=./.suppress.cppcheck"

check:
	cmd /C "d:\llvm\bin\clang-tidy.exe $(CPPSRC)"

clint:
	cmd /C "python ..\ClaudeLint.py --exclude der_libs"

cstale:
	cmd /C "python ..\check_compile_commands_stale.py"

depend: 
	makedepend $(IFLAGS) $(CPPSRC) $(CXXSRC)

$(BIN): $(OBJS)
	$(TOOLS)/$(GNAME) $(OBJS) $(LFLAGS) -o $(BIN) $(LIBS) 

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
