@echo off
set "V=%~dp0"
for %%I IN ( %V:~0,-1% ) do set "BASE_DIR0=%%~dpI"
for %%I IN ( %BASE_DIR0% ) do set "BASE_DIR1=%%~dpI"
set "BASE_DIR=%BASE_DIR1:~0,-1%"
@echo on

IF "x%VS_HOME%"=="x" (
@call "%BASE_DIR%\tool\vsvars.bat"
)

@IF EXIST "%BASE_DIR%\vendor\vs\lua-5.1.4" (
  goto lua_installed
) ELSE (
  goto lua_install
)

:lua_install
  echo "Downloading Lua 5.1.4"
  cd "%BASE_DIR%\vendor\vs"

  wget http://www.lua.org/ftp/lua-5.1.4.tar.gz
  
  echo "explode Lua 5.1.4"
  gunzip lua-5.1.4.tar.gz
  tar xvf lua-5.1.4.tar
  del /f lua-5.1.4.tar

  echo "make the Lua"
  copy lua_luavs_static.bat lua-5.1.4\etc\luavs_static.bat
  cd lua-5.1.4
  call etc\luavs_static.bat
  call etc\luavs.bat
  goto lfs_install

:lua_installed
  echo "Lua 5.1.4: installed"
  goto lfs_install

rem ----------------------------------------------------------
rem ----------------------------------------------------------
rem ----------------------------------------------------------

:lfs_install

@IF EXIST "%BASE_DIR%\vendor\vs\luafilesystem-1.5.0" (
  goto lfs_installed
) ELSE (
  goto lfs_install
)

:lfs_install
  echo "Downloading luafilesystem 1.5.0"
  cd "%BASE_DIR%\vendor\vs"

  wget --no-check-certificate https://github.com/keplerproject/luafilesystem/archive/v1.5.0.zip -O luafilesystem-1.5.0.zip

  echo "explode luafilesystem 1.5.0"
  unzip luafilesystem-1.5.0.zip
  del /f luafilesystem-1.5.0.zip

  echo "make the luafilesystem 1.5.0"
  cd "%BASE_DIR%/vendor/vs"
  copy luafilesystem_Makefile luafilesystem-1.5.0\Makefile.win
  copy luafilesystem_def luafilesystem-1.5.0\src\lfs.def
  cd luafilesystem-1.5.0
  nmake /F Makefile.win
  goto bitop_install

:lfs_installed
  echo "luafilesystem 1.5.0: installed"
  goto bitop_install

rem ----------------------------------------------------------
rem ----------------------------------------------------------
rem ----------------------------------------------------------

:bitop_install

@IF EXIST "%BASE_DIR%\vendor\vs\LuaBitOp-1.0.2" (
  goto bitop_installed
) ELSE (
  goto bitop_install
)

:bitop_install
  echo "Downloading LuaBitOp-1.0.2"
  cd "%BASE_DIR%\vendor\vs"

  wget http://bitop.luajit.org/download/LuaBitOp-1.0.2.tar.gz

  echo "explode LuaBitOp-1.0.2"
  gunzip LuaBitOp-1.0.2.tar.gz
  tar xvf LuaBitOp-1.0.2.tar
  del /f LuaBitOp-1.0.2.tar

  echo "make the LuaBitOp-1.0.2"
  cd ${BASE_DIR}/vendor/vendor
  copy bitop_msvcbuild.bat LuaBitOp-1.0.2\msvcbuild.bat
  cd LuaBitOp-1.0.2
  call msvcbuild.bat

  goto end_install

:bitop_installed
  echo "LuaBitOp-1.0.2: installed"
  goto end_install

rem ----------------------------------------------------------
rem ----------------------------------------------------------
rem ----------------------------------------------------------

:end_install


cd "%BASE_DIR%"
