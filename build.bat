@echo off

REM Simple build script template for small projects
REM No need for 'make' and stuff

echo Starting compilation...

REM Define compiler
set COMPILER=gcc

REM Output file name
set OUTPUT_FILE_NAME=main.exe

REM Define resource files to include
set RES_FILES=

REM Define the source files to include
set SRC_FILES=main.c

REM Debug build
@REM  %COMPILER% %SRC_FILES% %RES_FILES% -o %OUTPUT_FILE_NAME% -std=c99 -lgdi32 -luser32 -lwinmm -Wall

REM Release build
%COMPILER% %SRC_FILES% %RES_FILES% -o %OUTPUT_FILE_NAME% -lgdi32 -luser32 -lwinmm -mwindows -O3 -Wall -s -std=c99 -fno-strict-aliasing -fomit-frame-pointer -flto

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed.
    exit /b %ERRORLEVEL%
)
echo Compilation succeeded.

REM Check if the output file exists before attempting to run it
if exist %OUTPUT_FILE_NAME% (
    echo Running %OUTPUT_FILE_NAME%...
    %OUTPUT_FILE_NAME%
) else (
    echo %OUTPUT_FILE_NAME% not found.
)