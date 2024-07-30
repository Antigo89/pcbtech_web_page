@ECHO OFF
Echo CC.bat       Converting %3.%4
%1\Bin2C.exe %2\%3.%4 %3
XCopy %3.* Generated\%3.* /Q/Y
del %3.c
del %3.h
