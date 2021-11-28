@ECHO off
Echo CC.bat       Converting %2.%3 %4
%1\Bin2C.exe html\%2.%3 %2 %4
XCopy %2.* Generated\%2.* /Q/Y
del %2.c
del %2.h
