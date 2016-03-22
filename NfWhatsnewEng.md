
```
                    The plugin "Named Folders"
                    ~~~~~~~~~~~~~~~~~~~~~~~~~~
                    

[!] Important
[+] New
[-] Fixed error
[*] Changes

----------------------------------------------------------------------------
Version 3.0.0.254 release [22.01.2011]
----------------------------------------------------------------------------
[*] Small code improvements.
[*] Help is improved.

----------------------------------------------------------------------------
Version 3.0.0.253 RC2 [16.01.2011]
----------------------------------------------------------------------------
[!] Plugin supports two new  metacharacters "\**:N" and "\..*:N".
They allow to limit deep search for search in deep and search up.
Metacharacters "\**" and "\..*" are used for search with unlimited level search.
If option "Short syntax in paths" is turned on, then you are able to use
alternative variant for these metacharactes: N slashes and N+1 dots.
[-] Command "cd:\\network_path" to open network paths directly 
doesn't work in previous versions of the plugin. Now it's implemented.
[-] Pressing Esc during process of searching stopped search process
without visualisation of already found results.
[-] Plugin don't open aliases that links network directories with
dot(s) in path, i.e. "\\computer\abc.cde"
[-] Commands "cd:%var%" and "cd:%var" don't work under WinXP in 
some cases.
[+] Command "cd:path" allows to specify path in quotes, i.e. 
cd:"c:\temp"
[+] Commands cd:--f" and "cd:--df" supports new possibility to
specify required directory directly, without alias. I.e. "cd:--f c:\temp\**". 
If required path contains spaces, the path should be qouted, i.e.:
cd:--df "c:\program files\**:2\*diz"
[*] Help is improved

----------------------------------------------------------------------------
Version 3.0.0.249 RC1 [7.01.2011]
----------------------------------------------------------------------------
[*] Direct path commands use technology of search by related
paths (see above). I.e. they support metachars "\**", "..*", "\*", "\..".
Commands "cd:z:" and "cd:z:\" are executed as "cd:z:\*"
[-] Command "cd:a/" created catalog. Now, it doesn't create
catalog but only open exist catalog on NF-panel, see #38
[+] Implicit commands of creation shortcuts for two directories
(on both panels) are implemented: "cd:::", "cd::+", "cd:::catalog/",
"cd::+catalog/", see #38
[-] Command "cd::a/" didn't create catalog "a" if name of
catalog contained only single letter, see #38
[-] Pressing Ctrl+'/' on NF-panel resulted to creation garbage
catalogs on panel, see #39. Now, Ctrl+'/' opens parent directory and set
position on "..". Ctrl+PageUp opens parent directory and set position on
previously selected catalog, see #41.
[-] Implicit shorcut creation commands allow to specify catalog,
i.e: cd::catalog1/. If "catalog1" doesn't exists the command will suggest
to create catalog, not shorcut. So, to create shortcut with implicit name
in new catalog "catalog1" you should execute command "cd::catalog1/" two
times.
[*] Command cd::a/b suggests to create "a", if "a" doesn't
exist, and the suggests to create shortcut "b". Previously it did nothing
if "a" didn't exist. See #20.
[-] A lot of bugs were fixed.

----------------------------------------------------------------------------
Version 3.0.0.245 beta [25.12.2010]
----------------------------------------------------------------------------
Key changes
~~~~~~~~~~~
[!] Plugin uses Far API 2.0 and unicode.
[!] 64-bit version of the plugin is released. Now plugin is available both for x86 and x64.
[!] Plugin has own open source project on google code: 
http://code.google.com/p/namedfolders/. You can write bugreports and suggestion to
issue tracker here. You can also download most recent snapshot of the plugin
(all releases will be send to PlugRing as previously).
[!] NF3 doesn't work correctly with russian shortcuts, that
were created under NF2. As result, it's necessary to recreate all shortcuts,
that contain Russian letters in names and/or in paths.

Changes
~~~~~~~
[-] Launch of x64-apps from FAR x86 is fixed, see #6 
(http://code.google.com/p/namedfolders/issues/detail?id=6)
[*] Procedure of removing douple prefixes is changed. In previous
versions NF, command "cD:ab:command" was equal to "cd:command" and  
command "cd:cd:cd:command" didn't work at all. Now, the plugin ignores
all prefixes except last one. So, the command "ab:cd:ef:command" is
equal to "ef:command".
[*] Search in local paths is changed. No, the command "cd:alias.." 
can be written in more classic way "cd:alias\..". Both variants ".." and "\.."
are supported to specify parent folder.
[+] New possibility to edit shortuct from variants menu.
F4 is used to edit menu. To change view mode "shorctuts and values" on
plese use F7 instead of F4.
[+] Macros FastPrefix is fixed for Far 2.0. Now, key ':'
types "cd:" in command line and changes Russian to English if Russian
layout was selected. So, you can press ':' and types your NF command
in English langugage.
[+] Variants menu shows total list of items.
[!] Technology of search by related paths is completely changed.
New metacharacters "\**" and "\..*" are used to search without deep
limitations insted of depricatead metacharacters "\*\" and "\.\".
Sequence of N-metacharacters "\*", "\.." is used to search on 
N levels instead of sequence of slashes and dishes. Old syntax
with slashes and dishes are supported also (it can be turned on/off
in plugin settings, see param "Short syntax in paths".
See wiki page 
http://code.google.com/p/namedfolders/wiki/WIkiRelatedPathsRu 
for details.

[*] If you execute command "cd:a\path" and plugin hasn't found
directory matched to "path", it will show standard confirmation 
"do you want to go to nearest directory?". Previous version didn't
show confirmation, they open the directory assigned to shortcut "a".
You can disable this confirmations in plugin settings.

Settings
~~~~~~~~
[*] Possibility to specify fast key for the plugin in disk
menu is removed. It's not actual anymore after releasing Far 2.0 build
1692. Issue #15
[+] New flag "Show catalogs in disk menu". If it's turned on,
then NF shows catalog in disk menu besid standard menu item "Named Folders".
This is experimental implementation of Issue #8.
[+] New flag "Short syntax in paths". This flag enables
possibility to use "\\\" instead of "\*\*\*" and "...." instead of "\..\.."
in related path. Also, it allows to use "cd:a.." instead of "cd:a\.."


```