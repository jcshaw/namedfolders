set ROOTDIR=%CD%

cd %ROOTDIR%\..\src\                                                   	
devenv /rebuild %3 /out %ROOTDIR%\nf-%1-%2.log ".\nf2x.%1.sln"
copy "C:\Program Files\Far\Plugins\NamedFolders\NamedFolders.dll" %ROOTDIR%\output\nf-%1-%2.dll
