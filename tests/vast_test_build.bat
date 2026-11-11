@echo off

set DEF_COMPILER_FLAGS=-std=c89 -pedantic ^
-Wall -Wextra -Werror -Wvla -Wconversion -Wdouble-promotion -Wsign-conversion ^
-Wmissing-field-initializers -Wuninitialized -Winit-self -Wunused -Wunused-macros -Wunused-local-typedefs

REM Compile
cc -s -O2 %DEF_COMPILER_FLAGS% vast_test.c -o vast_test.exe

REM Execute
vast_test.exe
