date /T > tmpfile
set /p DATE= < tmpfile
del tmpfile 

time /T > tmpfile
set /p TIME= < tmpfile
del tmpfile 


set PACKAGE=%DATE:~5,2%月%DATE:~8,2%日%TIME:~0,2%时-内测包

echo %PACKAGE%
rd /S /Q %PACKAGE%
del /Q %PACKAGE%.7z

md %PACKAGE%

nbpack.exe %PACKAGE%\res_ ..\z_kxx\res -E *.wav *.ogg *.mtl *.obj

rem md %PACKAGE%\res
rem md %PACKAGE%\res\bgm
rem copy /Y ..\z_kxx\res\bgm\*.mid  %PACKAGE%\res\bgm

copy /Y kxx.exe %PACKAGE%

"%DIR7ZIP%\7z.exe" a -r %PACKAGE%.7z %PACKAGE%

@rem =====================

set PACKAGE=%DATE:~5,2%月%DATE:~8,2%日%TIME:~0,2%时-BGM

echo %PACKAGE%
rd /S /Q %PACKAGE%
del /Q %PACKAGE%.7z

md %PACKAGE%
md %PACKAGE%\res
md %PACKAGE%\res\bgm

copy /Y ..\z_kxx\res\bgm\*.ogg  %PACKAGE%\res\bgm
@rem copy /Y ..\z_kxx\res\bgm\*.mid  %PACKAGE%\res\bgm

"%DIR7ZIP%\7z.exe" a -r %PACKAGE%.7z %PACKAGE%



