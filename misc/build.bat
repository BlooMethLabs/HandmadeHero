@echo off

mkdir ..\build
pushd ..\build
cl -MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7 -Fmwin32_handmade.map ..\code\win32_handmade.cpp /link -opt:ref -subsystem:windows,5.1 user32.lib Gdi32.lib
popd