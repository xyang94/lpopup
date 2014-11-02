@rem Script to build Lua BitOp with MSVC.

@rem First change the paths to your Lua installation below.
@rem Then open a "Visual Studio .NET Command Prompt", cd to this directory
@rem and run this script. Afterwards copy the resulting bit.dll to
@rem the directory where lua.exe is installed.

@if not defined INCLUDE goto :FAIL

@setlocal
@rem Path to the Lua includes and the library file for the Lua DLL:
@set LUA_INC=-I ..\lua-5.1.4\src

cl /nologo /MT /O2 /W3 /c /Fobit.obj %LUA_INC% bit.c
