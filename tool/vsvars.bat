@set "VS_HOME=C:\Program Files (x86)\Microsoft Visual Studio 11.0"
@set "BIN_ARCH=64"
@if "%BIN_ARCH%"=="64" (
  @call "%VS_HOME%\VC\bin\x86_amd64\vcvarsx86_amd64.bat"
)
@if "%BIN_ARCH%"=="32" (
  @call "%VS_HOME%\VC\bin\vcvars32.bat"
)
set
