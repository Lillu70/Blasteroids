@echo off
cls
pushd bin

set common_linker_flags= /incremental:no /opt:ref User32.lib Gdi32.lib

cl ../src/Blasteroids.cpp /MT /GR- /EHa- /options:strict /sdl /nologo /O2 /Zi /fp:fast /GA /Ot /std:c++20 /link %common_linker_flags%
popd