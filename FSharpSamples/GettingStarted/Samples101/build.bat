@if "%_echo%"=="" echo off
setlocal

REM ------------------------------------------------------------------
REM Configure the sample, i.e. where to find the F# compiler

if "%FSHARP_HOME%"=="" ( set FSHARP_HOME=..\..\..)
if "%FSC%"=="" ( set FSC=%FSHARP_HOME%\bin\fsc.exe )
if "%RESXC%"=="" ( set RESXC=%FSHARP_HOME%\bin\resxc.exe )

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

REM ------------------------------------------------------------------
REM Compile the F# code 

%FSC% --no-warn 62 -o samples101.exe SampleForm.resx sample.fs beginnersInLightSyntax.fs sampleform.fs program.fs

if ERRORLEVEL 1 goto Exit

echo ********************************************
echo Built ok, you may now run samples101.exe
echo ********************************************


:Exit
endlocal

exit /b %ERRORLEVEL%
