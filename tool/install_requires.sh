#!/bin/bash

OLD_PWD=`pwd`; cd $(dirname $(dirname $0)); BASE_DIR=`pwd`; cd ${OLD_PWD}
source ${BASE_DIR}/tool/common.sh

if [ ! -d "${BASE_DIR}/vendor/gcc/lua-5.1.4" ]; then
  echo "Downloading Lua 5.1.4"
  cd ${BASE_DIR}/vendor/gcc
  wget http://www.lua.org/ftp/lua-5.1.4.tar.gz
  
  echo "explode Lua 5.1.4"
  tar zxvf lua-5.1.4.tar.gz
  rm -f lua-5.1.4.tar.gz

  echo "make the Lua"
  cd lua-5.1.4
  # aix ansi bsd freebsd generic linux macosx mingw posix solaris
  make mingw
else
  echo "Lua 5.1.4: installed"
fi

if [ ! -d "${BASE_DIR}/vendor/gcc/luafilesystem-1.5.0" ]; then
  echo "Downloading luafilesystem 1.5.0"
  cd ${BASE_DIR}/vendor/gcc
  wget --no-check-certificate https://github.com/keplerproject/luafilesystem/archive/v1.5.0.zip -O luafilesystem-1.5.0.zip
  
  echo "explode luafilesystem 1.5.0"
  unzip luafilesystem-1.5.0.zip
  rm -f luafilesystem-1.5.0.zip

  echo "make the luafilesystem 1.5.0"
  cd ${BASE_DIR}/vendor/gcc
  cp luafilesystem_Makefile luafilesystem-1.5.0/Makefile
  cd luafilesystem-1.5.0
  make
else
  echo "luafilesystem 1.5.0: installed"
fi

if [ ! -d "${BASE_DIR}/vendor/gcc/LuaBitOp-1.0.2" ]; then
  echo "Downloading LuaBitOp-1.0.2"
  cd ${BASE_DIR}/vendor/gcc
  wget http://bitop.luajit.org/download/LuaBitOp-1.0.2.tar.gz
  
  echo "explode LuaBitOp-1.0.2"
  tar zxvf LuaBitOp-1.0.2.tar.gz
  rm -f LuaBitOp-1.0.2.tar.gz

  echo "make the LuaBitOp-1.0.2"
  cd ${BASE_DIR}/vendor/gcc
  cp luabitop_Makefile LuaBitOp-1.0.2/Makefile
  cd LuaBitOp-1.0.2
  make
else
  echo "LuaBitOp-1.0.2: installed"
fi


