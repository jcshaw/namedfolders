set CDDIR=%CD%

del ".\output\nf2x-2010-x86.dll"
del ".\output\nf2x-2010-x64.dll"

cd %CDDIR%
call "!vcvars32-2010.bat"

cd %CDDIR%
call "!vcvars64-2010.bat"

cd %CDDIR%
copy "..\bin2010\nf2x\Release\nf2x.dll" ".\output\nf2x-2010-x86.dll"
copy "..\bin2010\x64\Release\nf2x.dll" ".\output\nf2x-2010-x64.dll"