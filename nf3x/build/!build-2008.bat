set CDDIR=%CD%

del ".\output\nf2x-2008-x86.dll"
del ".\output\nf2x-2008-x64.dll"

cd %CDDIR%
call "!vsvars32-2008.bat"

cd %CDDIR%
call "!vsvars64-2008.bat"

cd %CDDIR%
copy "..\bin\nf2x\Release\nf2x.dll" ".\output\nf2x-2008-x86.dll"
copy "..\bin\x64\Release\nf2x.dll" ".\output\nf2x-2008-x64.dll"