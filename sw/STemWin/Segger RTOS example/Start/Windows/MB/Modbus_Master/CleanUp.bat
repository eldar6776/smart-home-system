@ECHO OFF

FOR %%i IN (Exe, Output, Ship, Debug) DO IF EXIST %%i RD %%i /S/Q

FOR %%i IN (EX~, DEP, OPT, PLG, APS, NCB, TMP, ZIP, LOG, ILK, SIO, ERR, TPU) DO IF EXIST *.%%i DEL *.%%i
