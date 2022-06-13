@setlocal
@REM 1. Configure the sample, i.e. where to find the F# compiler and TLBIMP tool.

@if "%FSHARP_HOME%"=="" ( set FSHARP_HOME=..\..\..)
@set FSC=%FSHARP_HOME%\bin\fsc.exe

@REM 2. Build the sample

%FSC% --target-winexe -g editor.fs
%FSC% --target-winexe -g helloform.fs
@if ERRORLEVEL 1 goto Exit

@echo You can run the samples:
@echo helloform.exe
@echo editor.exe
:Exit

@endlocal
@exit /b %ERRORLEVEL%
