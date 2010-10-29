msxsl.exe !version.xml file_id.src.xslt -xw -t -o lang\file_id.diz
msxsl.exe !version.xml readme.src.eng.xslt -xw -t -o lang\!readme.eng
msxsl.exe !version.xml readme.src.rus.xslt -xw -t -o lang\!readme.rus
msxsl.exe whatsnew.rus.xml whatsnew.rus.xslt -xw -t -o lang\whatsnew.rus.txt
msxsl.exe whatsnew.eng.xml whatsnew.eng.xslt -xw -t -o lang\whatsnew.eng.txt
msxsl.exe NamedFolders.Hlf.xml hlf.xslt -xw -t -o lang\NamedFolders.Hlf
msxsl.exe NamedFoldersRus.Hlf.xml hlf.xslt -xw -t -o lang\NamedFoldersRus.Hlf