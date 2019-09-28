1. dumpbin /exports msvcrt.dll > msvcrt.def
2. lib /def:msvcrt.def /out:msvcrt.lib /machine:x64