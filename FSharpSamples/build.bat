@if "%_echo%"=="" echo off

setlocal
for %%i in (Samples101 SimpleInterop SimpleForm ConcurrentLife TypeFinder Sockets AsyncDelegates Differentiate Parsing WinForms) do ( 
   if exist %%i\build.bat (
	pushd %%i
	echo **************************************************
	echo Building sample %%i
	echo **************************************************
	setlocal
        call build.bat
        endlocal
	@if ERRORLEVEL 1 goto Exit
	popd
   ) 
)

:Exit

endlocal
exit /b %ERRORLEVEL%

