@ECHO OFF

ATTRIB -H *.suo

FOR %%i IN (Output, Ship, ipch) DO IF EXIST %%i RD %%i /S/Q

FOR %%i IN (EX~, DEP, OPT, PLG, APS, NCB, TMP, ZIP, LOG, ILK, SIO, ERR, TPU, SUO, NCB, SDF) DO IF EXIST *.%%i DEL *.%%i
