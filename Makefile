# makefile for media_list app
# SHELL=cmd.exe
USE_DEBUG = NO
USE_64BIT = NO
USE_UNICODE = YES
USE_CLANG = NO
# sadly, cygwin mingw does not support gdiplus...
USE_CYGWIN = NO

include der_libs\tool_select.mak

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

LINTFILES=lintdefs.cpp lintdefs.ref.h 

LIBS=-lshlwapi -lgdi32 -lcomdlg32

OBJS = $(CPPSRC:.cpp=.o)  $(CXXSRC:.cxx=.o) 

#GPP_NAME=g++
#GPP_NAME=clang++

BASE := MediaList

# Automatically parse the latest version block
VERSION := $(shell grep -oE '\[[0-9]+\.[0-9]+\]' CHANGELOG.md | head -n 1 | tr -d '[]')
DIST_ZIP := $(BASE)V$(VERSION).zip
#**************************************************************************
%.o: %.cpp
	$(TOOLS)/$(GNAME) $(CFLAGS) $< -o $@

%.o: %.cxx
	$(TOOLS)/$(GNAME) $(CxxFLAGS) $< -o $@

BIN = $(BASE).exe

all: $(BIN)

clean:
	rm -f $(OBJS) *.exe *~ *.zip

# Your new automated release workflow
release:
	cmd /C "@echo Preparing GitHub release for v$(VERSION)..."
	sed -n '/## \['$(VERSION)'\]/,/## \[/p' CHANGELOG.md | sed '$$d' > temp_notes.md
	gh release create v$(VERSION) ./$(DIST_ZIP) ./CHANGELOG.md --notes-file temp_notes.md
	rm temp_notes.md
	cmd /C "@echo Release v$(VERSION) successfully uploaded to GitHub!"wc:
	
dist:
	rm -f *.zip
	zip $(DIST_ZIP) $(BIN) Readme.md MediaInfo.dll CHANGELOG.md

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
