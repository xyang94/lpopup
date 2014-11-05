COMMONHEADS	= $(shell find src -name '*h')
COMMONSRCS	= $(shell find src -name '*cpp' -not -name '*lpopup*')
COMMONOBJS	= $(COMMONSRCS:.cpp=.o) src/lp_resource.o

MAINPACKAGE	= bin/lpopup.exe
MAINHEADS	= $(COMMONHEADS)
MAINSRCS	= $(COMMONSRCS) src/lpopup.cpp
MAINOBJS	= $(COMMONOBJS) $(MAINSRCS:.cpp=.o)

DEBUG=-DDEBUG -g
RM=rm -rf
CP=cp
MKDIR=mkdir -p
VIRTUALENV=virtualenv
BINDIR=bin
LUA_DIR=./vendor/gcc/lua-5.1.4
LUA_FILESYSTEM_DIR=./vendor/gcc/luafilesystem-1.5.0
LUA_BITOP_DIR=./vendor/gcc/LuaBitOp-1.0.2

CC=g++
LD=g++
WINDRES=windres

CPPFLAGS=-I./src -I$(LUA_DIR)/src -I$(LUA_DIR)/etc -I$(LUA_FILESYSTEM_DIR)/src -I$(LUA_BITOP_DIR) $(DEBUG) -Wall -Wno-deprecated -std=gnu++0x -mwindows
LDFLAGS=-static-libgcc -static-libstdc++ -lshlwapi -luuid -lole32 -lcomctl32 -L$(LUA_DIR)/src -llua -L$(LUA_FILESYSTEM_DIR)/src -llfs -L$(LUA_BITOP_DIR) -lbit -static -mwindows

.SUFFIXES: .o .cpp .rc
.PHONY: clean run venv pip docs

all: $(MAINPACKAGE)

$(MAINPACKAGE): $(MAINOBJS)
	$(LD) $^ -o $@ $(LDFLAGS)

$(MAINOBJS): $(MAINHEADS)

.cpp.o:
	$(CC) $(CPPFLAGS) -c $< -o $@

.rc.o:
	$(WINDRES) -o $@ $<

clean:
	$(RM) $(PACKAGE) $(MAINOBJS)
	$(RM) core gmon.out

run:
	$(PACKAGE)

package:
	(echo "$$(gcc --version)" | grep i686 > /dev/null 2>&1; \
	 _LP_32=$$?;                                                  \
	 if [ $${_LP_32} == 0 ]; then                                 \
	   _LP_ARCH=x86_32;                                           \
	 else                                                         \
	   _LP_ARCH=x86_64;                                           \
	 fi;                                                          \
	 _LP_VERSION=`grep LP_VERSION src/lp_constants.h | sed -e 's/#define LP_VERSION //' | sed -e 's/"//g'`; \
	 _LP_PACKAGE_DIR=lpopup-$${_LP_ARCH}-$${_LP_VERSION};                         \
	 $(MKDIR) $${_LP_PACKAGE_DIR};                                                \
	 $(CP) bin/lpopup.lua $${_LP_PACKAGE_DIR};                                    \
	 $(CP) bin/launcher_lpopup.conf.lua $${_LP_PACKAGE_DIR};                      \
	 $(CP) bin/ext_lpopup.conf.lua $${_LP_PACKAGE_DIR};                           \
	 $(CP) bin/lpopup.exe $${_LP_PACKAGE_DIR};                                    \
	 $(CP) -r docs/build/html $${_LP_PACKAGE_DIR}/docs;                           \
	 $(RM) $${_LP_PACKAGE_DIR}/docs/.git;                                         \
	 zip -r lpopup-$${_LP_ARCH}-$${_LP_VERSION}.zip $${_LP_PACKAGE_DIR};          \
	)


dist: 
	${MAKE} "DEBUG="
	strip --strip-all bin/lpopup.exe

venv:
	${VIRTUALENV} .venv

pip:
	pip install sphinx

docs:
	sphinx-build -d docs/build/doctrees -b html docs/source docs/build/html

