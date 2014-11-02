mkdir vs
mkdir vs\Release
mkdir vs\lib
mkdir vs\bin
mkdir vs\include
cd vs\Release
cl /O2 /W3 /MT /c ../../src/l*.c
del lua.obj luac.obj
lib /out:../lib/lua.lib l*.obj
cl /O2 /W3 /MT /c ../../src/lua.c
link /out:../bin/lua.exe lua.obj ../lib/lua.lib
cd ../../
