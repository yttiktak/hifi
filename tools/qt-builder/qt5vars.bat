@echo off

REM Set up \Microsoft Visual Studio 2017, where <arch> is \c amd64, \c x86, etc.
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Edit this location to point to the source code of Qt
SET _ROOT=..\qt5

SET PATH=%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin;%PATH%

REM Uncomment the below line when using a git checkout of the source repository
SET PATH=%_ROOT%\qtrepotools\bin;%PATH%

SET _ROOT=

REM Keeps the command line open when this script is run.
cmd /k