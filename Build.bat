@echo off
if not exist "bin\" mkdir "bin"
if not exist "bin\sounds\" mkdir "bin\sounds\"

cls
pushd bin

set common_linker_flags= /incremental:no /opt:ref User32.lib Gdi32.lib

cl ../src/Blasteroids.cpp /nologo /Zi /std:c++17 /link %common_linker_flags%
popd


