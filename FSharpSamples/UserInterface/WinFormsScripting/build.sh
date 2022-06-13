#!/bin/sh

mono ../../../bin/fscp10ntc.exe editor.fs
chmod u+x editor.exe

mono ../../../bin/fscp10ntc.exe helloform.fs
chmod u+x helloform.exe

echo You can run the samples:
echo   mono ./helloform.exe
echo   mono ./editor.exe

