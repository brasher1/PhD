@if "%_echo%"=="" echo off
setlocal


REM ------------------------------------------------------------------
REM Work out if this version of the CLR supports generics.
REM This sample requires generics to be supported.

set CORDIR=unknown
if exist "%FSHARP_HOME%\setup\cordir.exe" ( 
  for /f "usebackq" %%i in (`"%FSHARP_HOME%\setup\cordir.exe"`) do ( 
    if exist %%i (
      set CORDIR=%%i
    )
  )
)

if "%FSHARP_HOME%"=="" ( set FSHARP_HOME=..\..\..)
if "%FSC%"=="" ( set FSC=%FSHARP_HOME%\bin\fsc.exe )

set SUPPORTING_GENERICS=true
if NOT "%CORDIR%"=="unknown" (
  if NOT "%CORDIR:v1.0=XXX%" == "%CORDIR%" (set SUPPORTING_GENERICS=false)
  if NOT "%CORDIR:v1.1=XXX%" == "%CORDIR%" (set SUPPORTING_GENERICS=false)
  if NOT "%FSC:no-generics=XXX%" == "%FSC%" (set SUPPORTING_GENERICS=false)
)

if "%SUPPORTING_GENERICS%"=="false" (
  echo *** This sample requires a CLR that supports generics, 
  echo *** e.g. v2.0 of the Microsoft .NET CLR or later.  Edit build.bat
  echo ** if your CLR supports generics
  goto Exit
)

REM -----------------------------------------
REM Configure the sample, i.e. where to find the F# compiler and C# compiler.

echo on
%FSC% --target-winexe --no-warn 40 -O2 -g -o Life.exe alg.fsi alg.fs worker.fs client.fs
echo off
if ERRORLEVEL 1 goto Error

echo ********************************************
echo Built ok, you may now run Life.exe
echo ********************************************

echo Passed fsharp ok.
:Exit
endlocal
exit /b 0


echo Ran fsharp ok.
:Error
endlocal

exit /b 1

