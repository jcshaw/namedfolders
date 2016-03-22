The command "cd:path" opens specified path on active panel.
F.e. "cd:c:\temp" opens directory "c:\temp".

It's possible to use common metacharacters `('*', '?', '..')` in "path".
F.e. `"cd:c:\t*"` suggests to select all directories matched to mask `"c:\t*"`. Command `"cd:c:\t\*"` will suggest to select one of subdirectories of `"c:\t"`. Please note, that `"cd:c:\t\"` is equal to `"cd:c:\t\*"` (see [issue #13](https://code.google.com/p/namedfolders/issues/detail?id=#13)).

NF tries to find best matched directory for "path". If "path" really exists then NF simply opens it without suggestion other variants (TODO: configurable?). If path doesn't exists NF suggests list of best matched directories.

F.e. user gives command "cd:c:\t". If "c:\t" exists then it will be opened. Otherwise, NF will suggest list of directories matched to mask `"c:\*t*"`. Attention: `*t*` - two stars here! (TODO: describe search police details).