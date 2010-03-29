@echo off
if not '%1' == '' goto main
    echo Usage: %0 VERSION-STRING
    goto exit
:main
if not '%OS%' == 'Windows_NT' goto OS2
    zip nyacus-%1.zip nyacus.exe _nya nyacus.txt
    zip nyados-%1.zip nyados.exe _nya nyados.txt
    hg archive -t zip nya-%1.zip
    goto exit
:OS2
    zip nyaos2-%1.zip nyaos2.exe _nya nyaos2.txt
:exit
